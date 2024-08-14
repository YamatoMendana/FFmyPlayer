#include "playerDisplay.h"

#include <signal.h>
#include <QDebug>



PlayerDisplay::PlayerDisplay()
{
	int flags = 0;

	signal(SIGINT, sigterm_handler); 
	signal(SIGTERM, sigterm_handler); 

	avformat_network_init();

	um_stCloseHandlerMap.insert({ AVMEDIA_TYPE_AUDIO,std::bind(&PlayerDisplay::audioStreamClose,this) });
	um_stCloseHandlerMap.insert({ AVMEDIA_TYPE_VIDEO,std::bind(&PlayerDisplay::videoStreamClose,this) });
	um_stCloseHandlerMap.insert({ AVMEDIA_TYPE_SUBTITLE,std::bind(&PlayerDisplay::subtitleStreamClose,this) });

	audio_dev = GlobalSingleton::getInstance()->getConfigValue<SDL_AudioDeviceID>("audio_dev");
	filter_nbthreads = GlobalSingleton::getInstance()->getConfigValue<int>("filter_nbthreads");
	audio_callback_time = GlobalSingleton::getInstance()->getConfigValue<int64_t>("audio_callback_time");
	seek_by_bytes = GlobalSingleton::getInstance()->getConfigValue<int>("seek_by_bytes");
	start_time = GlobalSingleton::getInstance()->getConfigValue<int64_t>("start_time");

	Audio_disable = GlobalSingleton::getInstance()->getConfigValue<bool>("Audio_disable");
	Video_disable = GlobalSingleton::getInstance()->getConfigValue<bool>("Video_disable");
	Subtitle_disable = GlobalSingleton::getInstance()->getConfigValue<bool>("Subtitle_disable");
	Display_disable = GlobalSingleton::getInstance()->getConfigValue<bool>("Display_disable");

	infinite_buffer = GlobalSingleton::getInstance()->getConfigValue<int>("infinite_buffer");
	loop = GlobalSingleton::getInstance()->getConfigValue<int>("loop");
	autoexit = GlobalSingleton::getInstance()->getConfigValue<int>("autoexit");
}

PlayerDisplay::~PlayerDisplay()
{

}

