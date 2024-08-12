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
	
}

PlayerDisplay::~PlayerDisplay()
{

}

int PlayerDisplay::stream_open(const char* filename)
{
	int ret;

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
		return e.getErrorCode();
	}
	stream_close();
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
//	AVFormatContext* ic = is->ic;
//	AVCodecContext* avctx;
//	const AVCodec* codec;
//	const char* forced_codec_name = NULL;
//	AVDictionary* opts = NULL;
//	const AVDictionaryEntry* t = NULL;
//	int sample_rate;
//	AVChannelLayout ch_layout = { 0 };
	int ret = 0;
//	int stream_lowres = lowres;
//
//	if (stream_index < 0 || stream_index >= ic->nb_streams)
//		return -1;
//
//	avctx = avcodec_alloc_context3(NULL);
//	if (!avctx)
//		return AVERROR(ENOMEM);
//
//	ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
//	if (ret < 0)
//		goto fail;
//	avctx->pkt_timebase = ic->streams[stream_index]->time_base;
//
//	codec = avcodec_find_decoder(avctx->codec_id);
//
//	switch (avctx->codec_type) {
//	case AVMEDIA_TYPE_AUDIO: is->last_audio_stream = stream_index; forced_codec_name = audio_codec_name; break;
//	case AVMEDIA_TYPE_SUBTITLE: is->last_subtitle_stream = stream_index; forced_codec_name = subtitle_codec_name; break;
//	case AVMEDIA_TYPE_VIDEO: is->last_video_stream = stream_index; forced_codec_name = video_codec_name; break;
//	}
//	if (forced_codec_name)
//		codec = avcodec_find_decoder_by_name(forced_codec_name);
//	if (!codec) {
//		if (forced_codec_name) av_log(NULL, AV_LOG_WARNING,
//			"No codec could be found with name '%s'\n", forced_codec_name);
//		else                   av_log(NULL, AV_LOG_WARNING,
//			"No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
//		ret = AVERROR(EINVAL);
//		goto fail;
//	}
//
//	avctx->codec_id = codec->id;
//	if (stream_lowres > codec->max_lowres) {
//		av_log(avctx, AV_LOG_WARNING, "The maximum value for lowres supported by the decoder is %d\n",
//			codec->max_lowres);
//		stream_lowres = codec->max_lowres;
//	}
//	avctx->lowres = stream_lowres;
//
//	if (fast)
//		avctx->flags2 |= AV_CODEC_FLAG2_FAST;
//
//	opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);
//	if (!av_dict_get(opts, "threads", NULL, 0))
//		av_dict_set(&opts, "threads", "auto", 0);
//	if (stream_lowres)
//		av_dict_set_int(&opts, "lowres", stream_lowres, 0);
//
//	av_dict_set(&opts, "flags", "+copy_opaque", AV_DICT_MULTIKEY);
//
//	if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
//		goto fail;
//	}
//	if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
//		av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
//		ret = AVERROR_OPTION_NOT_FOUND;
//		goto fail;
//	}
//
//	is->eof = 0;
//	ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
//	switch (avctx->codec_type) {
//	case AVMEDIA_TYPE_AUDIO:
//	{
//		AVFilterContext* sink;
//
//		is->audio_filter_src.freq = avctx->sample_rate;
//		ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &avctx->ch_layout);
//		if (ret < 0)
//			goto fail;
//		is->audio_filter_src.fmt = avctx->sample_fmt;
//		if ((ret = configure_audio_filters(is, afilters, 0)) < 0)
//			goto fail;
//		sink = is->out_audio_filter;
//		sample_rate = av_buffersink_get_sample_rate(sink);
//		ret = av_buffersink_get_ch_layout(sink, &ch_layout);
//		if (ret < 0)
//			goto fail;
//	}
//
//	/* prepare audio output */
//	if ((ret = audio_open(is, &ch_layout, sample_rate, &is->audio_tgt)) < 0)
//		goto fail;
//	is->audio_hw_buf_size = ret;
//	is->audio_src = is->audio_tgt;
//	is->audio_buf_size = 0;
//	is->audio_buf_index = 0;
//
//	/* init averaging filter */
//	is->audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
//	is->audio_diff_avg_count = 0;
//	/* since we do not have a precise anough audio FIFO fullness,
//	   we correct audio sync only if larger than this threshold */
//	is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;
//
//	is->audio_stream = stream_index;
//	is->audio_st = ic->streams[stream_index];
//
//	if ((ret = decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread)) < 0)
//		goto fail;
//	if ((is->ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !is->ic->iformat->read_seek) {
//		is->auddec.start_pts = is->audio_st->start_time;
//		is->auddec.start_pts_tb = is->audio_st->time_base;
//	}
//	if ((ret = decoder_start(&is->auddec, audio_thread, "audio_decoder", is)) < 0)
//		goto out;
//	SDL_PauseAudioDevice(audio_dev, 0);
//	break;
//	case AVMEDIA_TYPE_VIDEO:
//		is->video_stream = stream_index;
//		is->video_st = ic->streams[stream_index];
//
//		if ((ret = decoder_init(&is->viddec, avctx, &is->videoq, is->continue_read_thread)) < 0)
//			goto fail;
//		if ((ret = decoder_start(&is->viddec, video_thread, "video_decoder", is)) < 0)
//			goto out;
//		is->queue_attachments_req = 1;
//		break;
//	case AVMEDIA_TYPE_SUBTITLE:
//		is->subtitle_stream = stream_index;
//		is->subtitle_st = ic->streams[stream_index];
//
//		if ((ret = decoder_init(&is->subdec, avctx, &is->subtitleq, is->continue_read_thread)) < 0)
//			goto fail;
//		if ((ret = decoder_start(&is->subdec, subtitle_thread, "subtitle_decoder", is)) < 0)
//			goto out;
//		break;
//	default:
//		break;
//	}
//	goto out;
//
//fail:
//	avcodec_free_context(&avctx);
//out:
//	av_channel_layout_uninit(&ch_layout);
//	av_dict_free(&opts);
//
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
		if (fmtctx && !pFmtCtx)
			avformat_close_input(&fmtctx);

		av_packet_free(&pkt);
		if (ret != 0) {
			
		}
	}
	return ;
}
