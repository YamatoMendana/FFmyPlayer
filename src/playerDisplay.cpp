#include "playerDisplay.h"

#include <signal.h>
#include <QDebug>
#include "Common.h"


PlayerDisplay::PlayerDisplay(QObject* parent):QObject(parent)
{
	int flags = 0;

	avformat_network_init();

	init();

	um_stCloseHandlerMap.insert({ AVMEDIA_TYPE_AUDIO,std::bind(&PlayerDisplay::audioStreamClose,this,std::placeholders::_1) });
	um_stCloseHandlerMap.insert({ AVMEDIA_TYPE_VIDEO,std::bind(&PlayerDisplay::videoStreamClose,this,std::placeholders::_1) });
	um_stCloseHandlerMap.insert({ AVMEDIA_TYPE_SUBTITLE,std::bind(&PlayerDisplay::subtitleStreamClose,this,std::placeholders::_1) });

}


PlayerDisplay::~PlayerDisplay()
{
	avformat_network_deinit();
	SDL_Quit();
}

int PlayerDisplay::init()
{
	if (bInit == true)
	{
		return bInit;
	}
	flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;

	if (Audio_disable)
		flags &= ~SDL_INIT_AUDIO;
	else {
		if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"))
			SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
	}
	if (Display_disable) {
		flags &= ~SDL_INIT_VIDEO;
	}
	if (SDL_Init(flags)) {
		qDebug() << QString("Could not initialize SDL - %1\n").arg(SDL_GetError());
		exit(1);
	}
	
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

	bInit = true;

	return bInit;

}

bool PlayerDisplay::startPlay(QString filename, WId widId)
{
	m_bloop = false;
	if (m_tPlayLoopThread.joinable())
	{
		m_tPlayLoopThread.join();
	}
	m_widId = widId;
	VideoState* is = nullptr;
	char strFileName[256] = { 0 };
	sprintf(strFileName, filename.toStdString().c_str(), filename.size());
	is = stream_open(strFileName);
	pCurStream = is;
	if (is == nullptr)
	{
		do_exit(pCurStream);
	}

	m_tPlayLoopThread = std::thread(&PlayerDisplay::LoopThread, this, is);
	emit sigFileOpen(filename);
	return true;
}