int PlayerDisplay::stream_open(const char* filename)
{
	int ret;

	auto startup_volume = GlobalSingleton::getInstance()->getConfigValue<int>("startup_volume");

	nLast_video_stream = nVideo_stream = -1;
	nLast_audio_stream = nAudio_stream = -1;
	nLast_subtitle_stream = nSubtitle_stream = -1;
	pFilename = av_strdup(filename);
	try{
		if (!pFilename)
		{
			QString error = QString("%1:%2 Can not find the filename,the file name is error\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), WITHOUT_FILE_NAME);
		}

		if (pictq.init(&videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
		{
			QString error = QString("%1:%2 picture list init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PICTURE_LIST_INIT_FAIL);
		}
		if (subpq.init(&subtitleq, SUBPICTURE_QUEUE_SIZE, 0) < 0)
		{
			QString error = QString("%1:%2 subpicture list init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), SUBTITLE_LIST_INIT_FAIL);
		}
		if (sampq.init(&audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
		{
			QString error = QString("%1:%2 audio list init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), AUDIO_LIST_INIT_FAIL);
		}


		if (videoq.init() < 0 ||
			audioq.init() < 0 ||
			subtitleq.init() < 0)
		{
			QString error = QString("%1:%2 v/a/s PackList init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PACKET_LIST_INIT_FAIL);
		}

		vidclk.init_clock(&videoq);
		audclk.init_clock(&audioq);
		extclk.init_clock(&subtitleq);

		nAudio_clock_serial = -1;
		if (startup_volume < 0)
			qDebug() << QString("volume=%1 < 0, setting to 0\n").arg(startup_volume);
		if (startup_volume > 100)
			qDebug() << QString("volume=%1 < 100, setting to 100\n").arg(startup_volume);
		startup_volume = av_clip(startup_volume, 0, 100);
		startup_volume = av_clip(SDL_MIX_MAXVOLUME * startup_volume / 100, 0, SDL_MIX_MAXVOLUME);
		nAudio_volume = startup_volume;
		bMuted = false;
		nAv_sync_type = AV_SYNC_AUDIO_MASTER;

		std::thread t1(&PlayerDisplay::read_thread,this);
		t1.join();
	}
	catch (const PlayerException& e) {
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		stream_close();
	}
	
	return 0;
}

void PlayerDisplay::stream_close()
{
	bAbort_request = 1;

	if (nAudio_stream >= 0)
		stream_component_close(nAudio_stream);
	if (nVideo_stream >= 0)
		stream_component_close(nVideo_stream);
	if (nSubtitle_stream >= 0)
		stream_component_close(nSubtitle_stream);

	avformat_close_input(&pFmtCtx);

	videoq.destroy();
	audioq.destroy();
	subtitleq.destroy();

	pictq.destory();
	sampq.destory();
	subpq.destory();

	sws_freeContext(pSub_convert_ctx);
	av_free(pFilename);
	av_free(this);
}

void PlayerDisplay::get_default_windowSize(int* width, int* height, AVRational* sar)
{
	AVStream* st = pFmtCtx->streams[nSt_index[AVMEDIA_TYPE_VIDEO]];
	AVCodecParameters* codecpar = st->codecpar;
	AVRational rational = av_guess_sample_aspect_ratio(pFmtCtx, st, NULL);
	if (codecpar->width)
	{	
		*width = codecpar->width;
		*height = codecpar->height;
		memcpy(sar, &rational,sizeof(rational));
	}
}

void PlayerDisplay::stream_component_close(int stream_index)
{
	AVCodecParameters* codecpar;

	if (stream_index < 0 || stream_index >= pFmtCtx->nb_streams)
		return;
	codecpar = pFmtCtx->streams[stream_index]->codecpar;

	pFmtCtx->streams[stream_index]->discard = AVDISCARD_ALL;

	auto func = getHandler(codecpar->codec_type);
	func();
}

int PlayerDisplay::stream_component_open(int stream_index)
{
	AVCodecContext* avctx;
	const AVCodec* codec;
	const char* forced_codec_name = nullptr;
	AVDictionary* opts = nullptr;
	const AVDictionaryEntry* t = nullptr;
	AVChannelLayout ch_layout;
	memset(&ch_layout, 0, sizeof(AVChannelLayout));


	int sample_rate;
	int ret = 0;
	int stream_lowres = lowres;

	try {
		if (stream_index < 0 || stream_index >= pFmtCtx->nb_streams)
			return -1;

		avctx = avcodec_alloc_context3(NULL);
		if (!avctx)
			return AVERROR(ENOMEM);

		ret = avcodec_parameters_to_context(avctx, pFmtCtx->streams[stream_index]->codecpar);
		if (ret < 0)
		{
			QString error = QString("%1:%2 avcodec_parameters_to_context failed.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PARAMETERS_TO_CONTEXT_FAIL);
		}

		avctx->pkt_timebase = pFmtCtx->streams[stream_index]->time_base;

		codec = avcodec_find_decoder(avctx->codec_id);

		switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO: 
		{
			nLast_audio_stream = stream_index; 
			forced_codec_name = audio_codec_name; 
			break;
		}
		case AVMEDIA_TYPE_SUBTITLE: {
			nLast_subtitle_stream = stream_index; 
			forced_codec_name = subtitle_codec_name; 
			break;
		}
		case AVMEDIA_TYPE_VIDEO: 
		{
			nLast_video_stream = stream_index; 
			forced_codec_name = video_codec_name; 
			break;
		}
		}
		if (forced_codec_name)
			codec = avcodec_find_decoder_by_name(forced_codec_name);
		if (!codec) {
			QString error;
			if (forced_codec_name)
			{
				error = QString("%1:%2 No codec could be found with name '%3'.\n").arg(__FILE__).arg(__LINE__)
					.arg(forced_codec_name);
			}
			else
			{
				error = QString("%1:%2 No decoder could be found for codec '%3'.\n").arg(__FILE__).arg(__LINE__)
					.arg(avcodec_get_name(avctx->codec_id));
			}
			ret = AVERROR(EINVAL);
			throw PlayerException(error.toStdString(), DECODEC_BY_NAME_FAIL);
		}

		avctx->codec_id = codec->id;
		if (stream_lowres > codec->max_lowres) {
			QString error = QString("%1:%2 The maximum value for lowres supported by the decoder is '%3'.\n").arg(__FILE__).arg(__LINE__)
				.arg(codec->max_lowres);
			stream_lowres = codec->max_lowres;
		}
		avctx->lowres = stream_lowres;

		if (fast)
			avctx->flags2 |= AV_CODEC_FLAG2_FAST;

		opts = filter_codec_opts(codec_opts, avctx->codec_id, pFmtCtx, pFmtCtx->streams[stream_index], codec);
		if (!av_dict_get(opts, "threads", NULL, 0))
			av_dict_set(&opts, "threads", "auto", 0);
		if (stream_lowres)
			av_dict_set_int(&opts, "lowres", stream_lowres, 0);

		av_dict_set(&opts, "flags", "+copy_opaque", AV_DICT_MULTIKEY);

		if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
			QString error = QString("%1:%2 avcodec open failed.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), DECODEC_OPEN_FAIL);
		}
		if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
			ret = AVERROR_OPTION_NOT_FOUND;
			QString error = QString("%1:%2 Option %s not found '%3'.\n").arg(__FILE__).arg(__LINE__).arg(t->key);
			throw PlayerException(error.toStdString(), DECODEC_OPEN_FAIL);
		}

		nEof = 0;
		pFmtCtx->streams[stream_index]->discard = AVDISCARD_DEFAULT;
		switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
		{
			AVFilterContext* sink;

			struAudio_filter_src.freq = avctx->sample_rate;
			ret = av_channel_layout_copy(&struAudio_filter_src.ch_layout, &avctx->ch_layout);
			if (ret < 0)
			{
				QString error = QString("%1:%2 channel layout copy failed '%3'.\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), CHANNAL_LAYOUT_COPY_FAIL);
			}
			struAudio_filter_src.fmt = avctx->sample_fmt;
			if ((ret = configure_audio_filters(afilters, 0)) < 0)
			{
				QString error = QString("%1:%2 set audio filters configure fail\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), AUDIO_FILTER_CONFIGURE_FAIL);
			}
			sink = pOut_audio_filter;
			sample_rate = av_buffersink_get_sample_rate(sink);
			ret = av_buffersink_get_ch_layout(sink, &ch_layout);
			if (ret < 0)
			{
				QString error = QString("%1:%2 get channal layout by buffersink failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), GET_CHANNAL_LAYOUT_FAIL);
			}
		}

		if ((ret = audio_open(&ch_layout, sample_rate, &struAudio_tgt)) < 0)
		{
			QString error = QString("%1:%2 audio open failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), AUDIO_OPEN_FAIL);
		}
		nAudio_hw_buf_size = ret;
		struAudio_src = struAudio_tgt;
		nAudio_buf_size = 0;
		nAudio_buf_index = 0;

		nAudio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
		nAudio_diff_avg_count = 0;

		nAudio_diff_threshold = (double)(nAudio_hw_buf_size) / struAudio_tgt.bytes_per_sec;

		nAudio_stream = stream_index;
		audio_st = pFmtCtx->streams[stream_index];

		if ((ret = auddec.decoder_init(avctx, &audioq, &cond)) < 0)
		{
			QString error = QString("%1:%2 Decode init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), DECODER_INIT_FAIL);
		}
		if ((pFmtCtx->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !pFmtCtx->iformat->read_seek) {
			auddec.set_start_pts(audio_st->start_time);
			auddec.set_start_pts_tb(audio_st->time_base);
		}
		if ((ret = auddec.decoder_start(&PlayerDisplay::audio_thread, *this)) < 0)
		{
			QString error = QString("%1:%2 Decode Start failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), DECODER_START_FAIL);
		}
		SDL_PauseAudioDevice(audio_dev, 0);
		break;
		case AVMEDIA_TYPE_VIDEO:
			nVideo_stream = stream_index;
			video_st = pFmtCtx->streams[stream_index];

			if ((ret = viddec.decoder_init(avctx, &videoq,&cond)) < 0)
			{
				QString error = QString("%1:%2 Decode init failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), DECODER_INIT_FAIL);
			}
			if ((ret = viddec.decoder_start(&PlayerDisplay::video_thread,*this)) < 0)
			{
				QString error = QString("%1:%2 Decode Start failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), DECODER_START_FAIL);
			}
			nAttachments_req = 1;
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			nSubtitle_stream = stream_index;
			subtitle_st = pFmtCtx->streams[stream_index];

			if ((ret = subdec.decoder_init(avctx, &subtitleq, &cond)) < 0)
			{
				QString error = QString("%1:%2 Decode init failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), DECODER_INIT_FAIL);
			}
			if ((ret = subdec.decoder_start(&PlayerDisplay::subtitle_thread,*this)) < 0)
			{
				QString error = QString("%1:%2 Decode Start failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), DECODER_START_FAIL);
			}
			break;
		default:
			break;
		}
	}
	catch (PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		if (avctx) {
			avcodec_free_context(&avctx);
		}
		av_channel_layout_uninit(&ch_layout);
		av_dict_free(&opts);
		return ret;
	}
	
	av_channel_layout_uninit(&ch_layout);
	av_dict_free(&opts);
	return ret;
	
}

int PlayerDisplay::check_stream_specifier(AVFormatContext* s, AVStream* st, const char* spec)
{
	int ret = avformat_match_stream_specifier(s, st, spec);
	if (ret < 0)
		qDebug() << QString("Invalid stream specifier: %1.\n").arg(spec);
	return ret;
}

AVDictionary* PlayerDisplay::filter_codec_opts(AVDictionary* opts, enum AVCodecID codec_id, AVFormatContext* s, AVStream* st, const AVCodec* codec)
{
	AVDictionary* ret = NULL;
	const AVDictionaryEntry* t = NULL;
	int            flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
		: AV_OPT_FLAG_DECODING_PARAM;
	char          prefix = 0;
	const AVClass* cc = avcodec_get_class();

	if (!codec)
		codec = s->oformat ? avcodec_find_encoder(codec_id)
		: avcodec_find_decoder(codec_id);

	switch (st->codecpar->codec_type) {
	case AVMEDIA_TYPE_VIDEO:
		prefix = 'v';
		flags |= AV_OPT_FLAG_VIDEO_PARAM;
		break;
	case AVMEDIA_TYPE_AUDIO:
		prefix = 'a';
		flags |= AV_OPT_FLAG_AUDIO_PARAM;
		break;
	case AVMEDIA_TYPE_SUBTITLE:
		prefix = 's';
		flags |= AV_OPT_FLAG_SUBTITLE_PARAM;
		break;
	}

	while (t = av_dict_iterate(opts, t)) {
		const AVClass* priv_class;
		char* p = strchr(t->key, ':');

		if (p)
			switch (check_stream_specifier(s, st, p + 1)) {
			case  1: *p = 0; break;
			case  0:         continue;
			default:         exit(-1);
			}

		if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
			!codec ||
			((priv_class = codec->priv_class) &&
				av_opt_find(&priv_class, t->key, NULL, flags,
					AV_OPT_SEARCH_FAKE_OBJ)))
			av_dict_set(&ret, t->key, t->value, 0);
		else if (t->key[0] == prefix &&
			av_opt_find(&cc, t->key + 1, NULL, flags,
				AV_OPT_SEARCH_FAKE_OBJ))
			av_dict_set(&ret, t->key + 1, t->value, 0);

		if (p)
			*p = ':';
	}
	return ret;
}

AVDictionary** PlayerDisplay::setup_find_stream_info_opts(AVFormatContext* s, AVDictionary* codec_opts)
{
	int i;
	AVDictionary** opts;

	if (!s->nb_streams)
		return NULL;
	opts = (AVDictionary**)av_calloc(s->nb_streams, sizeof(*opts));
	if (!opts)
	{
		int ret = AVERROR(ENOMEM);
		qDebug() << "AVDictionary opts alloc failed";
		exit(ret);
	}
	for (i = 0; i < s->nb_streams; i++)
		opts[i] = filter_codec_opts(codec_opts, s->streams[i]->codecpar->codec_id,
			s, s->streams[i], NULL);
	return opts;
}

int PlayerDisplay::configure_filtergraph(AVFilterGraph* graph, const char* filtergraph, AVFilterContext* source_ctx, AVFilterContext* sink_ctx)
{
	int ret, i;
	int nb_filters = graph->nb_filters;
	AVFilterInOut* outputs = nullptr;
	AVFilterInOut* inputs = nullptr;

	try
	{
		if (filtergraph) {
			outputs = avfilter_inout_alloc();
			inputs = avfilter_inout_alloc();
			if (!outputs || !inputs) {
				ret = AVERROR(ENOMEM);
				QString error = QString("%1:%2 filter inout alloc failed: %3\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), FILTER_INPUT_CRATER_FAIL);
			}

			outputs->name = av_strdup("in");
			outputs->filter_ctx = source_ctx;
			outputs->pad_idx = 0;
			outputs->next = NULL;

			inputs->name = av_strdup("out");
			inputs->filter_ctx = sink_ctx;
			inputs->pad_idx = 0;
			inputs->next = NULL;

			//解析滤镜图（filtergraph）字符串并将其添加到现有的滤镜图对象中
			if ((ret = avfilter_graph_parse_ptr(graph, filtergraph, &inputs, &outputs, NULL)) < 0)
			{
				QString error = QString("%1:%2 filter graph parse failed: %3\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), FILTER_PARSE_FAIL);
			}
		}
		else {
			//创建两个滤镜之间的连接
			if ((ret = avfilter_link(source_ctx, 0, sink_ctx, 0)) < 0)
			{
				QString error = QString("%1:%2 filter link failed: %3\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), FILTER_LINK_FAIL);
			}
		}

		for (i = 0; i < graph->nb_filters - nb_filters; i++)
			FFSWAP(AVFilterContext*, graph->filters[i], graph->filters[i + nb_filters]);

		ret = avfilter_graph_config(graph, NULL);
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	avfilter_inout_free(&outputs);
	avfilter_inout_free(&inputs);
	return ret;
}

int PlayerDisplay::configure_audio_filters(const char* afilters, int force_output_format)
{
	int ret;
	static const std::vector<AVSampleFormat> sample_fmts = { AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_NONE };
	std::vector<int> sample_rates = { 0, -1 };
	AVFilterContext* filt_asrc = nullptr, * filt_asink = nullptr;
	char aresample_swr_opts[512] = "";
	const AVDictionaryEntry* e = NULL;
	AVBPrint bp;
	char asrc_args[256];

	try {
		avfilter_graph_free(&pAgraph);
		if (!(pAgraph = avfilter_graph_alloc()))
			return AVERROR(ENOMEM);
		pAgraph->nb_threads = filter_nbthreads;

		av_bprint_init(&bp, 0, AV_BPRINT_SIZE_AUTOMATIC);

		//遍历字典（dictionary）中的所有条目
		while ((e = av_dict_iterate(swr_opts, e)))
			av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
		if (strlen(aresample_swr_opts))
			aresample_swr_opts[strlen(aresample_swr_opts) - 1] = '\0';
		av_opt_set(pAgraph, "aresample_swr_opts", aresample_swr_opts, 0);

		av_channel_layout_describe_bprint(&struAudio_filter_src.ch_layout, &bp);

		ret = snprintf(asrc_args, sizeof(asrc_args),
			"sample_rate=%d:sample_fmt=%s:time_base=%d/%d:channel_layout=%s",
			struAudio_filter_src.freq, av_get_sample_fmt_name(struAudio_filter_src.fmt),
			1, struAudio_filter_src.freq, bp.str);

		//abuffer创建音频缓冲区滤镜
		ret = avfilter_graph_create_filter(&filt_asrc,
			avfilter_get_by_name("abuffer"), "ffplay_abuffer",
			asrc_args, NULL, pAgraph);
		if (ret < 0)
		{
			QString error = QString("%1:%2 create graph filter failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), CREATE_GRAPH_FILTER_FAIL);
		}

		//abuffersink创建一个音频缓冲区接收器
		ret = avfilter_graph_create_filter(&filt_asink,
			avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
			NULL, NULL, pAgraph);
		if (ret < 0)
		{
			QString error = QString("%1:%2 create graph filter failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), CREATE_GRAPH_FILTER_FAIL);
		}

		if ((ret = av_opt_set_int_list(filt_asink, "sample_fmts", sample_fmts.data(), AV_SAMPLE_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
		{
			QString error = QString("%1:%2 opt set failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), OPTIONAL_SET_FAIL);
		}
		if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN)) < 0)
		{
			QString error = QString("%1:%2 opt set failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), OPTIONAL_SET_FAIL);
		}

		if (force_output_format) {
			sample_rates[0] = struAudio_tgt.freq;
			if ((ret = av_opt_set_int(filt_asink, "all_channel_counts", 0, AV_OPT_SEARCH_CHILDREN)) < 0)
			{
				QString error = QString("%1:%2 opt set failed: %3\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), OPTIONAL_SET_FAIL);
			}
			if ((ret = av_opt_set(filt_asink, "ch_layouts", bp.str, AV_OPT_SEARCH_CHILDREN)) < 0)
			{
				QString error = QString("%1:%2 opt set failed: %3\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), OPTIONAL_SET_FAIL);
			}
			if ((ret = av_opt_set_int_list(filt_asink, "sample_rates", sample_rates.data(), -1, AV_OPT_SEARCH_CHILDREN)) < 0)
			{
				QString error = QString("%1:%2 opt set failed: %3\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), OPTIONAL_SET_FAIL);
			}
		}


		if ((ret = configure_filtergraph(pAgraph, afilters, filt_asrc, filt_asink)) < 0)
		{
			QString error = QString("%1:%2 configure filtergraph failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), FILTERGRAPH_CONFIGURE_FAIL);
		}

		pIn_audio_filter = filt_asrc;
		pOut_audio_filter = filt_asink;
	}
	catch (PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		if (ret < 0)
			avfilter_graph_free(&pAgraph);
	}
	
	av_bprint_finalize(&bp, NULL);
	return ret;
}

int PlayerDisplay::is_realtime(AVFormatContext* s)
{
	if (!strcmp(s->iformat->name, "rtp")
		|| !strcmp(s->iformat->name, "rtsp")
		|| !strcmp(s->iformat->name, "sdp")
		)
		return 1;

	if (s->pb && (!strncmp(s->url, "rtp:", 4)
		|| !strncmp(s->url, "udp:", 4)
		)
		)
		return 1;
	return 0;
}

int PlayerDisplay::get_master_sync_type()
{
	if (nAv_sync_type == AV_SYNC_VIDEO_MASTER) {
		if (video_st)
			return AV_SYNC_VIDEO_MASTER;
		else
			return AV_SYNC_AUDIO_MASTER;
	}
	else if (nAv_sync_type == AV_SYNC_AUDIO_MASTER) {
		if (audio_st)
			return AV_SYNC_AUDIO_MASTER;
		else
			return AV_SYNC_EXTERNAL_CLOCK;
	}
	else {
		return AV_SYNC_EXTERNAL_CLOCK;
	}
}

double PlayerDisplay::get_master_clock()
{
	double val;

	switch (get_master_sync_type()) {
	case AV_SYNC_VIDEO_MASTER:
		val = vidclk.get_clock();
		break;
	case AV_SYNC_AUDIO_MASTER:
		val = audclk.get_clock();
		break;
	default:
		val = extclk.get_clock();
		break;
	}
	return val;
}

void PlayerDisplay::audioStreamClose()
{
	auddec.decoder_abort(&sampq);
	auddec.decoder_destroy();
	swr_free(&pSwr_ctx);
	av_freep(&pAudio_buf1);
	nAudio_buf1_size = 0;
	pAudio_buf = nullptr;

	if (pRdft) {
		av_rdft_end(pRdft);
		av_freep(&pRdft_data);
		pRdft = nullptr;
		nRdft_bits = 0;
	}

	audio_st = NULL;
	nAudio_stream = -1;
}

void PlayerDisplay::videoStreamClose()
{
	viddec.decoder_abort(&pictq);
	viddec.decoder_destroy();
	video_st = NULL;
	nVideo_stream = -1;
}

void PlayerDisplay::subtitleStreamClose()
{
	subdec.decoder_abort(&subpq);
	subdec.decoder_destroy();
	subtitle_st = NULL;
	nSubtitle_stream = -1;
}

streamClosehandler PlayerDisplay::getHandler(int codec_type)
{
	auto it = um_stCloseHandlerMap.find(codec_type);
	if (it == um_stCloseHandlerMap.end())
	{
		return[=]() {
			qDebug() << __FILE__ << ":" << __LINE__ << "codec_type:" << codec_type << "can not find handler!";
			};

	}
	return um_stCloseHandlerMap[codec_type];
}

int PlayerDisplay::stream_has_enough_packets(AVStream* st, int stream_id, AvPacketList* list)
{
	return stream_id < 0 ||
		list->get_abort_request() ||
		(st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
		list->get_nb_packets() > MIN_FRAMES && 
		(!list->get_duration() || av_q2d(st->time_base) * list->get_duration() > 1.0);
}

void PlayerDisplay::stream_seek(int64_t pos, int64_t rel, int by_bytes)
{
	if (!seek_req) {
		seek_pos = pos;
		nSeek_rel = rel;
		seek_flags &= ~AVSEEK_FLAG_BYTE;
		if (by_bytes)
			seek_flags |= AVSEEK_FLAG_BYTE;
		seek_req = 1;
		cond.notify_all();
	}
}

void PlayerDisplay::stream_toggle_pause()
{
	if (nPaused) {
		int last_updated = vidclk.get_last_update();
		nFrame_timer += av_gettime_relative() / 1000000.0 - last_updated;
		if (nRead_pause_return != AVERROR(ENOSYS)) {
			vidclk.set_pause(0);
		}
		vidclk.set_clock(vidclk.get_clock(), vidclk.get_serial());
	}
	extclk.set_clock(extclk.get_clock(), extclk.get_serial());
	nPaused = nPaused ? 0 : 1;
	audclk.set_pause(nPaused);
	vidclk.set_pause(nPaused);
	extclk.set_pause(nPaused);
}

void PlayerDisplay::step_to_next_frame()
{
	if (nPaused)
		stream_toggle_pause();
	nStep = 1;
}

void PlayerDisplay::insert_filt(AVFilterContext*& last_filter, AVFilterGraph* graph, const char* name, const char* arg)
{
	int ret;
	AVFilterContext* filt_ctx;

	try {
		ret = avfilter_graph_create_filter(&filt_ctx, avfilter_get_by_name(name), "ffplay_", arg, NULL, graph);
		if (ret < 0) 
		{
			QString error = QString("create graph filter failed\n");
			throw PlayerException(error.toStdString(), CREATE_GRAPH_FILTER_FAIL);
		}

		ret = avfilter_link(filt_ctx, 0, last_filter, 0);
		if (ret < 0) 
		{
			QString error = QString("filter link failed\n");
			throw PlayerException(error.toStdString(), FILTER_LINK_FAIL);
		}

		last_filter = filt_ctx;
	}
	catch (const PlayerException& e) {
		throw; // 重新抛出异常，以便调用者可以处理
	}
}

double PlayerDisplay::av_display_rotation_get(const int32_t matrix[9])
{
	double rotation, scale[2];

	scale[0] = hypot(CONV_FP(matrix[0]), CONV_FP(matrix[3]));
	scale[1] = hypot(CONV_FP(matrix[1]), CONV_FP(matrix[4]));

	if (scale[0] == 0.0 || scale[1] == 0.0)
		return NAN;

	rotation = atan2(CONV_FP(matrix[1]) / scale[1],
		CONV_FP(matrix[0]) / scale[0]) * 180 / M_PI;

	return -rotation;
}

void PlayerDisplay::av_display_rotation_set(int32_t matrix[9], double angle)
{
	double radians = -angle * M_PI / 180.0f;
	double c = cos(radians);
	double s = sin(radians);

	memset(matrix, 0, 9 * sizeof(int32_t));

	matrix[0] = CONV_DB(c);
	matrix[1] = CONV_DB(-s);
	matrix[3] = CONV_DB(s);
	matrix[4] = CONV_DB(c);
	matrix[8] = 1 << 30;
}

double PlayerDisplay::get_rotation(int32_t* displaymatrix)
{
	double theta = 0;
	if (displaymatrix)
		theta = -round(av_display_rotation_get((int32_t*)displaymatrix));

	theta -= 360 * floor(theta / 360 + 0.9 / 360);

	if (fabs(theta - 90 * round(theta / 90)) > 2)
		qDebug() << "Odd rotation angle.\n";

	return theta;
}

int PlayerDisplay::configure_video_filters(AVFilterGraph* graph, const char* vfilters, AVFrame* frame)
{
	int ret;
	enum AVPixelFormat pix_fmts[FF_ARRAY_ELEMS(sdl_texture_format_map)];
	char sws_flags_str[512] = "";
	char buffersrc_args[256];

	AVFilterContext* filt_src = nullptr;
	AVFilterContext* filt_out = nullptr;
	AVFilterContext* last_filter = nullptr;

	const AVDictionaryEntry* e = nullptr;
	try
	{
		AVCodecParameters* codecpar = video_st->codecpar;
		AVRational fr = av_guess_frame_rate(pFmtCtx, video_st, NULL);
		
		int nb_pix_fmts = 0;
		int i, j;

		for (i = 0; i < renderer_info.num_texture_formats; i++) {
			for (j = 0; j < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; j++) {
				if (renderer_info.texture_formats[i] == sdl_texture_format_map[j].texture_fmt) {
					pix_fmts[nb_pix_fmts++] = sdl_texture_format_map[j].format;
					break;
				}
			}
		}
		pix_fmts[nb_pix_fmts] = AV_PIX_FMT_NONE;

		while ((e = av_dict_iterate(sws_dict, e))) {
			if (!strcmp(e->key, "sws_flags")) {
				av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", "flags", e->value);
			}
			else
				av_strlcatf(sws_flags_str, sizeof(sws_flags_str), "%s=%s:", e->key, e->value);
		}
		if (strlen(sws_flags_str))
			sws_flags_str[strlen(sws_flags_str) - 1] = '\0';

		graph->scale_sws_opts = av_strdup(sws_flags_str);

		snprintf(buffersrc_args, sizeof(buffersrc_args),
			"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
			frame->width, frame->height, frame->format,
			video_st->time_base.num, video_st->time_base.den,
			codecpar->sample_aspect_ratio.num, FFMAX(codecpar->sample_aspect_ratio.den, 1));
		if (fr.num && fr.den)
			av_strlcatf(buffersrc_args, sizeof(buffersrc_args), ":frame_rate=%d/%d", fr.num, fr.den);

		//"buffer"在滤镜图中创建一个输入缓冲区
		if ((ret = avfilter_graph_create_filter(&filt_src,
			avfilter_get_by_name("buffer"),
			"ffplay_buffer", buffersrc_args, NULL,
			graph)) < 0)
		{
			QString error = QString("create graph filter failed\n");
			throw PlayerException(error.toStdString(), CREATE_GRAPH_FILTER_FAIL);
		}

		ret = avfilter_graph_create_filter(&filt_out,
			avfilter_get_by_name("buffersink"),
			"ffplay_buffersink", NULL, NULL, graph);
		if (ret < 0)
		{
			QString error = QString("create graph filter failed\n");
			throw PlayerException(error.toStdString(), CREATE_GRAPH_FILTER_FAIL);
		}

		if ((ret = av_opt_set_int_list(filt_out, "pix_fmts", pix_fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN)) < 0)
		{
			QString error = QString("option set failed\n");
			throw PlayerException(error.toStdString(), OPTIONAL_SET_FAIL);
		}

		last_filter = filt_out;

		if (autorotate) {
			double theta = 0.0;
			int32_t* displaymatrix = NULL;
			AVFrameSideData* sd = av_frame_get_side_data(frame, AV_FRAME_DATA_DISPLAYMATRIX);
			if (sd)
				displaymatrix = (int32_t*)sd->data;
			if (!displaymatrix)
				displaymatrix = (int32_t*)av_stream_get_side_data(video_st, AV_PKT_DATA_DISPLAYMATRIX, NULL);
			theta = get_rotation(displaymatrix);

			if (fabs(theta - 90) < 1.0) {//顺时针旋转
				insert_filt(last_filter, graph,"transpose", "clock");
			}
			else if (fabs(theta - 180) < 1.0) {//水平垂直
				insert_filt(last_filter,graph,"hflip", NULL);
				insert_filt(last_filter, graph,"vflip", NULL);
			}
			else if (fabs(theta - 270) < 1.0) {//逆时针旋转
				insert_filt(last_filter, graph,"transpose", "cclock");
			}
			else if (fabs(theta) > 1.0) {//旋转特定角度
				char rotate_buf[64];
				snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
				insert_filt(last_filter, graph,"rotate", rotate_buf);
			}
		}

		if ((ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0)
		{
			QString error = QString("filtergraph configure failed\n");
			throw PlayerException(error.toStdString(), FILTERGRAPH_CONFIGURE_FAIL);
		}

		pIn_video_filter = filt_src;
		pOut_video_filter = filt_out;
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}

	return ret;
}

int PlayerDisplay::queue_picture(AVFrame* src_frame, double pts, double duration, int64_t pos, int serial)
{
	Frame* vp;

	if (!(vp = pictq.frame_queue_peek_writable()))
		return -1;

	vp->sar = src_frame->sample_aspect_ratio;
	vp->uploaded = 0;

	vp->width = src_frame->width;
	vp->height = src_frame->height;
	vp->format = src_frame->format;

	vp->pts = pts;
	vp->duration = duration;
	vp->pos = pos;
	vp->serial = serial;

	av_frame_move_ref(vp->frame, src_frame);
	pictq.frame_queue_push();
	return 0;
}

int PlayerDisplay::get_video_frame(AVFrame* frame)
{
	int got_picture;

	if ((got_picture = viddec.decoder_decode_frame(frame, NULL)) < 0)
		return -1;

	if (got_picture) {
		//解码后的显示时间戳
		double dpts = NAN;

		if (frame->pts != AV_NOPTS_VALUE)
			//dpts=PTS×time_base
			dpts = av_q2d(video_st->time_base) * frame->pts;

		//猜测视频帧的样本宽高比
		frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(pFmtCtx, video_st, frame);

		if (framedrop > 0 || (framedrop && get_master_sync_type() != AV_SYNC_VIDEO_MASTER)) {
			if (frame->pts != AV_NOPTS_VALUE) {
				double diff = dpts - get_master_clock();
				if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
					diff - nFrame_last_filter_delay < 0 &&
					viddec.get_serial() == vidclk.get_serial() &&
					videoq.get_nb_packets()
					) 
				{
					nFrame_drops_early++;
					av_frame_unref(frame);
					got_picture = 0;
				}
			}
		}
	}

	return got_picture;
}

int PlayerDisplay::synchronize_audio(int nb_samples)
{
	int wanted_nb_samples = nb_samples;

	//如果获取主时钟类型不是音频为主时钟
	if (get_master_sync_type() != AV_SYNC_AUDIO_MASTER) {
		double diff, avg_diff;
		int min_nb_samples, max_nb_samples;

		diff = audclk.get_clock() - get_master_clock();

		if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
			//引入 nAudio_diff_avg_coef，可以对音频时间差进行平滑处理，减少瞬时波动对累积值的影响
			//通过不断累积音频时间差，可以更好地跟踪音频和视频之间的长期同步情况
			//使用指数加权平均法可以给最近的音频时间差赋予更高的权重
			nAudio_diff_cum = diff + nAudio_diff_avg_coef * nAudio_diff_cum;
			if (nAudio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
				nAudio_diff_avg_count++;
			}
			else {
				//计算平均音频时间差avg_diff,通过累积音频时间差 nAudio_diff_cum 乘以 (1.0 - nAudio_diff_avg_coef)
				avg_diff = nAudio_diff_cum * (1.0 - nAudio_diff_avg_coef);

				if (fabs(avg_diff) >= nAudio_diff_threshold) {
					//根据音频时间差 diff 和音频源的采样率 struAudio_src.freq，计算所需的音频样本数量 wanted_nb_samples。
					wanted_nb_samples = nb_samples + (int)(diff * struAudio_src.freq);
					//计算最小和最大允许的音频样本数量 min_nb_samples 和 max_nb_samples
					min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					//使用 av_clip 函数将 wanted_nb_samples 限制在这个范围内
					wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
				}
				QString error = QString("diff=%1 adiff=%2 sample_diff=%3 apts=%4 %5\n")
					.arg(diff).arg(avg_diff).arg(wanted_nb_samples - nb_samples)
					.arg(nAudio_clock).arg(nAudio_diff_threshold);
				qDebug() << error;
			}
		}
		else {
			nAudio_diff_avg_count = 0;
			nAudio_diff_cum = 0;
		}
	}

	return wanted_nb_samples;
}

void PlayerDisplay::update_sample_display(short* samples, int samples_size)
{
	int size, len;

	size = samples_size / sizeof(short);
	while (size > 0) {
		len = SAMPLE_ARRAY_SIZE - nSample_array_index;
		if (len > size)
			len = size;
		memcpy(sample_array + nSample_array_index, samples, len * sizeof(short));
		samples += len;
		nSample_array_index += len;
		if (nSample_array_index >= SAMPLE_ARRAY_SIZE)
			nSample_array_index = 0;
		size -= len;
	}
}

int PlayerDisplay::audio_decode_frame()
{
	int data_size, resampled_data_size;
	av_unused double audio_clock0;
	int wanted_nb_samples;
	Frame* af;

	if (nPaused)
		return -1;

	do {
		//检查当前时间和上次音频回调时间之间的差值,确保音频数据能够及时填充到音频缓冲区
#if defined(_WIN32)
		while (sampq.frame_queue_nb_remaining() == 0) {
			//如果这个差值超过了音频缓冲区大小的一半所对应的时间
			//1000000LL * nAudio_hw_buf_size / struAudio_tgt.bytes_per_sec / 2
			// 允许的最大时间差是音频缓冲区大小对应时间的一半。
			if ((av_gettime_relative() - audio_callback_time) > 1000000LL * nAudio_hw_buf_size / struAudio_tgt.bytes_per_sec / 2)
				return -1;
			av_usleep(1000);
		}
#endif
		if (!(af = sampq.frame_queue_peek_readable()))
			return -1;
		sampq.frame_queue_next();
	} while (af->serial != audioq.get_serial());

	data_size = av_samples_get_buffer_size(NULL, af->frame->ch_layout.nb_channels,
		af->frame->nb_samples,
		(AVSampleFormat)af->frame->format, 1);

	wanted_nb_samples = synchronize_audio(af->frame->nb_samples);

	if (af->frame->format != struAudio_src.fmt ||
		av_channel_layout_compare(&af->frame->ch_layout, &struAudio_src.ch_layout) ||
		af->frame->sample_rate != struAudio_src.freq ||
		(wanted_nb_samples != af->frame->nb_samples && !pSwr_ctx)) {
		swr_free(&pSwr_ctx);
		swr_alloc_set_opts2(&pSwr_ctx,
			&struAudio_tgt.ch_layout, struAudio_tgt.fmt, struAudio_tgt.freq,
			&af->frame->ch_layout, (AVSampleFormat)af->frame->format, af->frame->sample_rate,
			0, NULL);
		if (!pSwr_ctx || swr_init(pSwr_ctx) < 0) {
			QString error = QString("Cannot create sample rate converter for conversion of %1 Hz %2 %3 channels to %4 Hz %5 %6 channels!\n")
				.arg(af->frame->sample_rate).arg(av_get_sample_fmt_name((AVSampleFormat)af->frame->format)).arg(af->frame->ch_layout.nb_channels)
				.arg(struAudio_tgt.freq).arg(av_get_sample_fmt_name(struAudio_tgt.fmt)).arg(struAudio_tgt.ch_layout.nb_channels);
			qDebug() << error;
			swr_free(&pSwr_ctx);
			return -1;
		}
		if (av_channel_layout_copy(&struAudio_src.ch_layout, &af->frame->ch_layout) < 0)
			return -1;
		struAudio_src.freq = af->frame->sample_rate;
		struAudio_src.fmt = (AVSampleFormat)af->frame->format;
	}

	if (pSwr_ctx) {
		const uint8_t** in = (const uint8_t**)af->frame->extended_data;
		uint8_t** out = &pAudio_buf1;
		int out_count = (int64_t)wanted_nb_samples * struAudio_tgt.freq / af->frame->sample_rate + 256;
		int out_size = av_samples_get_buffer_size(NULL, struAudio_tgt.ch_layout.nb_channels, out_count, struAudio_tgt.fmt, 0);
		int len2;
		if (out_size < 0) {
			QString error = QString("av_samples_get_buffer_size() failed\n");
			qDebug() << error;
			return -1;
		}
		if (wanted_nb_samples != af->frame->nb_samples) {
			//设置音频重采样器的补偿参数,处理音频播放中的同步问题
			if (swr_set_compensation(pSwr_ctx, (wanted_nb_samples - af->frame->nb_samples) * struAudio_tgt.freq / af->frame->sample_rate,
				wanted_nb_samples * struAudio_tgt.freq / af->frame->sample_rate) < 0) {
				QString error = QString("swr_set_compensation() failed\n");
				qDebug() << error;
				return -1;
			}
		}
		//快速分配内存 需要频繁分配和释放内存的场景避免内存碎片问题
		av_fast_malloc(&pAudio_buf1, &nAudio_buf1_size, out_size);
		if (!pAudio_buf1)
			return AVERROR(ENOMEM);
		//音频重采样操作 将输入音频数据从一种格式转换为另一种格式
		len2 = swr_convert(pSwr_ctx, out, out_count, in, af->frame->nb_samples);
		if (len2 < 0) {
			QString error = QString("swr_convert() failed\n");
			qDebug() << error;
			return -1;
		}
		if (len2 == out_count) {
			QString error = QString("audio buffer is probably too small\n");
			qDebug() << error;
			if (swr_init(pSwr_ctx) < 0)
				swr_free(&pSwr_ctx);
		}
		pAudio_buf = pAudio_buf1;
		resampled_data_size = len2 * struAudio_tgt.ch_layout.nb_channels * av_get_bytes_per_sample(struAudio_tgt.fmt);
	}
	else {
		pAudio_buf = af->frame->data[0];
		resampled_data_size = data_size;
	}

	audio_clock0 = nAudio_clock;
	//更新音频时钟
	if (!isnan(af->pts))
		nAudio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
	else
		nAudio_clock = NAN;
	nAudio_clock_serial = af->serial;
	return resampled_data_size;
}

int PlayerDisplay::cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1, enum AVSampleFormat fmt2, int64_t channel_count2)
{
	if (channel_count1 == 1 && channel_count2 == 1)
		return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
	else
		return channel_count1 != channel_count2 || fmt1 != fmt2;
}

void PlayerDisplay::sdl_audio_callback(void* opaque, Uint8* stream, int len)
{
	PlayerDisplay* player = static_cast<PlayerDisplay*>(opaque);
	player->audio_callback(stream, len);
}

void PlayerDisplay::audio_callback(Uint8* stream, int len)
{
	int audio_size, len1;

	audio_callback_time = av_gettime_relative();

	while (len > 0) {
		if (nAudio_buf_index >= nAudio_buf_size) {
			audio_size = audio_decode_frame();
			if (audio_size < 0) {
				pAudio_buf = NULL;
				nAudio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / struAudio_tgt.frame_size * struAudio_tgt.frame_size;
			}
			else {
				if (show_mode != SHOW_MODE_VIDEO)
					update_sample_display((int16_t*)pAudio_buf, audio_size);
				nAudio_buf_size = audio_size;
			}
			nAudio_buf_index = 0;
		}
		len1 = nAudio_buf_size - nAudio_buf_index;
		if (len1 > len)
			len1 = len;
		if (!bMuted && pAudio_buf && nAudio_volume == SDL_MIX_MAXVOLUME)
			memcpy(stream, (uint8_t*)pAudio_buf + nAudio_buf_index, len1);
		else {
			memset(stream, 0, len1);
			if (!bMuted && pAudio_buf)
				//将音频数据混合到目标音频流
				SDL_MixAudioFormat(stream, (uint8_t*)pAudio_buf + nAudio_buf_index, AUDIO_S16SYS, len1, nAudio_volume);
		}
		len -= len1;
		stream += len1;
		nAudio_buf_index += len1;
	}
	nAudio_write_buf_size = nAudio_buf_size - nAudio_buf_index;
	if (!isnan(nAudio_clock)) {
		audclk.set_clock_at(nAudio_clock - (double)(2 * nAudio_hw_buf_size + nAudio_write_buf_size) / struAudio_tgt.bytes_per_sec, nAudio_clock_serial, audio_callback_time / 1000000.0);
		//时钟主从同步
		extclk.sync_clock_to_slave(&audclk);
	}
}

int PlayerDisplay::audio_open(AVChannelLayout* wanted_channel_layout, int wanted_sample_rate, struct AudioParams* audio_hw_params)
{
	SDL_AudioSpec wanted_spec, spec;
	const char* env;
	static const vector<int> next_nb_channels = { 0, 0, 1, 6, 2, 6, 4, 6 };
	static const vector<int> next_sample_rates = { 0, 44100, 48000, 96000, 192000 };
	int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) - 1;
	int wanted_nb_channels = wanted_channel_layout->nb_channels;

	env = SDL_getenv("SDL_AUDIO_CHANNELS");
	if (env) {
		wanted_nb_channels = std::atoi(env);
		av_channel_layout_uninit(wanted_channel_layout);
		av_channel_layout_default(wanted_channel_layout, wanted_nb_channels);
	}
	if (wanted_channel_layout->order != AV_CHANNEL_ORDER_NATIVE) {
		av_channel_layout_uninit(wanted_channel_layout);
		av_channel_layout_default(wanted_channel_layout, wanted_nb_channels);
	}
	wanted_nb_channels = wanted_channel_layout->nb_channels;
	wanted_spec.channels = wanted_nb_channels;
	wanted_spec.freq = wanted_sample_rate;
	if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
		QString error = QString("Invalid sample rate or channel count!\n");
		qDebug() << error;
		return -1;
	}
	while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
		next_sample_rate_idx--;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.silence = 0;
	wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
	wanted_spec.callback = sdl_audio_callback;
	wanted_spec.userdata = this;
	while (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
		QString warming = QString("SDL_OpenAudio (%1 channels, %2 Hz): %3\n")
			.arg(wanted_spec.channels).arg(wanted_spec.freq).arg(SDL_GetError());
		qDebug() << warming;
		wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
		if (!wanted_spec.channels) {
			wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
			wanted_spec.channels = wanted_nb_channels;
			if (!wanted_spec.freq) {
				QString error = QString("No more combinations to try, audio open failed\n");
				return -1;
			}
		}
		av_channel_layout_default(wanted_channel_layout, wanted_spec.channels);
	}
	if (spec.format != AUDIO_S16SYS) {
		QString error = QString("SDL advised audio format %1 is not supported!\n")
			.arg(spec.format);
		qDebug() << error;
		return -1;
	}
	if (spec.channels != wanted_spec.channels) {
		av_channel_layout_uninit(wanted_channel_layout);
		av_channel_layout_default(wanted_channel_layout, spec.channels);
		if (wanted_channel_layout->order != AV_CHANNEL_ORDER_NATIVE) {
			QString error = QString("SDL advised channel count %1 is not supported!\n")
				.arg(spec.channels);
			qDebug() << error;
			return -1;
		}
	}

	audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
	audio_hw_params->freq = spec.freq;
	if (av_channel_layout_copy(&audio_hw_params->ch_layout, wanted_channel_layout) < 0)
		return -1;
	audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->ch_layout.nb_channels, 1, audio_hw_params->fmt, 1);
	audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->ch_layout.nb_channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
	if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
		av_log(NULL, AV_LOG_ERROR, "av_samples_get_buffer_size failed\n");
		return -1;
	}
	return spec.size;
}

void PlayerDisplay::audio_thread()
{
	int ret = 0;
	AVFrame* frame = nullptr;
	Frame* af;
	int last_serial = -1;
	int reconfigure;
	int got_frame = 0;
	AVRational tb;
	
	try
	{
		AVFrame* frame = av_frame_alloc();
		if (!frame)
		{
			QString error = QString("frame alloc failed\n");
			throw PlayerException(error.toStdString(), FRAME_ALLOC_FAIL);
		}
		do {
			if ((got_frame = auddec.decoder_decode_frame(frame, nullptr)) < 0)
			{
				QString error = QString("decode frame failed\n");
				throw PlayerException(error.toStdString(), DECODER_DECODE_FAIL);
			}

			if (got_frame) {
				tb = { 1, frame->sample_rate };

				reconfigure =
					cmp_audio_fmts(struAudio_filter_src.fmt, struAudio_filter_src.ch_layout.nb_channels,
						(AVSampleFormat)frame->format, frame->ch_layout.nb_channels) ||
					av_channel_layout_compare(&struAudio_filter_src.ch_layout, &frame->ch_layout) ||
					struAudio_filter_src.freq != frame->sample_rate ||
					auddec.get_serial() != last_serial;

				if (reconfigure) {
					char buf1[1024], buf2[1024];
					av_channel_layout_describe(&struAudio_filter_src.ch_layout, buf1, sizeof(buf1));
					av_channel_layout_describe(&frame->ch_layout, buf2, sizeof(buf2));

					QString info = QString("Audio frame changed from rate:%1 ch:%2 fmt:%3 layout:%4 serial:%5 to rate:%6 ch:%7 fmt:%8 layout:%9 serial:%10\n")
						.arg(struAudio_filter_src.freq).arg(struAudio_filter_src.ch_layout.nb_channels).arg(av_get_sample_fmt_name(struAudio_filter_src.fmt))
						.arg(buf1).arg(last_serial).arg(frame->sample_rate).arg(frame->ch_layout.nb_channels).arg(av_get_sample_fmt_name((AVSampleFormat)frame->format))
						.arg(buf2).arg(auddec.get_serial());


					struAudio_filter_src.fmt = (AVSampleFormat)frame->format;
					ret = av_channel_layout_copy(&struAudio_filter_src.ch_layout, &frame->ch_layout);
					if (ret < 0)
					{
						QString error = QString("channel_layout copy failed\n");
						throw PlayerException(error.toStdString(), CHANNAL_LAYOUT_COPY_FAIL);
					}
					struAudio_filter_src.freq = frame->sample_rate;
					last_serial = auddec.get_serial();

					if ((ret = configure_audio_filters(afilters, 1)) < 0)
					{
						QString error = QString("audio filters configure failed\n");
						throw PlayerException(error.toStdString(), AUDIO_FILTER_CONFIGURE_FAIL);
					}
				}

				//视频帧或音频帧添加到缓冲区源滤镜
				if ((ret = av_buffersrc_add_frame(pIn_audio_filter, frame)) < 0)
				{
					QString error = QString("add frame to buffersrc failed\n");
					throw PlayerException(error.toStdString(), ADD_FRAME_FAIL);
				}

				//从缓冲区接收器滤镜（buffer sink filter）中获取一个视频帧或音频帧
				while ((ret = av_buffersink_get_frame_flags(pOut_audio_filter, frame, 0)) >= 0) {
					FrameData* fd = frame->opaque_ref ? (FrameData*)frame->opaque_ref->data : NULL;
					tb = av_buffersink_get_time_base(pOut_audio_filter);
					if (!(af = sampq.frame_queue_peek_writable()))
					{
						QString error = QString("framelist peek failed\n");
						throw PlayerException(error.toStdString(), FRAMELIST_PEEK_FAIL);
					}

					af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
					af->pos = fd ? fd->pkt_pos : -1;
					af->serial = auddec.get_serial();
					af->duration = av_q2d({ frame->nb_samples, frame->sample_rate });

					av_frame_move_ref(af->frame, frame);
					sampq.frame_queue_push();

					if (audioq.get_serial() != auddec.get_serial())
						break;
				}
				if (ret == AVERROR_EOF)
					auddec.set_finished(auddec.get_serial());
			}
		} while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	avfilter_graph_free(&pAgraph);
	av_frame_free(&frame);
	
	return;
}

void PlayerDisplay::video_thread()
{
	int ret;
	AVFrame* frame = nullptr;av_frame_alloc();
	double pts;
	double duration;
	AVFilterGraph* graph = nullptr;
	AVFilterContext* filt_out = nullptr;
	AVFilterContext* filt_in = nullptr;

	int last_w = 0;
	int last_h = 0;
	enum AVPixelFormat last_format = (AVPixelFormat)(-2);
	int last_serial = -1;
	int last_vfilter_idx = 0;

	try
	{
		frame = av_frame_alloc();
		AVRational tb = video_st->time_base;
		AVRational frame_rate = av_guess_frame_rate(pFmtCtx, video_st, NULL);

		if (!frame)
		{
			QString error = QString("frame alloc failed\n");
			throw PlayerException(error.toStdString(), FRAME_ALLOC_FAIL);
		}

		for (;;) {
			ret = get_video_frame(frame);
			if (ret < 0)
			{
				QString error = QString("get video frame failed\n");
				throw PlayerException(error.toStdString(), VIDEO_FRAME_GET_FAIL);
			}
			if (!ret)
				continue;

			if (last_w != frame->width
				|| last_h != frame->height
				|| last_format != frame->format
				|| last_serial != viddec.get_serial()
				|| last_vfilter_idx != nVfilter_idx) {
				QString error = QString("Video frame changed from size:%1x%2 format:%3 serial:%4 to size:%5x%6 format:%7 serial:%8\n")
					.arg(last_w).arg(last_h)
					.arg((const char*)av_x_if_null(av_get_pix_fmt_name(last_format), "none")).arg(last_serial)
					.arg(frame->width).arg(frame->height)
					.arg((const char*)av_x_if_null(av_get_pix_fmt_name((AVPixelFormat)frame->format), "none")).arg(viddec.get_serial());

				avfilter_graph_free(&graph);
				graph = avfilter_graph_alloc();
				if (!graph) 
				{
					QString error = QString("graph alloc failed\n");
					throw PlayerException(error.toStdString(), GRAPH_ALLOC_FAIL);
				}
				graph->nb_threads = filter_nbthreads;
				if ((ret = configure_video_filters(graph, vfilters_list ? vfilters_list[nVfilter_idx] : nullptr, frame)) < 0)
				{
					QString error = QString("video filter configure failed\n");
					throw PlayerException(error.toStdString(), VIDEO_FILTER_CONFIGURE_FAIL);
				}
				filt_in = pIn_video_filter;
				filt_out = pOut_video_filter;
				last_w = frame->width;
				last_h = frame->height;
				last_format = (AVPixelFormat)frame->format;
				last_serial = viddec.get_serial();
				last_vfilter_idx = nVfilter_idx;
				frame_rate = av_buffersink_get_frame_rate(filt_out);
			}

			ret = av_buffersrc_add_frame(filt_in, frame);
			if (ret < 0)
			{
				QString error = QString("add frame to buffersrc failed\n");
				throw PlayerException(error.toStdString(), ADD_FRAME_FAIL);
			}

			while (ret >= 0) {
				FrameData* fd;

				nFrame_last_returned_time = av_gettime_relative() / 1000000.0;

				ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
				if (ret < 0) {
					if (ret == AVERROR_EOF)
						viddec.set_finished(viddec.get_serial());
					ret = 0;
					break;
				}

				fd = frame->opaque_ref ? (FrameData*)frame->opaque_ref->data : NULL;

				nFrame_last_filter_delay = av_gettime_relative() / 1000000.0 - nFrame_last_returned_time;
				if (fabs(nFrame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0)
					nFrame_last_filter_delay = 0;
				tb = av_buffersink_get_time_base(filt_out);
				duration = (frame_rate.num && frame_rate.den ? av_q2d({ frame_rate.den, frame_rate.num }) : 0);
				pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
				ret = queue_picture(frame, pts, duration, fd ? fd->pkt_pos : -1, viddec.get_serial());
				if (ret < 0)
				{
					QString error = QString("add frame to list failed\n");
					throw PlayerException(error.toStdString(), ADD_FRAME_FAIL);
				}
				av_frame_unref(frame);
				if (videoq.get_serial() != viddec.get_serial())
					break;
			}


		}
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	avfilter_graph_free(&graph);
	av_frame_free(&frame);
	return ;
}

void PlayerDisplay::subtitle_thread()
{
	//VideoState* is = arg;
	//Frame* sp;
	//int got_subtitle;
	//double pts;

	//for (;;) {
	//	if (!(sp = frame_queue_peek_writable(&is->subpq)))
	//		return 0;

	//	if ((got_subtitle = decoder_decode_frame(&is->subdec, NULL, &sp->sub)) < 0)
	//		break;

	//	pts = 0;

	//	if (got_subtitle && sp->sub.format == 0) {
	//		if (sp->sub.pts != AV_NOPTS_VALUE)
	//			pts = sp->sub.pts / (double)AV_TIME_BASE;
	//		sp->pts = pts;
	//		sp->serial = is->subdec.pkt_serial;
	//		sp->width = is->subdec.avctx->width;
	//		sp->height = is->subdec.avctx->height;
	//		sp->uploaded = 0;

	//		/* now we can update the picture count */
	//		frame_queue_push(&is->subpq);
	//	}
	//	else if (got_subtitle) {
	//		avsubtitle_free(&sp->sub);
	//	}
	//}
	return ;
}

void PlayerDisplay::read_thread()
{
	AVFormatContext* fmtctx = nullptr;
	AVPacket* pkt = nullptr;
	const AVDictionaryEntry* t;
	int err, i, ret;
	int st_index[AVMEDIA_TYPE_NB];
	int scan_all_pmts_set = 0;
	int pkt_in_play_range = 0;
	int64_t pkt_ts;
	int64_t stream_start_time;
	std::mutex* wait_mutex = nullptr;

	try {
		wait_mutex = &mutex;
		if (!wait_mutex) {
			ret = AVERROR(ENOMEM);
			QString error = QString("%1:%2 Mutex is nullptr: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), COMMON_FAIL);
		}

		memset(st_index, -1, sizeof(st_index));
		nEof = 0;

		pkt = av_packet_alloc();
		if (!pkt)
		{
			ret = AVERROR(ENOMEM);
			QString error = QString("%1:%2 Could not allocate packet.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PACKET_ALLOC_FAIL);
		}
		fmtctx = avformat_alloc_context();
		if (!fmtctx) {
			ret = AVERROR(ENOMEM);
			QString error = QString("%1:%2 Could not allocate context.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), CONTEXT_ALLOC_FAIL);
		}
		fmtctx->interrupt_callback.callback = decode_interrupt_cb;
		fmtctx->interrupt_callback.opaque = this;
		if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
			av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
			scan_all_pmts_set = 1;
		}
		err = avformat_open_input(&fmtctx, pFilename, pIformat, &format_opts);
		if (err < 0) {
			ret = -1;
			QString error = QString("%1:%2 avformat open input failed\n").arg(__FILE__).arg(__LINE__).arg(pFilename);
			throw PlayerException(error.toStdString(), INPUT_OPEN_FAIL);
		}
		if (scan_all_pmts_set)
			av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

		if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
			ret = AVERROR_OPTION_NOT_FOUND;
			QString error = QString("%1:%2 Option %3 not found.\n").arg(__FILE__).arg(__LINE__).arg(t->key);
			throw PlayerException(error.toStdString(), DICT_GET_FAIL);
		}
		fmtctx = pFmtCtx;

		//读取文件生成缺失的 PTS
		if (genpts)
			fmtctx->flags |= AVFMT_FLAG_GENPTS;

		//注入全局侧数据
		av_format_inject_global_side_data(fmtctx);

		if (bFind_stream_info) {
			AVDictionary** opts = setup_find_stream_info_opts(fmtctx, codec_opts);
			int orig_nb_streams = fmtctx->nb_streams;

			//读取文件流信息
			err = avformat_find_stream_info(fmtctx, opts);

			for (i = 0; i < orig_nb_streams; i++)
				av_dict_free(&opts[i]);
			av_freep(&opts);

			if (err < 0) {
				ret = -1;
				QString error = QString("%1:%2 could not find stream info.\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), STREAM_INFO_GET_FIAL);
			}
		}

		if (fmtctx->pb)
			fmtctx->pb->eof_reached = 0; 

		if (seek_by_bytes < 0)
			seek_by_bytes = !(fmtctx->iformat->flags & AVFMT_NO_BYTE_SEEK) &&
			!!(fmtctx->iformat->flags & AVFMT_TS_DISCONT) &&
			std::strcmp("ogg", fmtctx->iformat->name) != 0;

		nMax_frame_duration = (fmtctx->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

		if (start_time != AV_NOPTS_VALUE) {
			int64_t timestamp;

			timestamp = start_time;
			if (fmtctx->start_time != AV_NOPTS_VALUE)
				timestamp += fmtctx->start_time;
			ret = avformat_seek_file(fmtctx, -1, INT64_MIN, timestamp, INT64_MAX, 0);
			if (ret < 0) {
				QString error = QString("%1:%2 %3:could not seek to position %4.\n").arg(__FILE__).arg(__LINE__)
					.arg(pFilename).arg((double)timestamp / AV_TIME_BASE);
				qDebug() << error;
			}
		}

		nRealtime = is_realtime(fmtctx);

		for (i = 0; i < fmtctx->nb_streams; i++) {
			AVStream* st = fmtctx->streams[i];
			enum AVMediaType type = st->codecpar->codec_type;
			st->discard = AVDISCARD_ALL;
			if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
				if (avformat_match_stream_specifier(fmtctx, st, wanted_stream_spec[type]) > 0)
					st_index[type] = i;
		}
		for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
			if (wanted_stream_spec[i] && st_index[i] == -1) {
				AVMediaType ntype = static_cast<AVMediaType>(i);
				QString error = QString("%1:%2 Stream specifier %3 does not match any %4 stream\n").arg(__FILE__).arg(__LINE__)
					.arg(wanted_stream_spec[i]).arg(av_get_media_type_string(ntype));
				qDebug() << error;
				st_index[i] = INT_MAX;
			}
		}

		if (!Video_disable)
			st_index[AVMEDIA_TYPE_VIDEO] =
			av_find_best_stream(fmtctx, AVMEDIA_TYPE_VIDEO,
				st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
		if (!Audio_disable)
			st_index[AVMEDIA_TYPE_AUDIO] =
			av_find_best_stream(fmtctx, AVMEDIA_TYPE_AUDIO,
				st_index[AVMEDIA_TYPE_AUDIO],
				st_index[AVMEDIA_TYPE_VIDEO],
				NULL, 0);
		if (!Video_disable && !Subtitle_disable)
			st_index[AVMEDIA_TYPE_SUBTITLE] =
			av_find_best_stream(fmtctx, AVMEDIA_TYPE_SUBTITLE,
				st_index[AVMEDIA_TYPE_SUBTITLE],
				(st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
					st_index[AVMEDIA_TYPE_AUDIO] :
					st_index[AVMEDIA_TYPE_VIDEO]),
				NULL, 0);

		show_mode = SHOW_MODE_NONE;
		memcpy(nSt_index, st_index, sizeof(st_index));

		if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
			stream_component_open(st_index[AVMEDIA_TYPE_AUDIO]);
		}

		ret = -1;
		if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
			ret = stream_component_open(st_index[AVMEDIA_TYPE_VIDEO]);
		}
		if (show_mode == SHOW_MODE_NONE)
			show_mode = ret >= 0 ? SHOW_MODE_VIDEO : SHOW_MODE_RDFT;

		if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
			stream_component_open(st_index[AVMEDIA_TYPE_SUBTITLE]);
		}

		if (nVideo_stream < 0 && nAudio_stream < 0) {
			ret = -1;
			QString error = QString("%1:%2 Failed to open file '%3' or configure filtergraph\n").arg(__FILE__).arg(__LINE__)
				.arg(pFilename);
			throw PlayerException(error.toStdString(), FILE_OPEN_FAIL);
		}


		if (infinite_buffer < 0 && nRealtime)
			infinite_buffer = 1;

		for (;;) {
			if (bAbort_request)
				break;
			if (nPaused != nLast_paused) {
				nLast_paused = nPaused;
				if (nPaused)
					nRead_pause_return = av_read_pause(fmtctx);
				else
					av_read_play(fmtctx);
			}
#if CONFIG_RTSP_DEMUXER || CONFIG_MMSH_PROTOCOL
			if (paused &&
				(!strcmp(ic->iformat->name, "rtsp") ||
					(ic->pb && !strncmp(input_filename, "mmsh:", 5)))) {
				SDL_Delay(10);
				continue;
			}
#endif
			if (seek_req) {
				//目标时间戳
				int64_t seek_target = seek_pos;
				//最小时间戳
				int64_t seek_min = nSeek_rel > 0 ? seek_target - nSeek_rel + 2 : INT64_MIN;
				//最大时间戳
				int64_t seek_max = nSeek_rel < 0 ? seek_target - nSeek_rel - 2 : INT64_MAX;

				ret = avformat_seek_file(fmtctx, -1, seek_min, seek_target, seek_max, seek_flags);
				if (ret < 0) {
					QString error = QString("%1:%2 %3: error while seeking\n").arg(__FILE__).arg(__LINE__)
						.arg(fmtctx->url);
					qDebug() << error;
				}
				else {
					if (nAudio_stream >= 0)
						audioq.packet_queue_flush();
					if (nSubtitle_stream >= 0)
						subtitleq.packet_queue_flush();
					if (nVideo_stream >= 0)
						videoq.packet_queue_flush();
					if (seek_flags & AVSEEK_FLAG_BYTE) {
						extclk.set_clock(NAN, 0);
					}
					else {
						extclk.set_clock(seek_target / (double)AV_TIME_BASE, 0);
					}
				}
				seek_req = 0;
				nAttachments_req = 1;
				nEof = 0;
				if (nPaused)
					step_to_next_frame();
		}
			if (nAttachments_req) {
				if (video_st && video_st->disposition & AV_DISPOSITION_ATTACHED_PIC) {
					if ((ret = av_packet_ref(pkt, &video_st->attached_pic)) < 0)
					{
						QString error = QString("%1:%2 av_packet_ref failed:%3\n").arg(__FILE__).arg(__LINE__)
							.arg(av_err2str(ret));
						throw PlayerException(error.toStdString(), FILE_OPEN_FAIL);
					}
					videoq.packet_queue_put(pkt);
					videoq.packet_queue_put_nullpacket(pkt, nVideo_stream);
				}
				nAttachments_req = 0;
			}

			if (infinite_buffer < 1 &&
				(audioq.get_size() + videoq.get_size() + subtitleq.get_size() > MAX_QUEUE_SIZE
					|| (stream_has_enough_packets(audio_st, nAudio_stream, &audioq) &&
						stream_has_enough_packets(video_st, nVideo_stream, &videoq) &&
						stream_has_enough_packets(subtitle_st, nSubtitle_stream, &subtitleq)))) {
				
				std::unique_lock<std::mutex> lock(mutex);
				cond.wait_for(lock, std::chrono::milliseconds(1000 * 10));

				continue;
			}
			if (!nPaused &&
				(!audio_st || (auddec.get_finished() == audioq.get_serial() && sampq.frame_queue_nb_remaining() == 0)) &&
				(!video_st || (viddec.get_finished() == videoq.get_serial() && pictq.frame_queue_nb_remaining() == 0))) {
				if (loop != 1 && (!loop || --loop)) {
					stream_seek(start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
				}
				else if (autoexit) {
					ret = AVERROR_EOF;
					QString error = QString("%1:%2 loop over\n").arg(__FILE__).arg(__LINE__);
					throw PlayerException(error.toStdString(), COMMON_FAIL);
				}
			}
			ret = av_read_frame(fmtctx, pkt);
			if (ret < 0) {
				if ((ret == AVERROR_EOF || avio_feof(fmtctx->pb)) && !nEof) {
					if (nVideo_stream >= 0)
						videoq.packet_queue_put_nullpacket(pkt, nVideo_stream);
					if (nAudio_stream >= 0)
						audioq.packet_queue_put_nullpacket(pkt, nAudio_stream);
					if (nSubtitle_stream >= 0)
						subtitleq.packet_queue_put_nullpacket(pkt, nSubtitle_stream);
					nEof = 1;
				}
				if (fmtctx->pb && fmtctx->pb->error) {
					if (autoexit)
					{
						QString error = QString("%1:%2 IOcontext error\n").arg(__FILE__).arg(__LINE__);
						throw PlayerException(error.toStdString(), COMMON_FAIL);
					}
					else
						break;
				}
				std::unique_lock<std::mutex> lock(mutex);
				cond.wait_for(lock, std::chrono::milliseconds(1000 * 10));
				continue;
			}
			else {
				nEof = 0;
			}
			
			stream_start_time = fmtctx->streams[pkt->stream_index]->start_time;
			pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
			pkt_in_play_range = duration == AV_NOPTS_VALUE ||
				(pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
				av_q2d(fmtctx->streams[pkt->stream_index]->time_base) -
				(double)(start_time != AV_NOPTS_VALUE ? start_time : 0) / 1000000
				<= ((double)duration / 1000000);
			if (pkt->stream_index == nAudio_stream && pkt_in_play_range) {
				audioq.packet_queue_put(pkt);
			}
			else if (pkt->stream_index == nVideo_stream && pkt_in_play_range
				&& !(video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
				videoq.packet_queue_put(pkt);
			}
			else if (pkt->stream_index == nSubtitle_stream && pkt_in_play_range) {
				subtitleq.packet_queue_put(pkt);
			}
			else {
				av_packet_unref(pkt);
			}
		}

		ret = 0;
	}
	catch (PlayerException& e) {
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		if (fmtctx && !pFmtCtx)
			avformat_close_input(&fmtctx);

		av_packet_free(&pkt);
		if (ret != 0) {
			
		}
	}
	return ;
}