VideoState* PlayerDisplay::stream_open(const char* filename)
{
	int ret;
	VideoState* is = nullptr;
	
	try{
		is = (VideoState*)av_mallocz(sizeof(VideoState));
		if (!is)
		{
			QString error = QString("%1:%2 VideoState malloc failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), WITHOUT_FILE_NAME);
		}
		//memset(is, 0, sizeof(VideoState));

		is->last_video_stream = is->video_stream = -1;
		is->last_audio_stream = is->audio_stream = -1;
		is->last_subtitle_stream = is->subtitle_stream = -1;
		is->filename = av_strdup(filename);

		if (!is->filename)
		{
			QString error = QString("%1:%2 Can not find the filename,the file name is error\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), WITHOUT_FILE_NAME);
		}

		//指定输入格式
		is->ytop = 0;
		is->xleft = 0;

		if (frame_queue_init(&is->pictq, &is->videoq, VIDEO_PICTURE_QUEUE_SIZE, 1) < 0)
		{
			QString error = QString("%1:%2 picture list init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PICTURE_LIST_INIT_FAIL);
		}
		if (frame_queue_init(&is->subpq, &is->subtitleq, SUBPICTURE_QUEUE_SIZE, 0) < 0)
		{
			QString error = QString("%1:%2 subpicture list init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), SUBTITLE_LIST_INIT_FAIL);
		}
		if (frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0)
		{
			QString error = QString("%1:%2 audio list init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), AUDIO_LIST_INIT_FAIL);
		}


		if (packet_queue_init(&is->videoq) < 0 ||
			packet_queue_init(&is->audioq) < 0 ||
			packet_queue_init(&is->subtitleq) < 0)
		{
			QString error = QString("%1:%2 v/a/s PackList init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PACKET_LIST_INIT_FAIL);
		}

		if (!(is->continue_read_thread = SDL_CreateCond())) 
		{
			QString error = QString("%1:%2 SDL_CreateCond()\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), SDL_CREATE_COND_FAIL);
		}

		init_clock(&is->vidclk, &is->videoq.serial);
		init_clock(&is->audclk, &is->audioq.serial);
		init_clock(&is->extclk, &is->extclk.serial);

		is->audio_clock_serial = -1;
		if (nStartup_volume < 0)
			qDebug() << QString("volume=%1 < 0, setting to 0\n").arg(nStartup_volume);
		if (nStartup_volume > 100)
			qDebug() << QString("volume=%1 < 100, setting to 100\n").arg(nStartup_volume);
		nStartup_volume = av_clip(nStartup_volume, 0, 100);
		nStartup_volume = av_clip(SDL_MIX_MAXVOLUME * nStartup_volume / 100, 0, SDL_MIX_MAXVOLUME);
		is->audio_volume = nStartup_volume;
		bMuted = false;
		emit sigVideoVolume(nStartup_volume * 1.0 / SDL_MIX_MAXVOLUME);
		emit sigPauseState(is->paused);
		is->av_sync_type = AV_SYNC_AUDIO_MASTER;
		is->read_tid = std::thread(&PlayerDisplay::read_thread,this,is);
	}
	catch (const PlayerException& e) {
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		stream_close(is);
		return nullptr;
	}
	
	return is;

}

void PlayerDisplay::stream_close(VideoState* is)
{
	is->abort_request = 1;
	is->read_tid.join();


	if (is->audio_stream >= 0)
		stream_component_close(is, is->audio_stream);
	if (is->video_stream >= 0)
		stream_component_close(is, is->video_stream);
	if (is->subtitle_stream >= 0)
		stream_component_close(is, is->subtitle_stream);

	avformat_close_input(&is->ic);

	packet_queue_destroy(&is->videoq);
	packet_queue_destroy(&is->audioq);
	packet_queue_destroy(&is->subtitleq);

	frame_queue_destory(&is->pictq);
	frame_queue_destory(&is->sampq);
	frame_queue_destory(&is->subpq);

	SDL_DestroyCond(is->continue_read_thread);
	sws_freeContext(is->img_convert_ctx);
	sws_freeContext(is->sub_convert_ctx);
	av_free(is->filename);
	if (is->vid_texture)
		SDL_DestroyTexture(is->vid_texture);
	if (is->sub_texture)
		SDL_DestroyTexture(is->sub_texture);
	av_free(is);
}

bool PlayerDisplay::toggle_pause()
{
	if (pCurStream == nullptr)
		return true;
	stream_toggle_pause(pCurStream);
	pCurStream->step = 0;

	return pCurStream->paused;
}

void PlayerDisplay::stop()
{
	m_bloop = false;
}

bool PlayerDisplay::toggle_mute()
{
	bMuted = !bMuted;
	return bMuted;
}

int PlayerDisplay::update_volume(int sign, double step)
{
	double volume_level = pCurStream->audio_volume ? (20 * log(pCurStream->audio_volume / (double)SDL_MIX_MAXVOLUME) / log(10)) : -1000.0;
	int new_volume = lrint(SDL_MIX_MAXVOLUME * pow(10.0, (volume_level + sign * step) / 20.0));
	pCurStream->audio_volume = av_clip(pCurStream->audio_volume == new_volume ? (pCurStream->audio_volume + sign) : new_volume, 0, SDL_MIX_MAXVOLUME);
	nCurrent_volume = pCurStream->audio_volume;
	return nCurrent_volume;
}

void PlayerDisplay::stream_seek(int64_t pos, int64_t rel, int by_bytes)
{
	if (!pCurStream->seek_req) {
		pCurStream->seek_pos = pos;
		pCurStream->seek_rel = rel;
		pCurStream->seek_flags &= ~AVSEEK_FLAG_BYTE;
		pCurStream->seek_req = 1;
		SDL_CondSignal(pCurStream->continue_read_thread);
	}
}

void PlayerDisplay::seek(double seconds)
{
	if (pCurStream == nullptr)
		return;

	//转化为微秒
	double pos = seconds;
	if (isnan(pos))
		pos = (double)pCurStream->seek_pos / AV_TIME_BASE;
	if (pCurStream->ic->start_time != AV_NOPTS_VALUE && pos < pCurStream->ic->start_time / (double)AV_TIME_BASE)
		pos = pCurStream->ic->start_time / (double)AV_TIME_BASE;
	stream_seek((int64_t)(pos * AV_TIME_BASE), 0, 0);
}

void PlayerDisplay::seek_forward()
{
	if (pCurStream == nullptr)
		return;

	//转化为微秒
	double pos = get_master_clock(pCurStream);
	if (isnan(pos))
		pos = (double)pCurStream->seek_pos / AV_TIME_BASE;
	pos += incr;
	if (pCurStream->ic->start_time != AV_NOPTS_VALUE && pos < pCurStream->ic->start_time / (double)AV_TIME_BASE)
		pos = pCurStream->ic->start_time / (double)AV_TIME_BASE;
	stream_seek((int64_t)(pos * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
}

void PlayerDisplay::seek_back()
{
	if (pCurStream == nullptr)
		return;

	//转化为微秒
	double pos = get_master_clock(pCurStream);
	if (isnan(pos))
		pos = (double)pCurStream->seek_pos / AV_TIME_BASE;
	pos -= incr;
	if (pCurStream->ic->start_time != AV_NOPTS_VALUE && pos < pCurStream->ic->start_time / (double)AV_TIME_BASE)
		pos = pCurStream->ic->start_time / (double)AV_TIME_BASE;
	stream_seek((int64_t)(pos * AV_TIME_BASE), (int64_t)(-incr * AV_TIME_BASE), 0);
}

void PlayerDisplay::stream_toggle_pause(VideoState* is)
{
	if (is->paused) {
		is->frame_timer += av_gettime_relative() / 1000000.0 - is->vidclk.last_updated;
		if (is->read_pause_return != AVERROR(ENOSYS)) {
			is->vidclk.paused = 0;
		}
		set_clock(&is->vidclk, get_clock(&is->vidclk), is->vidclk.serial);
	}
	set_clock(&is->extclk, get_clock(&is->extclk), is->extclk.serial);
	is->paused = is->paused ? 0 : 1;
	is->vidclk.paused = is->paused;
	is->audclk.paused = is->paused;
	is->extclk.paused = is->paused;
}

void PlayerDisplay::step_to_next_frame(VideoState* is)
{
	if (is->paused)
		stream_toggle_pause(is);
	is->step = 1;
}

void PlayerDisplay::stream_cycle_channel(VideoState* is,int codec_type)
{
	AVFormatContext* ic = is->ic;
	int start_index, stream_index;
	int old_index;
	AVStream* st;
	AVProgram* p = NULL;
	int nb_streams = is->ic->nb_streams;

	try
	{
		if (codec_type == AVMEDIA_TYPE_VIDEO) {
			start_index = is->last_video_stream;
			old_index = is->video_stream;
		}
		else if (codec_type == AVMEDIA_TYPE_AUDIO) {
			start_index = is->last_audio_stream;
			old_index = is->audio_stream;
		}
		else {
			start_index = is->last_subtitle_stream;
			old_index = is->subtitle_stream;
		}
		stream_index = start_index;

		if (codec_type != AVMEDIA_TYPE_VIDEO && is->video_stream != -1) {
			p = av_find_program_from_stream(ic, NULL, is->video_stream);
			if (p) {
				nb_streams = p->nb_stream_indexes;
				for (start_index = 0; start_index < nb_streams; start_index++)
					if (p->stream_index[start_index] == stream_index)
						break;
				if (start_index == nb_streams)
					start_index = -1;
				stream_index = start_index;
			}
		}

		for (;;) {
			if (++stream_index >= nb_streams)
			{
				if (codec_type == AVMEDIA_TYPE_SUBTITLE)
				{
					stream_index = -1;
					is->last_subtitle_stream = -1;
					throw PlayerException("without find the stream\n", COMMON_FAIL);
				}
				if (start_index == -1)
					return;
				stream_index = 0;
			}
			if (stream_index == start_index)
				return;
			st = is->ic->streams[p ? p->stream_index[stream_index] : stream_index];
			if (st->codecpar->codec_type == codec_type) {
				switch (codec_type) {
				case AVMEDIA_TYPE_AUDIO:
					if (st->codecpar->sample_rate != 0 &&
						st->codecpar->ch_layout.nb_channels != 0)
						throw PlayerException("without find the stream\n", COMMON_FAIL);
					break;
				case AVMEDIA_TYPE_VIDEO:
				case AVMEDIA_TYPE_SUBTITLE:
					throw PlayerException("without find the stream\n", COMMON_FAIL);
				default:
					break;
				}
			}
		}
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	if (p && stream_index != -1)
		stream_index = p->stream_index[stream_index];
	QString str = QString("Switch % 1 stream from # % 2 to # % 3\n").arg(av_get_media_type_string((AVMediaType)codec_type))
		.arg(old_index).arg(stream_index);
	qDebug() << str;
	stream_component_close(is,old_index);
	stream_component_open(is,stream_index);
}

double PlayerDisplay::get_clock(Clock* c)
{
	if (*c->queue_serial != c->serial)
		return NAN;
	if (c->paused) {
		return c->pts;
	}
	else {
		double time = av_gettime_relative() / 1000000.0;
		return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
	}
}

void PlayerDisplay::set_clock_at(Clock* c, double pts, int serial, double time)
{
	c->pts = pts;
	c->last_updated = time;
	c->pts_drift = c->pts - time;
	c->serial = serial;
}

void PlayerDisplay::set_clock(Clock* c, double pts, int serial)
{
	double time = av_gettime_relative() / 1000000.0;
	set_clock_at(c, pts, serial, time);
}

void PlayerDisplay::set_clock_speed(Clock* c, double speed)
{
	set_clock(c, get_clock(c), c->serial);
	c->speed = speed;
}

void PlayerDisplay::init_clock(Clock* c, int* queue_serial)
{
	c->speed = 1.0;
	c->paused = 0;
	c->queue_serial = queue_serial;
	set_clock(c, NAN, -1);
}

void PlayerDisplay::sync_clock_to_slave(Clock* c, Clock* slave)
{
	double clock = get_clock(c);
	double slave_clock = get_clock(slave);
	if (!std::isnan(slave_clock) && (std::isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
		set_clock(c, slave_clock, slave->serial);
}

int PlayerDisplay::stream_component_open(VideoState* is,int stream_index)
{
	AVFormatContext* ic = is->ic;
	AVCodecContext* avctx = nullptr;
	const AVCodec* codec = nullptr;
	AVDictionary* opts = nullptr;
	const AVDictionaryEntry* t = nullptr;
	const char* forced_codec_name = nullptr;
	AVChannelLayout ch_layout;
	memset(&ch_layout, 0, sizeof(AVChannelLayout));

	int sample_rate;
	int ret = 0;
	int stream_lowres = 0;

	try {
		if (stream_index < 0 || stream_index >= ic->nb_streams)
			return -1;

		avctx = avcodec_alloc_context3(NULL);
		if (!avctx)
			return AVERROR(ENOMEM);

		ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
		if (ret < 0)
		{
			QString error = QString("%1:%2 avcodec_parameters_to_context failed.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PARAMETERS_TO_CONTEXT_FAIL);
		}

		avctx->pkt_timebase = ic->streams[stream_index]->time_base;

		codec = avcodec_find_decoder(avctx->codec_id);

		switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
		{
			is->last_audio_stream = stream_index;
			break;
		}
		case AVMEDIA_TYPE_SUBTITLE: {
			is->last_subtitle_stream = stream_index;
			break;
		}
		case AVMEDIA_TYPE_VIDEO:
		{
			is->last_video_stream = stream_index;
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

		opts = nullptr;
		if (!av_dict_get(opts, "threads", NULL, 0))
			av_dict_set(&opts, "threads", "auto", 0);
		if (stream_lowres)
			av_dict_set_int(&opts, "lowres", stream_lowres, 0);

		//av_dict_set(&opts, "flags", "+copy_opaque", AV_DICT_MULTIKEY);

		if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
			QString error = QString("%1:%2 avcodec open failed.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), DECODEC_OPEN_FAIL);
		}
		if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
			ret = AVERROR_OPTION_NOT_FOUND;
			QString error = QString("%1:%2 Option %s not found '%3'.\n").arg(__FILE__).arg(__LINE__).arg(t->key);
			throw PlayerException(error.toStdString(), DECODEC_OPEN_FAIL);
		}

		is->eof = 0;
		ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;
		switch (avctx->codec_type) {
		case AVMEDIA_TYPE_AUDIO:
		{
			AVFilterContext* sink;

			is->audio_filter_src.freq = avctx->sample_rate;
			ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &avctx->ch_layout);
			if (ret < 0)
			{
				QString error = QString("%1:%2 channel layout copy failed '%3'.\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), CHANNAL_LAYOUT_COPY_FAIL);
			}
			is->audio_filter_src.fmt = avctx->sample_fmt;
			if ((ret = configure_audio_filters(is, afilters, 0)) < 0)
			{
				QString error = QString("%1:%2 set audio filters configure fail\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), AUDIO_FILTER_CONFIGURE_FAIL);
			}
			sink = is->out_audio_filter;
			sample_rate = av_buffersink_get_sample_rate(sink);
			ret = av_buffersink_get_ch_layout(sink, &ch_layout);
			if (ret < 0)
			{
				QString error = QString("%1:%2 get channal layout by buffersink failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), GET_CHANNAL_LAYOUT_FAIL);
			}
		}

		// 在单独的线程中调用
		if ((ret = audio_open(&ch_layout, sample_rate, &is->audio_tgt)) < 0)
		{
			QString error = QString("%1:%2 audio open failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), AUDIO_OPEN_FAIL);
		}
		is->audio_hw_buf_size = ret;
		is->audio_src = is->audio_tgt;
		is->audio_buf_size = 0;
		is->audio_buf_index = 0;

		is->audio_diff_avg_coef = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
		is->audio_diff_avg_count = 0;

		is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;

		is->audio_stream = stream_index;
		is->audio_st = ic->streams[stream_index];

		if ((ret = decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread)) < 0)
		{
			QString error = QString("%1:%2 Decode init failed\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), DECODER_INIT_FAIL);
		}
		if ((is->ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !is->ic->iformat->read_seek) {
			is->auddec.start_pts = is->audio_st->start_time;
			is->auddec.start_pts_tb = is->audio_st->time_base;
		}
		packet_queue_start(is->auddec.queue);
		is->auddec.decode_thread = std::thread(&PlayerDisplay::audio_thread, this, is);
		SDL_PauseAudioDevice(audio_dev, 0);
		break;
		case AVMEDIA_TYPE_VIDEO:
			is->video_stream = stream_index;
			is->video_st = ic->streams[stream_index];

			if ((ret = decoder_init(&is->viddec, avctx, &is->videoq, is->continue_read_thread)) < 0)
			{
				QString error = QString("%1:%2 Decode init failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), DECODER_INIT_FAIL);
			}
			packet_queue_start(is->viddec.queue);
			is->viddec.decode_thread = std::thread(&PlayerDisplay::video_thread, this, is);
			is->queue_attachments_req = 1;
			break;
		case AVMEDIA_TYPE_SUBTITLE:
			is->subtitle_stream = stream_index;
			is->subtitle_st = ic->streams[stream_index];

			if ((ret = decoder_init(&is->subdec, avctx, &is->subtitleq, is->continue_read_thread)) < 0)
			{
				QString error = QString("%1:%2 Decode init failed\n").arg(__FILE__).arg(__LINE__);
				throw PlayerException(error.toStdString(), DECODER_INIT_FAIL);
			}
			packet_queue_start(is->subdec.queue);
			is->subdec.decode_thread = std::thread(&PlayerDisplay::subtitle_thread, this, is);
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
	}

	av_channel_layout_uninit(&ch_layout);
	av_dict_free(&opts);
	return ret;

}

void PlayerDisplay::stream_component_close(VideoState* is,int stream_index)
{
	AVFormatContext* ic = is->ic;
	AVCodecParameters* codecpar;

	if (stream_index < 0 || stream_index >= ic->nb_streams)
		return;
	codecpar = ic->streams[stream_index]->codecpar;
	ic->streams[stream_index]->discard = AVDISCARD_ALL;

	auto handler = getHandler((int)codecpar->codec_type);
	if (handler) {
		handler(is);
	}
	else {
		qDebug() << __FILE__ << ":" << __LINE__ << "codec_type:" << codecpar->codec_type << "can not find handler!";
	}

}

void PlayerDisplay::audioStreamClose(VideoState* is)
{
	decoder_abort(&is->auddec, &is->sampq);
	SDL_CloseAudioDevice(audio_dev);
	decoder_destroy(&is->auddec);
	swr_free(&is->swr_ctx);
	av_freep(&is->audio_buf1);
	is->audio_buf1_size = 0;
	is->audio_buf = NULL;

	if (is->rdft) {
		av_rdft_end(is->rdft);
		av_freep(&is->rdft_data);
		is->rdft = NULL;
		is->rdft_bits = 0;
	}
	is->audio_st = NULL;
	is->audio_stream = -1;
}

void PlayerDisplay::videoStreamClose(VideoState* is)
{
	decoder_abort(&is->viddec, &is->pictq);
	decoder_destroy(&is->viddec);
	is->video_st = NULL;
	is->video_stream = -1;
}

void PlayerDisplay::subtitleStreamClose(VideoState* is)
{
	decoder_abort(&is->subdec, &is->subpq);
	decoder_destroy(&is->subdec);
	is->subtitle_st = NULL;
	is->subtitle_stream = -1;
}

streamClosehandler PlayerDisplay::getHandler(int codec_type)
{
	auto it = um_stCloseHandlerMap.find(codec_type);
	if (it == um_stCloseHandlerMap.end())
	{
		return nullptr;
	}
	return um_stCloseHandlerMap[codec_type];
}

int PlayerDisplay::audio_decode_frame(VideoState* is)
{
	int data_size, resampled_data_size;
	av_unused double audio_clock0;
	int wanted_nb_samples;
	Frame* af;

	if (is->paused)
		return -1;

	do {
		//检查当前时间和上次音频回调时间之间的差值,确保音频数据能够及时填充到音频缓冲区
#if defined(_WIN32)
		while (frame_queue_nb_remaining(&is->sampq) == 0) {
			//如果这个差值超过了音频缓冲区大小的一半所对应的时间
			//1000000LL * nAudio_hw_buf_size / struAudio_tgt.bytes_per_sec / 2
			// 允许的最大时间差是音频缓冲区大小对应时间的一半。
			if ((av_gettime_relative() - audio_callback_time) > 1000000LL * is->audio_hw_buf_size / is->audio_tgt.bytes_per_sec / 2)
				return -1;
			av_usleep(1000);
		}
#endif
		if (!(af = frame_queue_peek_readable(&is->sampq)))
			return -1;
		frame_queue_next(&is->sampq);
	} while (af->serial != is->audioq.serial);

	data_size = av_samples_get_buffer_size(NULL, af->frame->ch_layout.nb_channels,
		af->frame->nb_samples,
		(AVSampleFormat)af->frame->format, 1);

	wanted_nb_samples = synchronize_audio(is,af->frame->nb_samples);

	if (af->frame->format != is->audio_src.fmt ||
		av_channel_layout_compare(&af->frame->ch_layout, &is->audio_src.ch_layout) ||
		af->frame->sample_rate != is->audio_src.freq ||
		(wanted_nb_samples != af->frame->nb_samples && !is->swr_ctx)) {
		swr_free(&is->swr_ctx);
		swr_alloc_set_opts2(&is->swr_ctx,
			&is->audio_tgt.ch_layout, is->audio_tgt.fmt, is->audio_tgt.freq,
			&af->frame->ch_layout, (AVSampleFormat)af->frame->format, af->frame->sample_rate,
			0, NULL);
		if (!is->swr_ctx || swr_init(is->swr_ctx) < 0) {
			QString error = QString("Cannot create sample rate converter for conversion of %1 Hz %2 %3 channels to %4 Hz %5 %6 channels!\n")
				.arg(af->frame->sample_rate).arg(av_get_sample_fmt_name((AVSampleFormat)af->frame->format)).arg(af->frame->ch_layout.nb_channels)
				.arg(is->audio_tgt.freq).arg(av_get_sample_fmt_name(is->audio_tgt.fmt)).arg(is->audio_tgt.ch_layout.nb_channels);
			qDebug() << error;
			swr_free(&is->swr_ctx);
			return -1;
		}
		if (av_channel_layout_copy(&is->audio_src.ch_layout, &af->frame->ch_layout) < 0)
			return -1;
		is->audio_src.freq = af->frame->sample_rate;
		is->audio_src.fmt = (AVSampleFormat)af->frame->format;
	}

	if (is->swr_ctx) {
		const uint8_t** in = (const uint8_t**)af->frame->extended_data;
		uint8_t** out = &is->audio_buf1;
		int out_count = (int64_t)wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate + 256;
		int out_size = av_samples_get_buffer_size(NULL, is->audio_tgt.ch_layout.nb_channels, out_count, is->audio_tgt.fmt, 0);
		int len2;
		if (out_size < 0) {
			QString error = QString("av_samples_get_buffer_size() failed\n");
			qDebug() << error;
			return -1;
		}
		if (wanted_nb_samples != af->frame->nb_samples) {
			//设置音频重采样器的补偿参数,处理音频播放中的同步问题
			if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,
				wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0) {
				QString error = QString("swr_set_compensation() failed\n");
				qDebug() << error;
				return -1;
			}
		}
		//快速分配内存 需要频繁分配和释放内存的场景避免内存碎片问题
		av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
		if (!is->audio_buf1)
			return AVERROR(ENOMEM);
		//音频重采样操作 将输入音频数据从一种格式转换为另一种格式
		len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
		if (len2 < 0) {
			QString error = QString("swr_convert() failed\n");
			qDebug() << error;
			return -1;
		}
		if (len2 == out_count) {
			QString error = QString("audio buffer is probably too small\n");
			qDebug() << error;
			if (swr_init(is->swr_ctx) < 0)
				swr_free(&is->swr_ctx);
		}
		is->audio_buf = is->audio_buf1;
		resampled_data_size = len2 * is->audio_tgt.ch_layout.nb_channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
	}
	else {
		is->audio_buf = af->frame->data[0];
		resampled_data_size = data_size;
	}

	audio_clock0 = is->audio_clock;
	//更新音频时钟
	if (!isnan(af->pts))
		is->audio_clock = af->pts + (double)af->frame->nb_samples / af->frame->sample_rate;
	else
		is->audio_clock = NAN;
	is->audio_clock_serial = af->serial;
	return resampled_data_size;
}

void PlayerDisplay::fill_rectangle(int x, int y, int w, int h)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	if (w && h)
		SDL_RenderFillRect(renderer, &rect);
}

void PlayerDisplay::get_sdl_pix_fmt_and_blendmode(int format, Uint32* sdl_pix_fmt, SDL_BlendMode* sdl_blendmode)
{
	int i;
	*sdl_blendmode = SDL_BLENDMODE_NONE;
	*sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
	if (format == AV_PIX_FMT_RGB32 ||
		format == AV_PIX_FMT_RGB32_1 ||
		format == AV_PIX_FMT_BGR32 ||
		format == AV_PIX_FMT_BGR32_1)
		*sdl_blendmode = SDL_BLENDMODE_BLEND;
	for (i = 0; i < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; i++) {
		if (format == sdl_texture_format_map[i].format) {
			*sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
			return;
		}
	}
}

int PlayerDisplay::realloc_texture(SDL_Texture** texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture)
{
	Uint32 format;
	int access, w, h;
	if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format) {
		void* pixels;
		int pitch;
		if (*texture)
			SDL_DestroyTexture(*texture);
		if (!(*texture = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)))
			return -1;
		if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
			return -1;
		if (init_texture) {
			if (SDL_LockTexture(*texture, NULL, &pixels, &pitch) < 0)
				return -1;
			memset(pixels, 0, pitch * new_height);
			SDL_UnlockTexture(*texture);
		}
		av_log(NULL, AV_LOG_VERBOSE, "Created %dx%d texture with %s.\n", new_width, new_height, SDL_GetPixelFormatName(new_format));
	}
	return 0;
}

int PlayerDisplay::upload_texture(SDL_Texture** tex, AVFrame* frame)
{
	int ret = 0;
	Uint32 sdl_pix_fmt;
	SDL_BlendMode sdl_blendmode;
	get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
	if (realloc_texture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
		return -1;
	switch (sdl_pix_fmt) {
	case SDL_PIXELFORMAT_IYUV:
		if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
			ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0], frame->linesize[0],
				frame->data[1], frame->linesize[1],
				frame->data[2], frame->linesize[2]);
		}
		else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
			ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
				frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
				frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
		}
		else {
			av_log(NULL, AV_LOG_ERROR, "Mixed negative and positive linesizes are not supported.\n");
			return -1;
		}
		break;
	default:
		if (frame->linesize[0] < 0) {
			ret = SDL_UpdateTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
		}
		else {
			ret = SDL_UpdateTexture(*tex, NULL, frame->data[0], frame->linesize[0]);
		}
		break;
	}
	return ret;
}

void PlayerDisplay::set_sdl_yuv_conversion_mode(AVFrame* frame)
{
#if SDL_VERSION_ATLEAST(2,0,8)
	SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
	if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
		if (frame->color_range == AVCOL_RANGE_JPEG)
			mode = SDL_YUV_CONVERSION_JPEG;
		else if (frame->colorspace == AVCOL_SPC_BT709)
			mode = SDL_YUV_CONVERSION_BT709;
		else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M)
			mode = SDL_YUV_CONVERSION_BT601;
	}
	SDL_SetYUVConversionMode(mode); /* FIXME: no support for linear transfer */
#endif
}

void PlayerDisplay::video_image_display(VideoState* is)
{
	Frame* vp;
	Frame* sp = NULL;
	SDL_Rect rect;

	vp = frame_queue_peek_last(&is->pictq);
	if (is->subtitle_st) {
		if (frame_queue_nb_remaining(&is->subpq) > 0) {
			sp = frame_queue_peek(&is->subpq);

			if (vp->pts >= sp->pts + ((float)sp->sub.start_display_time / 1000)) {
				if (!sp->uploaded) {
					uint8_t* pixels[4];
					int pitch[4];
					int i;
					if (!sp->width || !sp->height) {
						sp->width = vp->width;
						sp->height = vp->height;
					}
					if (realloc_texture(&is->sub_texture, SDL_PIXELFORMAT_ARGB8888, sp->width, sp->height, SDL_BLENDMODE_BLEND, 1) < 0)
						return;

					for (i = 0; i < sp->sub.num_rects; i++) {
						AVSubtitleRect* sub_rect = sp->sub.rects[i];

						sub_rect->x = av_clip(sub_rect->x, 0, sp->width);
						sub_rect->y = av_clip(sub_rect->y, 0, sp->height);
						sub_rect->w = av_clip(sub_rect->w, 0, sp->width - sub_rect->x);
						sub_rect->h = av_clip(sub_rect->h, 0, sp->height - sub_rect->y);

						is->sub_convert_ctx = sws_getCachedContext(is->sub_convert_ctx,
							sub_rect->w, sub_rect->h, AV_PIX_FMT_PAL8,
							sub_rect->w, sub_rect->h, AV_PIX_FMT_BGRA,
							0, NULL, NULL, NULL);
						if (!is->sub_convert_ctx) {
							av_log(NULL, AV_LOG_FATAL, "Cannot initialize the conversion context\n");
							return;
						}
						if (!SDL_LockTexture(is->sub_texture, (SDL_Rect*)sub_rect, (void**)pixels, pitch)) {
							sws_scale(is->sub_convert_ctx, (const uint8_t* const*)sub_rect->data, sub_rect->linesize,
								0, sub_rect->h, pixels, pitch);
							SDL_UnlockTexture(is->sub_texture);
						}
					}
					sp->uploaded = 1;
				}
			}
			else
				sp = NULL;
		}
	}

	calculate_display_rect(&rect, is->xleft, is->ytop, is->width, is->height, vp->width, vp->height, vp->sar);
	set_sdl_yuv_conversion_mode(vp->frame);

	if (!vp->uploaded) {
		if (upload_texture(&is->vid_texture, vp->frame) < 0) {
			set_sdl_yuv_conversion_mode(NULL);
			return;
		}
		vp->uploaded = 1;
		vp->flip_v = vp->frame->linesize[0] < 0;
	}

	//宽高变化通知
	//something...

	SDL_RenderCopyEx(renderer, is->vid_texture, NULL, &rect, 0, NULL, static_cast<SDL_RendererFlip>(vp->flip_v ? SDL_FLIP_VERTICAL : 0));
	set_sdl_yuv_conversion_mode(NULL);
	if (sp) {
		SDL_RenderCopy(renderer, is->sub_texture, NULL, &rect);
	}
}

void PlayerDisplay::video_audio_display(VideoState* is)
{
	int i, i_start, x, y1, y, ys, delay, n, nb_display_channels;
	int ch, channels, h, h2;
	int64_t time_diff;
	int rdft_bits, nb_freq;

	for (rdft_bits = 1; (1 << rdft_bits) < 2 * is->height; rdft_bits++)
		;
	nb_freq = 1 << (rdft_bits - 1);

	channels = is->audio_tgt.ch_layout.nb_channels;
	nb_display_channels = channels;
	if (!is->paused) {
		int data_used = is->show_mode == is->SHOW_MODE_WAVES ? is->width : (2 * nb_freq);
		n = 2 * channels;
		delay = is->audio_write_buf_size;
		delay /= n;

		if (audio_callback_time) {
			time_diff = av_gettime_relative() - audio_callback_time;
			delay -= (time_diff * is->audio_tgt.freq) / 1000000;
		}

		delay += 2 * data_used;
		if (delay < data_used)
			delay = data_used;

		i_start = x = compute_mod(is->sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
		if (is->show_mode == is->SHOW_MODE_WAVES) {
			h = INT_MIN;
			for (i = 0; i < 1000; i += channels) {
				int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
				int a = is->sample_array[idx];
				int b = is->sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
				int c = is->sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
				int d = is->sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
				int score = a - d;
				if (h < score && (b ^ c) < 0) {
					h = score;
					i_start = idx;
				}
			}
		}

		is->last_i_start = i_start;
	}
	else {
		i_start = is->last_i_start;
	}

	if (is->show_mode == is->SHOW_MODE_WAVES) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		h = is->height / nb_display_channels;
		h2 = (h * 9) / 20;
		for (ch = 0; ch < nb_display_channels; ch++) {
			i = i_start + ch;
			y1 = is->ytop + ch * h + (h / 2); /* position of center line */
			for (x = 0; x < is->width; x++) {
				y = (is->sample_array[i] * h2) >> 15;
				if (y < 0) {
					y = -y;
					ys = y1 - y;
				}
				else {
					ys = y1;
				}
				fill_rectangle(is->xleft + x, ys, 1, y);
				i += channels;
				if (i >= SAMPLE_ARRAY_SIZE)
					i -= SAMPLE_ARRAY_SIZE;
			}
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

		for (ch = 1; ch < nb_display_channels; ch++) {
			y = is->ytop + ch * h;
			fill_rectangle(is->xleft, y, is->width, 1);
		}
	}
	else {
		if (realloc_texture(&is->vis_texture, SDL_PIXELFORMAT_ARGB8888, is->width, is->height, SDL_BLENDMODE_NONE, 1) < 0)
			return;

		if (is->xpos >= is->width)
			is->xpos = 0;
		nb_display_channels = FFMIN(nb_display_channels, 2);
		if (rdft_bits != is->rdft_bits) {
			av_rdft_end(is->rdft);
			av_free(is->rdft_data);
			is->rdft = av_rdft_init(rdft_bits, DFT_R2C);
			is->rdft_bits = rdft_bits;
			is->rdft_data = (FFTSample*)av_malloc_array(nb_freq, 4 * sizeof(*is->rdft_data));
		}
		if (!is->rdft || !is->rdft_data) {
			av_log(NULL, AV_LOG_ERROR, "Failed to allocate buffers for RDFT, switching to waves display\n");
			is->show_mode = is->SHOW_MODE_WAVES;
		}
		else {
			FFTSample* data[2];
			SDL_Rect rect = { is->xpos, 0,1,is->height };
			uint32_t* pixels;
			int pitch;
			for (ch = 0; ch < nb_display_channels; ch++) {
				data[ch] = is->rdft_data + 2 * nb_freq * ch;
				i = i_start + ch;
				for (x = 0; x < 2 * nb_freq; x++) {
					double w = (x - nb_freq) * (1.0 / nb_freq);
					data[ch][x] = is->sample_array[i] * (1.0 - w * w);
					i += channels;
					if (i >= SAMPLE_ARRAY_SIZE)
						i -= SAMPLE_ARRAY_SIZE;
				}
				av_rdft_calc(is->rdft, data[ch]);
			}

			if (!SDL_LockTexture(is->vis_texture, &rect, (void**)&pixels, &pitch)) {
				pitch >>= 2;
				pixels += pitch * is->height;
				for (y = 0; y < is->height; y++) {
					double w = 1 / sqrt(nb_freq);
					int a = sqrt(w * sqrt(data[0][2 * y + 0] * data[0][2 * y + 0] + data[0][2 * y + 1] * data[0][2 * y + 1]));
					int b = (nb_display_channels == 2) ? sqrt(w * hypot(data[1][2 * y + 0], data[1][2 * y + 1]))
						: a;
					a = FFMIN(a, 255);
					b = FFMIN(b, 255);
					pixels -= pitch;
					*pixels = (a << 16) + (b << 8) + ((a + b) >> 1);
				}
				SDL_UnlockTexture(is->vis_texture);
			}
			SDL_RenderCopy(renderer, is->vis_texture, NULL, NULL);
		}
		if (!is->paused)
			is->xpos++;
	}
}

void PlayerDisplay::video_display(VideoState* is)
{
	if (!is->width)
		video_open(is);
	if(renderer)
	{
		lock_guard<std::mutex> lock(mutex);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		if (is->audio_st && is->show_mode != is->SHOW_MODE_VIDEO)
			video_audio_display(is);
		else if (is->video_st)
			video_image_display(is);
		SDL_RenderPresent(renderer);
	}
}

int PlayerDisplay::video_open(VideoState* is)
{
	int w, h;

	w = screen_width ? screen_width : default_width;
	h = screen_height ? screen_height : default_height;

	if(!window)
	{
		int flags = SDL_WINDOW_SHOWN;
		flags |= SDL_WINDOW_RESIZABLE;
		window = SDL_CreateWindowFrom((void*)m_widId);
		SDL_GetWindowSize(window, &w, &h);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		if (window) {
			SDL_RendererInfo info;
			if (!renderer)
				renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (!renderer) {
				{
					QString error = QString("Failed to initialize a hardware accelerated renderer: %1\n").arg(SDL_GetError());
					qDebug() << error;
				}
				renderer = SDL_CreateRenderer(window, -1, 0);
			}
			if (renderer) {
				if (!SDL_GetRendererInfo(renderer, &info))
				{
					QString error = QString("Initialized %1 renderer.\n").arg(info.name);
					qDebug() << error;
				}

			}
		}
	}
	else {
		SDL_SetWindowSize(window, w, h);
	}

	if (!window || !renderer) {
		QString error = QString("SDL: could not set video mode - exiting\n");
		qDebug()<< error;
		do_exit(is);
	}

	is->width = w;
	is->height = h;

	return 0;
}

void PlayerDisplay::video_refresh(void* opaque, double* remaining_time)
{
	VideoState* is = (VideoState*)opaque;
	double time;

	Frame* sp, * sp2;
	rdftspeed = 0.02;

	try
	{
		if (!is->paused && get_master_sync_type(is) == AV_SYNC_EXTERNAL_CLOCK && is->realtime)
			check_external_clock_speed(is);

		if (!Display_disable && is->show_mode != is->SHOW_MODE_VIDEO && is->audio_st) {
			time = av_gettime_relative() / 1000000.0;
			if (is->force_refresh || is->last_vis_time + rdftspeed < time) {
				video_display(is);
				is->last_vis_time = time;
			}
			*remaining_time = FFMIN(*remaining_time, is->last_vis_time + rdftspeed - time);
		}

		if (is->video_st) 
		{
			while (1)
			{
				if (frame_queue_nb_remaining(&is->pictq) == 0) {
					// nothing to do, no picture to display in the queue
				}
				else {
					double last_duration, duration, delay;
					Frame* vp, * lastvp;

					lastvp = frame_queue_peek_last(&is->pictq);
					vp = frame_queue_peek(&is->pictq);

					if (vp->serial != is->videoq.serial) {
						frame_queue_next(&is->pictq);
						continue;
					}

					if (lastvp->serial != vp->serial)
						is->frame_timer = av_gettime_relative() / 1000000.0;

					if (is->paused)
						break;

					last_duration = vp_duration(is, lastvp, vp);
					delay = compute_target_delay(is, last_duration);

					time = av_gettime_relative() / 1000000.0;
					if (time < is->frame_timer + delay) {
						*remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
						break;
					}

					is->frame_timer += delay;
					if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
						is->frame_timer = time;

					SDL_LockMutex(is->pictq.mutex);
					if (!std::isnan(vp->pts))
						update_video_pts(is, vp->pts, vp->serial);
					SDL_UnlockMutex(is->pictq.mutex);

					if (frame_queue_nb_remaining(&is->pictq) > 1) {
						Frame* nextvp = frame_queue_peek_next(&is->pictq);
						duration = vp_duration(is, vp, nextvp);
						if (!is->step && (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) && time > is->frame_timer + duration) {
							is->frame_drops_late++;
							frame_queue_next(&is->pictq);
							continue;
						}
					}

					if (is->subtitle_st) 
					{
						while (frame_queue_nb_remaining(&is->subpq) > 0) {
							sp = frame_queue_peek(&is->subpq);

							if (frame_queue_nb_remaining(&is->subpq) > 1)
								sp2 = frame_queue_peek_next(&is->subpq);
							else
								sp2 = NULL;

							if (sp->serial != is->subtitleq.serial
								|| (is->vidclk.pts > (sp->pts + ((float)sp->sub.end_display_time / 1000)))
								|| (sp2 && is->vidclk.pts > (sp2->pts + ((float)sp2->sub.start_display_time / 1000))))
							{
								if (sp->uploaded) {
									int i;
									for (i = 0; i < sp->sub.num_rects; i++) {
										AVSubtitleRect* sub_rect = sp->sub.rects[i];
										uint8_t* pixels;
										int pitch, j;

										if (!SDL_LockTexture(is->sub_texture, (SDL_Rect*)sub_rect, (void**)&pixels, &pitch)) {
											for (j = 0; j < sub_rect->h; j++, pixels += pitch)
												memset(pixels, 0, (sub_rect->w) << 2);
											SDL_UnlockTexture(is->sub_texture);
										}
									}
								}
								frame_queue_next(&is->subpq);
							}
							else {
								break;
							}
						}
					}

					frame_queue_next(&is->pictq);
					is->force_refresh = 1;

					if (is->step && !is->paused)
						stream_toggle_pause(is);

					break;
				}
			}
			if (is->force_refresh && is->pictq.rindex_shown)
				video_display(is);
		}
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	
	
	is->force_refresh = 0;
	emit sigVideoPlaySeconds(get_master_clock(is));
}

void PlayerDisplay::refresh_loop_wait_event(VideoState* is, SDL_Event* event)
{
	double remaining_time = 0.0;
	SDL_PumpEvents();
	while (!SDL_PeepEvents(event, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) && m_bloop)
	{
		if (remaining_time > 0.0)
			av_usleep((int64_t)(remaining_time * 1000000.0));
		remaining_time = REFRESH_RATE;
		if (!is->paused || is->force_refresh)
			video_refresh(is, &remaining_time);
		SDL_PumpEvents();
	}
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

int PlayerDisplay::insert_vfilter(char* arg)
{
	int new_size = nNb_vfilters + 1;
	int elem_size = sizeof(*vfilters_list);
	if (new_size >= INT_MAX / elem_size) {
		qDebug() << "Array too big.\n";
		exit(1);
	}
	if (nNb_vfilters < new_size) {
		uint8_t* tmp = (uint8_t*)av_realloc_array(*vfilters_list, new_size, elem_size);
		if (!tmp)
			qDebug() << "array alloc fail";
		memset(tmp + nNb_vfilters * elem_size, 0, (new_size - nNb_vfilters) * elem_size);
		nNb_vfilters = new_size;
		*vfilters_list = reinterpret_cast <char*>(tmp);
	}
	vfilters_list[nNb_vfilters - 1] = arg;
	return 0;
}

void PlayerDisplay::insert_filter(AVFilterContext*& last_filter, AVFilterGraph* graph, const char* name, const char* arg)
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

int PlayerDisplay::configure_audio_filters(VideoState* is,const char* afilters, int force_output_format)
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
		avfilter_graph_free(&is->agraph);
		if (!(is->agraph = avfilter_graph_alloc()))
			return AVERROR(ENOMEM);
		is->agraph->nb_threads = filter_nbthreads;

		av_bprint_init(&bp, 0, AV_BPRINT_SIZE_AUTOMATIC);

		//遍历字典（dictionary）中的所有条目
		while ((e = av_dict_iterate(swr_opts, e)))
			av_strlcatf(aresample_swr_opts, sizeof(aresample_swr_opts), "%s=%s:", e->key, e->value);
		if (strlen(aresample_swr_opts))
			aresample_swr_opts[strlen(aresample_swr_opts) - 1] = '\0';
		av_opt_set(is->agraph, "aresample_swr_opts", aresample_swr_opts, 0);

		av_channel_layout_describe_bprint(&is->audio_filter_src.ch_layout, &bp);

		ret = snprintf(asrc_args, sizeof(asrc_args),
			"sample_rate=%d:sample_fmt=%s:time_base=%d/%d:channel_layout=%s",
			is->audio_filter_src.freq, av_get_sample_fmt_name(is->audio_filter_src.fmt),
			1, is->audio_filter_src.freq, bp.str);

		//abuffer创建音频缓冲区滤镜
		ret = avfilter_graph_create_filter(&filt_asrc,
			avfilter_get_by_name("abuffer"), "ffplay_abuffer",
			asrc_args, NULL, is->agraph);
		if (ret < 0)
		{
			QString error = QString("%1:%2 create graph filter failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), CREATE_GRAPH_FILTER_FAIL);
		}

		//abuffersink创建一个音频缓冲区接收器
		ret = avfilter_graph_create_filter(&filt_asink,
			avfilter_get_by_name("abuffersink"), "ffplay_abuffersink",
			NULL, NULL, is->agraph);
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
			sample_rates[0] = is->audio_tgt.freq;
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


		if ((ret = configure_filtergraph(is->agraph, afilters, filt_asrc, filt_asink)) < 0)
		{
			QString error = QString("%1:%2 configure filtergraph failed: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), FILTERGRAPH_CONFIGURE_FAIL);
		}

		is->in_audio_filter = filt_asrc;
		is->out_audio_filter = filt_asink;
	}
	catch (PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	if (ret < 0)
		avfilter_graph_free(&is->agraph);
	av_bprint_finalize(&bp, NULL);
	return ret;
}

int PlayerDisplay::configure_video_filters(AVFilterGraph* graph, VideoState* is,const char* vfilters, AVFrame* frame)
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
		AVCodecParameters* codecpar = is->video_st->codecpar;
		AVRational fr = av_guess_frame_rate(is->ic, is->video_st, NULL);

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
			is->video_st->time_base.num, is->video_st->time_base.den,
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
				displaymatrix = (int32_t*)av_stream_get_side_data(is->video_st, AV_PKT_DATA_DISPLAYMATRIX, NULL);
			theta = get_rotation(displaymatrix);

			if (fabs(theta - 90) < 1.0) {//顺时针旋转
				insert_filter(last_filter, graph, "transpose", "clock");
			}
			else if (fabs(theta - 180) < 1.0) {//水平垂直
				insert_filter(last_filter, graph, "hflip", NULL);
				insert_filter(last_filter, graph, "vflip", NULL);
			}
			else if (fabs(theta - 270) < 1.0) {//逆时针旋转
				insert_filter(last_filter, graph, "transpose", "cclock");
			}
			else if (fabs(theta) > 1.0) {//旋转特定角度
				char rotate_buf[64];
				snprintf(rotate_buf, sizeof(rotate_buf), "%f*PI/180", theta);
				insert_filter(last_filter, graph, "rotate", rotate_buf);
			}
		}

		if ((ret = configure_filtergraph(graph, vfilters, filt_src, last_filter)) < 0)
		{
			QString error = QString("filtergraph configure failed\n");
			throw PlayerException(error.toStdString(), FILTERGRAPH_CONFIGURE_FAIL);
		}

		is->in_video_filter = filt_src;
		is->out_video_filter = filt_out;
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}

	return ret;
}

void PlayerDisplay::do_exit(VideoState*& is)
{
	if (is)
	{
		stream_close(is);
		is = nullptr;
		pCurStream = nullptr;
	}
	if (renderer)
	{
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}

	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	emit sigStopFinish();
}

double PlayerDisplay::display_rotation_get(const int32_t matrix[9])
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

void PlayerDisplay::display_rotation_set(int32_t matrix[9], double angle)
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
		theta = -round(display_rotation_get((int32_t*)displaymatrix));

	theta -= 360 * floor(theta / 360 + 0.9 / 360);

	if (fabs(theta - 90 * round(theta / 90)) > 2)
		qDebug() << "Odd rotation angle.\n";

	return theta;
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

double PlayerDisplay::vp_duration(VideoState* is,Frame* vp, Frame* nextvp)
{
	if (vp->serial == nextvp->serial) {
		double duration = nextvp->pts - vp->pts;
		if (isnan(duration) || duration <= 0 || duration > is->max_frame_duration)
			return vp->duration;
		else
			return duration;
	}
	else {
		return 0.0;
	}
}

double PlayerDisplay::compute_target_delay(VideoState* is,double delay)
{
	double sync_threshold, diff = 0;

	if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) {
		diff = get_clock(&is->vidclk) - get_master_clock(is);

		sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
		if (!isnan(diff) && fabs(diff) < is->max_frame_duration) {
			if (diff <= -sync_threshold)
				delay = FFMAX(0, delay + diff);
			else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
				delay = delay + diff;
			else if (diff >= sync_threshold)
				delay = 2 * delay;
		}
	}

	return delay;
}

int PlayerDisplay::get_master_sync_type(VideoState* is)
{
	if (is->av_sync_type == AV_SYNC_VIDEO_MASTER) {
		if (is->video_st)
			return AV_SYNC_VIDEO_MASTER;
		else
			return AV_SYNC_AUDIO_MASTER;
	}
	else if (is->av_sync_type == AV_SYNC_AUDIO_MASTER) {
		if (is->audio_st)
			return AV_SYNC_AUDIO_MASTER;
		else
			return AV_SYNC_EXTERNAL_CLOCK;
	}
	else {
		return AV_SYNC_EXTERNAL_CLOCK;
	}
}

double PlayerDisplay::get_master_clock(VideoState* is)
{
	double val;

	switch (get_master_sync_type(is)) {
	case AV_SYNC_VIDEO_MASTER:
		val = get_clock(&is->vidclk);
		break;
	case AV_SYNC_AUDIO_MASTER:
		val = get_clock(&is->audclk);
		break;
	default:
		val = get_clock(&is->extclk);
		break;
	}
	return val;
}

void PlayerDisplay::check_external_clock_speed(VideoState* is)
{
	if (is->video_stream >= 0 && is->videoq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES ||
		is->audio_stream >= 0 && is->audioq.nb_packets <= EXTERNAL_CLOCK_MIN_FRAMES) {
		set_clock_speed(&is->extclk, FFMAX(EXTERNAL_CLOCK_SPEED_MIN, is->extclk.speed - EXTERNAL_CLOCK_SPEED_STEP));
	}
	else if ((is->video_stream < 0 || is->videoq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES) &&
		(is->audio_stream < 0 || is->audioq.nb_packets > EXTERNAL_CLOCK_MAX_FRAMES)) {
		set_clock_speed(&is->extclk, FFMIN(EXTERNAL_CLOCK_SPEED_MAX, is->extclk.speed + EXTERNAL_CLOCK_SPEED_STEP));
	}
	else {
		double speed = is->extclk.speed;
		if (speed != 1.0)
			set_clock_speed(&is->extclk, speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
	}
}

void PlayerDisplay::update_video_pts(VideoState* is,double pts, int serial)
{
	set_clock(&is->vidclk, pts, serial);
	sync_clock_to_slave(&is->extclk, &is->vidclk);
}

void PlayerDisplay::set_default_window_size(int width, int height, AVRational sar)
{
	SDL_Rect rect;
	int max_width = screen_width ? screen_width : INT_MAX;
	int max_height = screen_height ? screen_height : INT_MAX;
	if (max_width == INT_MAX && max_height == INT_MAX)
		max_height = height;
	calculate_display_rect(&rect, 0, 0, max_width, max_height, width, height, sar);
	default_width = rect.w;
	default_height = rect.h;
}

void PlayerDisplay::calculate_display_rect(SDL_Rect* rect, int scr_xleft, int scr_ytop, int scr_width, int scr_height, int pic_width, int pic_height, AVRational pic_sar)
{
	AVRational aspect_ratio = pic_sar;
	int64_t width, height, x, y;

	if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0)
		aspect_ratio = av_make_q(1, 1);

	aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height));

	height = scr_height;
	width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
	if (width > scr_width) {
		width = scr_width;
		height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
	}
	x = (scr_width - width) / 2;
	y = (scr_height - height) / 2;
	rect->x = scr_xleft + x;
	rect->y = scr_ytop + y;
	rect->w = FFMAX((int)width, 1);
	rect->h = FFMAX((int)height, 1);
}

int PlayerDisplay::stream_has_enough_packets(AVStream* st, int stream_id, PacketQueue* queue)
{
	return stream_id < 0 ||
		queue->abort_request ||
		(st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
		queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

int PlayerDisplay::queue_picture(VideoState* is, AVFrame* src_frame, double pts, double duration, int64_t pos, int serial)
{
	Frame* vp;

	if (!(vp = frame_queue_peek_writable(&is->pictq)))
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
	frame_queue_push(&is->pictq);
	return 0;
}

int PlayerDisplay::get_video_frame(VideoState* is, AVFrame* frame)
{
	int got_picture;

	if ((got_picture = decoder_decode_frame(&is->viddec, frame, NULL)) < 0)
		return -1;

	if (got_picture) {
		//解码后的显示时间戳
		double dpts = NAN;

		if (frame->pts != AV_NOPTS_VALUE)
			//dpts=PTS×time_base
			dpts = av_q2d(is->video_st->time_base) * frame->pts;

		//猜测视频帧的样本宽高比
		frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);

		if (framedrop > 0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
			if (frame->pts != AV_NOPTS_VALUE) {
				double diff = dpts - get_master_clock(is);
				if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
					diff - is->frame_last_filter_delay < 0 &&
					is->viddec.pkt_serial == is->vidclk.serial &&
					is->videoq.nb_packets
					) 
				{
					is->frame_drops_early++;
					av_frame_unref(frame);
					got_picture = 0;
				}
			}
		}
	}

	return got_picture;
}

int PlayerDisplay::synchronize_audio(VideoState* is,int nb_samples)
{
	int wanted_nb_samples = nb_samples;

	//如果获取主时钟类型不是音频为主时钟
	if (get_master_sync_type(is) != AV_SYNC_AUDIO_MASTER) {
		double diff, avg_diff;
		int min_nb_samples, max_nb_samples;

		diff = get_clock(&is->audclk) - get_master_clock(is);

		if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD) {
			//引入 nAudio_diff_avg_coef，可以对音频时间差进行平滑处理，减少瞬时波动对累积值的影响
			//通过不断累积音频时间差，可以更好地跟踪音频和视频之间的长期同步情况
			//使用指数加权平均法可以给最近的音频时间差赋予更高的权重
			is->audio_diff_cum = diff + is->audio_diff_avg_coef * is->audio_diff_cum;
			if (is->audio_diff_avg_count < AUDIO_DIFF_AVG_NB) {
				is->audio_diff_avg_count++;
			}
			else {
				//计算平均音频时间差avg_diff,通过累积音频时间差 nAudio_diff_cum 乘以 (1.0 - nAudio_diff_avg_coef)
				avg_diff = is->audio_diff_cum * (1.0 - is->audio_diff_avg_coef);

				if (fabs(avg_diff) >= is->audio_diff_threshold) {
					//根据音频时间差 diff 和音频源的采样率 struAudio_src.freq，计算所需的音频样本数量 wanted_nb_samples。
					wanted_nb_samples = nb_samples + (int)(diff * is->audio_src.freq);
					//计算最小和最大允许的音频样本数量 min_nb_samples 和 max_nb_samples
					min_nb_samples = ((nb_samples * (100 - SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					max_nb_samples = ((nb_samples * (100 + SAMPLE_CORRECTION_PERCENT_MAX) / 100));
					//使用 av_clip 函数将 wanted_nb_samples 限制在这个范围内
					wanted_nb_samples = av_clip(wanted_nb_samples, min_nb_samples, max_nb_samples);
				}
				QString error = QString("diff=%1 adiff=%2 sample_diff=%3 apts=%4 %5\n")
					.arg(diff).arg(avg_diff).arg(wanted_nb_samples - nb_samples)
					.arg(is->audio_clock).arg(is->audio_diff_threshold);
				qDebug() << error;
			}
		}
		else {
			is->audio_diff_avg_count = 0;
			is->audio_diff_cum = 0;
		}
	}

	return wanted_nb_samples;
}

int PlayerDisplay::cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1, enum AVSampleFormat fmt2, int64_t channel_count2)
{
	if (channel_count1 == 1 && channel_count2 == 1)
		return av_get_packed_sample_fmt(fmt1) != av_get_packed_sample_fmt(fmt2);
	else
		return channel_count1 != channel_count2 || fmt1 != fmt2;
}

void PlayerDisplay::update_sample_display(VideoState* is,short* samples, int samples_size)
{
	int size, len;

	size = samples_size / sizeof(short);
	while (size > 0) {
		len = SAMPLE_ARRAY_SIZE - is->sample_array_index;
		if (len > size)
			len = size;
		memcpy(is->sample_array + is->sample_array_index, samples, len * sizeof(short));
		samples += len;
		is->sample_array_index += len;
		if (is->sample_array_index >= SAMPLE_ARRAY_SIZE)
			is->sample_array_index = 0;
		size -= len;
	}
}

void PlayerDisplay::audio_callback(Uint8* stream, int len)
{
	int audio_size, len1;
	VideoState* is = pCurStream;
	audio_callback_time = av_gettime_relative();

	while (len > 0) {
		if (is->audio_buf_index >= is->audio_buf_size) {
			audio_size = audio_decode_frame(is);
			if (audio_size < 0) {
				is->audio_buf = NULL;
				is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
			}
			else {
				if (is->show_mode != is->SHOW_MODE_VIDEO)
					update_sample_display(is, (int16_t*)is->audio_buf, audio_size);
				is->audio_buf_size = audio_size;
			}
			is->audio_buf_index = 0;
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if (len1 > len)
			len1 = len;
		if (!bMuted && is->audio_buf && is->audio_volume == SDL_MIX_MAXVOLUME)
			memcpy(stream, (uint8_t*)is->audio_buf + is->audio_buf_index, len1);
		else {
			memset(stream, 0, len1);
			if (is->audio_buf)
				//将音频数据混合到目标音频流
				SDL_MixAudio(stream, (uint8_t*)is->audio_buf + is->audio_buf_index, len1, is->audio_volume);
		}
		len -= len1;
		stream += len1;
		is->audio_buf_index += len1;
	}
	is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
	if (!isnan(is->audio_clock)) {
		set_clock_at(&is->audclk, is->audio_clock - (double)(2 * is->audio_hw_buf_size + is->audio_write_buf_size) / is->audio_tgt.bytes_per_sec, is->audio_clock_serial, audio_callback_time / 1000000.0);
		//时钟主从同步
		sync_clock_to_slave(&is->extclk, &is->audclk);
	}
}

void PlayerDisplay::sdl_audio_callback(void* opaque, Uint8* stream, int len)
{
	PlayerDisplay* pd = static_cast<PlayerDisplay*>(opaque);
	pd->audio_callback(stream, len);
}

int PlayerDisplay::audio_open(AVChannelLayout* wanted_channel_layout, int wanted_sample_rate, struct AudioParams* audio_hw_params)
{
	qDebug() << "Received open audio request:"
		<< "Channel Layout:" << wanted_channel_layout
		<< "Sample Rate:" << wanted_sample_rate
		<< "Audio HW Params:" << audio_hw_params;

	SDL_AudioSpec wanted_spec, spec;
	const char* env;
	static const vector<int> next_nb_channels = { 0, 0, 1, 6, 2, 6, 4, 6 };
	static const vector<int> next_sample_rates = { 0, 44100, 48000, 96000, 192000 };
	int next_sample_rate_idx = next_sample_rates.size() - 1;
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

int PlayerDisplay::decode_interrupt_cb(void* ctx)
{
	VideoState* is = (VideoState*)ctx;
	return is->abort_request;
}

int PlayerDisplay::audio_thread(void* arg)
{
	VideoState* is = static_cast<VideoState*>(arg);
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
			if ((got_frame = decoder_decode_frame(&is->auddec, frame, NULL)) < 0)
			{
				QString error = QString("decode frame failed\n");
				throw PlayerException(error.toStdString(), DECODER_DECODE_FAIL);
			}

			if (got_frame) {
				tb = { 1, frame->sample_rate };

				reconfigure =
					cmp_audio_fmts(is->audio_filter_src.fmt, is->audio_filter_src.ch_layout.nb_channels,
						(AVSampleFormat)frame->format, frame->ch_layout.nb_channels) ||
					av_channel_layout_compare(&is->audio_filter_src.ch_layout, &frame->ch_layout) ||
					is->audio_filter_src.freq != frame->sample_rate ||
					is->auddec.pkt_serial != last_serial;

				if (reconfigure) {
					char buf1[1024], buf2[1024];
					av_channel_layout_describe(&is->audio_filter_src.ch_layout, buf1, sizeof(buf1));
					av_channel_layout_describe(&frame->ch_layout, buf2, sizeof(buf2));

					QString info = QString("Audio frame changed from rate:%1 ch:%2 fmt:%3 layout:%4 serial:%5 to rate:%6 ch:%7 fmt:%8 layout:%9 serial:%10\n")
						.arg(is->audio_filter_src.freq).arg(is->audio_filter_src.ch_layout.nb_channels).arg(av_get_sample_fmt_name(is->audio_filter_src.fmt))
						.arg(buf1).arg(last_serial).arg(frame->sample_rate).arg(frame->ch_layout.nb_channels).arg(av_get_sample_fmt_name((AVSampleFormat)frame->format))
						.arg(buf2).arg(is->auddec.pkt_serial);


					is->audio_filter_src.fmt = (AVSampleFormat)frame->format;
					ret = av_channel_layout_copy(&is->audio_filter_src.ch_layout, &frame->ch_layout);
					if (ret < 0)
					{
						QString error = QString("channel_layout copy failed\n");
						throw PlayerException(error.toStdString(), CHANNAL_LAYOUT_COPY_FAIL);
					}
					is->audio_filter_src.freq = frame->sample_rate;
					last_serial = is->auddec.pkt_serial;

					if ((ret = configure_audio_filters(is, afilters, 1)) < 0)
					{
						QString error = QString("audio filters configure failed\n");
						throw PlayerException(error.toStdString(), AUDIO_FILTER_CONFIGURE_FAIL);
					}
				}

				//视频帧或音频帧添加到缓冲区源滤镜
				if ((ret = av_buffersrc_add_frame(is->in_audio_filter, frame)) < 0)
				{
					QString error = QString("add frame to buffersrc failed\n");
					throw PlayerException(error.toStdString(), ADD_FRAME_FAIL);
				}

				//从缓冲区接收器滤镜（buffer sink filter）中获取一个视频帧或音频帧
				while ((ret = av_buffersink_get_frame_flags(is->out_audio_filter, frame, 0)) >= 0) {
					FrameData* fd = frame->opaque_ref ? (FrameData*)frame->opaque_ref->data : NULL;
					tb = av_buffersink_get_time_base(is->out_audio_filter);
					if (!(af = frame_queue_peek_writable(&is->sampq)))
					{
						QString error = QString("framelist peek failed\n");
						throw PlayerException(error.toStdString(), FRAMELIST_PEEK_FAIL);
					}

					af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
					af->pos = fd ? fd->pkt_pos : -1;
					af->serial = is->auddec.pkt_serial;
					af->duration = av_q2d({ frame->nb_samples, frame->sample_rate });

					av_frame_move_ref(af->frame, frame);
					frame_queue_push(&is->sampq);

					if (is->audioq.serial != is->auddec.pkt_serial)
						break;
				}
				if (ret == AVERROR_EOF)
					is->auddec.finished = is->auddec.pkt_serial;
			}
		} while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}
	avfilter_graph_free(&is->agraph);
	av_frame_free(&frame);
	
	return 0;

}

int PlayerDisplay::video_thread(void* arg)
{
	VideoState* is = static_cast<VideoState*>(arg);
	int ret;
	AVFrame* frame = nullptr;
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
		AVRational tb = is->video_st->time_base;
		AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, nullptr);

		if (!frame)
		{
			ret = AVERROR(ENOMEM);
			QString error = QString("frame alloc failed\n");
			throw PlayerException(error.toStdString(), FRAME_ALLOC_FAIL);
		}

		for (;;) {
			ret = get_video_frame(is, frame);
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
				|| last_serial != is->viddec.pkt_serial
				|| last_vfilter_idx != is->vfilter_idx) 
			{
				QString error = QString("Video frame changed from size:%1x%2 format:%3 serial:%4 to size:%5x%6 format:%7 serial:%8\n")
					.arg(last_w).arg(last_h)
					.arg((const char*)av_x_if_null(av_get_pix_fmt_name(last_format), "none")).arg(last_serial)
					.arg(frame->width).arg(frame->height)
					.arg((const char*)av_x_if_null(av_get_pix_fmt_name((AVPixelFormat)frame->format), "none")).arg(is->viddec.pkt_serial);

				avfilter_graph_free(&graph);
				graph = avfilter_graph_alloc();
				if (!graph) 
				{
					QString error = QString("graph alloc failed\n");
					throw PlayerException(error.toStdString(), GRAPH_ALLOC_FAIL);
				}
				graph->nb_threads = filter_nbthreads;
				if ((ret = configure_video_filters(graph, is,vfilters_list ? vfilters_list[is->vfilter_idx] : nullptr, frame)) < 0)
				{
					SDL_Event event;
					event.type = FF_QUIT_EVENT;
					event.user.data1 = is;
					SDL_PushEvent(&event);
					QString error = QString("video filter configure failed\n");
					throw PlayerException(error.toStdString(), VIDEO_FILTER_CONFIGURE_FAIL);
				}
				filt_in = is->in_video_filter;
				filt_out = is->out_video_filter;
				last_w = frame->width;
				last_h = frame->height;
				last_format = (AVPixelFormat)frame->format;
				last_serial = is->viddec.pkt_serial;
				last_vfilter_idx = is->vfilter_idx;
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

				is->frame_last_returned_time = av_gettime_relative() / 1000000.0;

				ret = av_buffersink_get_frame_flags(filt_out, frame, 0);
				if (ret < 0) {
					if (ret == AVERROR_EOF)
						is->viddec.finished = is->viddec.pkt_serial;
					ret = 0;
					break;
				}

				fd = frame->opaque_ref ? (FrameData*)frame->opaque_ref->data : NULL;

				is->frame_last_filter_delay = av_gettime_relative() / 1000000.0 - is->frame_last_returned_time;
				if (fabs(is->frame_last_filter_delay) > AV_NOSYNC_THRESHOLD / 10.0)
					is->frame_last_filter_delay = 0;
				tb = av_buffersink_get_time_base(filt_out);
				duration = (frame_rate.num && frame_rate.den ? av_q2d({ frame_rate.den, frame_rate.num }) : 0);
				pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
				ret = queue_picture(is, frame, pts, duration, fd ? fd->pkt_pos : -1, is->viddec.pkt_serial);
				if (ret < 0)
				{
					QString error = QString("add frame to list failed\n");
					throw PlayerException(error.toStdString(), ADD_FRAME_FAIL);
				}
				av_frame_unref(frame);
				if (is->videoq.serial != is->viddec.pkt_serial)
					break;
			}
			if (ret < 0)
				throw;

		}
	}
	catch (const PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		avfilter_graph_free(&graph);
		av_frame_free(&frame);
		return ret;
	}
	avfilter_graph_free(&graph);
	av_frame_free(&frame);
	return 0;


}

int PlayerDisplay::subtitle_thread(void* arg)
{
	VideoState* is = static_cast<VideoState*>(arg);
	Frame* sp;
	int got_subtitle;
	double pts;

	try {
		for (;;) {
			if (!(sp = frame_queue_peek_writable(&is->subpq)))
			{
				QString error = QString("frame alloc failed\n");
				throw PlayerException(error.toStdString(), FRAME_ALLOC_FAIL);
			}

			if ((got_subtitle = decoder_decode_frame(&is->subdec, NULL, &sp->sub)) < 0)
			{
				QString error = QString("decoder decode frame failed\n");
				throw PlayerException(error.toStdString(), DECODER_DECODE_FAIL);
			}

			pts = 0;

			if (got_subtitle && sp->sub.format == 0) {
				if (sp->sub.pts != AV_NOPTS_VALUE)
					pts = sp->sub.pts / (double)AV_TIME_BASE;
				sp->pts = pts;
				sp->serial = is->subdec.pkt_serial;
				sp->width = is->subdec.avctx->width;
				sp->height = is->subdec.avctx->height;
				sp->uploaded = 0;

				frame_queue_push(&is->subpq);
			}
			else if (got_subtitle) {
				avsubtitle_free(&sp->sub);
			}
		}
	}
	catch (PlayerException& e)
	{
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
	}

	return 0;
}

void PlayerDisplay::read_thread(VideoState* is)
{
	AVFormatContext* ic = nullptr;
	AVPacket* pkt = nullptr;
	const AVDictionaryEntry* t;
	AVDictionary** opts = nullptr;
	int err, i, ret;
	int st_index[AVMEDIA_TYPE_NB];
	int pkt_in_play_range = 0;
	int scan_all_pmts_set = 0;
	int64_t pkt_ts;
	int64_t stream_start_time;
	SDL_mutex* wait_mutex = nullptr;

	try {
		wait_mutex = SDL_CreateMutex();
		if (!wait_mutex) {
			ret = AVERROR(ENOMEM);
			QString error = QString("%1:%2 SDL_Mutex is nullptr: %3\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), SDL_CREATE_MUTEX_FAIL);
		}

		memset(st_index, -1, sizeof(st_index));
		is->eof = 0;


		pkt = av_packet_alloc();
		if (!pkt)
		{
			ret = AVERROR(ENOMEM);
			QString error = QString("%1:%2 Could not allocate packet.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), PACKET_ALLOC_FAIL);
		}
		ic = avformat_alloc_context();
		if (!ic) {
			ret = AVERROR(ENOMEM);
			QString error = QString("%1:%2 Could not allocate context.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), CONTEXT_ALLOC_FAIL);
		}
		ic->interrupt_callback.callback = PlayerDisplay::decode_interrupt_cb;
		ic->interrupt_callback.opaque = this;

		if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
			av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
			scan_all_pmts_set = 1;
		}

		is->iformat = nullptr;
		err = avformat_open_input(&ic, is->filename, is->iformat, &format_opts);
		if (err < 0) {
			ret = -1;
			QString error = QString("%1:%2 avformat open input failed\n").arg(__FILE__).arg(__LINE__).arg(is->filename);
			throw PlayerException(error.toStdString(), INPUT_OPEN_FAIL);
		}
		is->ic = ic;

		//读取文件生成缺失的 PTS
		if (genpts)
			ic->flags |= AVFMT_FLAG_GENPTS;

		//注入全局侧数据
		av_format_inject_global_side_data(ic);

		//读取文件流信息
		err = avformat_find_stream_info(ic, opts);
		if (err < 0) {
			ret = -1;
			QString error = QString("%1:%2 could not find stream info.\n").arg(__FILE__).arg(__LINE__);
			throw PlayerException(error.toStdString(), STREAM_INFO_GET_FIAL);
		}

		if (ic->pb)
			ic->pb->eof_reached = 0;

		is->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

		if (start_time != AV_NOPTS_VALUE) {
			int64_t timestamp;

			timestamp = start_time;
			if (ic->start_time != AV_NOPTS_VALUE)
				timestamp += ic->start_time;
			ret = avformat_seek_file(ic, -1, INT64_MIN, timestamp, INT64_MAX, 0);
			if (ret < 0) {
				QString error = QString("%1:%2 %3:could not seek to position %4.\n").arg(__FILE__).arg(__LINE__)
					.arg(is->filename).arg((double)timestamp / AV_TIME_BASE);
				qDebug() << error;
			}
		}

		is->realtime = is_realtime(ic);
		emit sigVideoTotalSeconds(ic->duration / 1000000LL);

		for (i = 0; i < ic->nb_streams; i++) {
			AVStream* st = ic->streams[i];
			enum AVMediaType type = st->codecpar->codec_type;
			st->discard = AVDISCARD_ALL;
			if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
				if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0)
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
			av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO,
				st_index[AVMEDIA_TYPE_VIDEO], -1, NULL, 0);
		if (!Audio_disable)
			st_index[AVMEDIA_TYPE_AUDIO] =
			av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO,
				st_index[AVMEDIA_TYPE_AUDIO],
				st_index[AVMEDIA_TYPE_VIDEO],
				NULL, 0);
		if (!Video_disable && !Subtitle_disable)
			st_index[AVMEDIA_TYPE_SUBTITLE] =
			av_find_best_stream(ic, AVMEDIA_TYPE_SUBTITLE,
				st_index[AVMEDIA_TYPE_SUBTITLE],
				(st_index[AVMEDIA_TYPE_AUDIO] >= 0 ?
					st_index[AVMEDIA_TYPE_AUDIO] :
					st_index[AVMEDIA_TYPE_VIDEO]),
				NULL, 0);

		is->show_mode = is->SHOW_MODE_NONE;
		if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
			AVStream* st = ic->streams[st_index[AVMEDIA_TYPE_VIDEO]];
			AVCodecParameters* codecpar = st->codecpar;
			AVRational sar = av_guess_sample_aspect_ratio(ic, st, NULL);
			if (codecpar->width)
				set_default_window_size(codecpar->width, codecpar->height, sar);
		}

		//打开音频流
		if (st_index[AVMEDIA_TYPE_AUDIO] >= 0) {
			stream_component_open(is,st_index[AVMEDIA_TYPE_AUDIO]);
		}

		//打开视频流
		ret = -1;
		if (st_index[AVMEDIA_TYPE_VIDEO] >= 0) {
			ret = stream_component_open(is,st_index[AVMEDIA_TYPE_VIDEO]);
		}
		if (is->show_mode == is->SHOW_MODE_NONE)
			is->show_mode = ret >= 0 ? is->SHOW_MODE_VIDEO : is->SHOW_MODE_RDFT;

		//打开字幕流
		if (st_index[AVMEDIA_TYPE_SUBTITLE] >= 0) {
			stream_component_open(is,st_index[AVMEDIA_TYPE_SUBTITLE]);
		}

		if (is->video_stream < 0 && is->audio_stream < 0) {
			ret = -1;
			QString error = QString("%1:%2 Failed to open file '%3' or configure filtergraph\n").arg(__FILE__).arg(__LINE__)
				.arg(is->filename);
			throw PlayerException(error.toStdString(), FILE_OPEN_FAIL);
		}


		if (infinite_buffer < 0 && is->realtime)
			infinite_buffer = 1;

		for (;;) {
			if (is->abort_request)
				break;
			if (is->paused != is->last_paused) {
				is->last_paused = is->paused;
				if (is->paused)
					is->read_pause_return = av_read_pause(ic);
				else
					av_read_play(ic);
			}

			if (is->seek_req) {
				//目标时间戳
				int64_t seek_target = is->seek_pos;
				//最小时间戳
				int64_t seek_min = is->seek_rel > 0 ? seek_target - is->seek_rel + 2 : INT64_MIN;
				//最大时间戳
				int64_t seek_max = is->seek_rel < 0 ? seek_target - is->seek_rel - 2 : INT64_MAX;

				ret = avformat_seek_file(ic, -1, seek_min, seek_target, seek_max, is->seek_flags);
				if (ret < 0) {
					QString error = QString("%1:%2 %3: error while seeking\n").arg(__FILE__).arg(__LINE__)
						.arg(ic->url);
					qDebug() << error;
				}
				else {
					if (is->audio_stream >= 0)
						packet_queue_flush(&is->audioq);
					if (is->subtitle_stream >= 0)
						packet_queue_flush(&is->subtitleq);
					if (is->video_stream >= 0)
						packet_queue_flush(&is->videoq);
					if (is->seek_flags & AVSEEK_FLAG_BYTE) {
						set_clock(&is->extclk, NAN, 0);
					}
					else {
						set_clock(&is->extclk, seek_target / (double)AV_TIME_BASE, 0);
					}
				}
				is->seek_req = 0;
				is->queue_attachments_req = 1;
				is->eof = 0;
				if (is->paused)
					step_to_next_frame(is);
		}
			if (is->queue_attachments_req) {
				if (is->video_st && is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC) {
					if ((ret = av_packet_ref(pkt, &is->video_st->attached_pic)) < 0)
					{
						QString error = QString("%1:%2 av_packet_ref failed:%3\n").arg(__FILE__).arg(__LINE__)
							.arg(av_err2str(ret));
						throw PlayerException(error.toStdString(), FILE_OPEN_FAIL);
					}
					packet_queue_put(&is->videoq, pkt);
					packet_queue_put_nullpacket(&is->videoq, pkt, is->video_stream);
				}
				is->queue_attachments_req = 0;
			}

			if (infinite_buffer < 1 &&
				(is->audioq.size + is->videoq.size + is->subtitleq.size > MAX_QUEUE_SIZE
					|| (stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq) &&
						stream_has_enough_packets(is->video_st, is->video_stream, &is->videoq) &&
						stream_has_enough_packets(is->subtitle_st, is->subtitle_stream, &is->subtitleq))))
			{
				SDL_LockMutex(wait_mutex);
				SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
				SDL_UnlockMutex(wait_mutex);

				continue;
			}
			if (!is->paused &&
				(!is->audio_st || (is->auddec.finished == is->audioq.serial && frame_queue_nb_remaining(&is->sampq) == 0)) &&
				(!is->video_st || (is->viddec.finished == is->videoq.serial && frame_queue_nb_remaining(&is->pictq) == 0))) 
			{
				if (m_bloop == true) {
					stream_seek(start_time != AV_NOPTS_VALUE ? start_time : 0, 0, 0);
					continue;
				}
				emit sigStop();
			}
			ret = av_read_frame(ic, pkt);
			if (ret < 0) {
				if ((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof) {
					if (is->video_stream >= 0)
						packet_queue_put_nullpacket(&is->videoq, pkt, is->video_stream);
					if (is->audio_stream >= 0)
						packet_queue_put_nullpacket(&is->audioq, pkt, is->audio_stream);
					if (is->subtitle_stream >= 0)
						packet_queue_put_nullpacket(&is->subtitleq, pkt, is->subtitle_stream);
					is->eof = 1;
				}
				if (ic->pb && ic->pb->error)
					break;
				SDL_LockMutex(wait_mutex);
				SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
				SDL_UnlockMutex(wait_mutex);
				continue;
			}
			else {
				is->eof = 0;
			}
			
			stream_start_time = ic->streams[pkt->stream_index]->start_time;
			pkt_ts = pkt->pts == AV_NOPTS_VALUE ? pkt->dts : pkt->pts;
			pkt_in_play_range = AV_NOPTS_VALUE == AV_NOPTS_VALUE ||
				(pkt_ts - (stream_start_time != AV_NOPTS_VALUE ? stream_start_time : 0)) *
				av_q2d(ic->streams[pkt->stream_index]->time_base) -
				(double)(0 != AV_NOPTS_VALUE ? 0 : 0) / 1000000
				<= ((double)AV_NOPTS_VALUE / 1000000);
			//按数据帧的类型存放至对应队列
			if (pkt->stream_index == is->audio_stream && pkt_in_play_range) {
				packet_queue_put(&is->audioq, pkt);
			}
			else if (pkt->stream_index == is->video_stream && pkt_in_play_range
				&& !(is->video_st->disposition & AV_DISPOSITION_ATTACHED_PIC)) {
				packet_queue_put(&is->videoq, pkt);
			}
			else if (pkt->stream_index == is->subtitle_stream && pkt_in_play_range) {
				packet_queue_put(&is->subtitleq, pkt);
			}
			else {
				av_packet_unref(pkt);
			}
		}

		ret = 0;
	}
	catch (PlayerException& e) {
		qDebug() << "PlayerException: " << e.what() << "Error code:" << e.getErrorCode();
		if (ic && !is->ic)
			avformat_close_input(&ic);

		if (ret != 0) {
			SDL_Event event;

			event.type = FF_QUIT_EVENT;
			event.user.data1 = is;
			SDL_PushEvent(&event);
		}
		SDL_DestroyMutex(wait_mutex);
	}
	return ;

}

void PlayerDisplay::LoopThread(VideoState* cur_stream)
{
	SDL_Event event;
	double incr, pos, frac;

	m_bloop = true;

	while (m_bloop)
	{
		double x;
		refresh_loop_wait_event(cur_stream, &event);
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_s: // S: Step to next frame
				step_to_next_frame(cur_stream);
				break;
			case SDLK_a:
				stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
				break;
			case SDLK_v:
				stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);
				break;
			case SDLK_c:
				stream_cycle_channel(cur_stream, AVMEDIA_TYPE_VIDEO);
				stream_cycle_channel(cur_stream, AVMEDIA_TYPE_AUDIO);
				stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE);
				break;
			case SDLK_t:
				stream_cycle_channel(cur_stream, AVMEDIA_TYPE_SUBTITLE);
				break;

			default:
				break;
			}
			break;
		case SDL_WINDOWEVENT:
			//窗口大小改变事件
			switch (event.window.event) {
			case SDL_WINDOWEVENT_RESIZED:
				screen_width = cur_stream->width = event.window.data1;
				screen_height = cur_stream->height = event.window.data2;
			case SDL_WINDOWEVENT_EXPOSED:
				cur_stream->force_refresh = 1;
			}
			break;
		case SDL_QUIT:
		case FF_QUIT_EVENT:
			do_exit(cur_stream);
			break;
		default:
			break;
		}
	}


	do_exit(cur_stream);
}

