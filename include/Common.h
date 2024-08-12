#ifndef __Common_H__
#define __Common_H__

#include <iostream>
#include <stdexcept>

#include "SDL.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_ARRAY_SIZE (8 * 65536)
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define AUDIO_DIFF_AVG_NB   20
#define SDL_AUDIO_MIN_BUFFER_SIZE 512

//最大的音频速度变化，以获得正确的同步
#define SAMPLE_CORRECTION_PERCENT_MAX 10

static bool Audio_disable;	//是否禁用音频播放
static bool Video_disable;	//是否禁用视频播放
static bool Subtitle_disable;	//禁用字幕显示
static bool Display_disable;	//是否禁用显示功能

static int startup_volume = 100;//默认音频音量
static int genpts = 0;	//更新PTS
static int seek_by_bytes = -1;	//时间戳查找
static int64_t start_time = AV_NOPTS_VALUE;
static int infinite_buffer = -1;	//控制输入缓冲区行为
static int loop = 1;	//播放循环
static int autoexit;	//自动退出


typedef struct Frame {
	//帧数据
	AVFrame* frame;
	//字幕数据
	AVSubtitle sub;
	
	int serial;
	double pts;
	double duration; 

	int64_t pos;          
	int width;
	int height;
	int format;
	AVRational sar;
	int uploaded;
	int flip_v;
} Frame;

typedef struct MyAVPacketList {
	AVPacket* pkt;
	int serial;
} MyAVPacketList;

typedef struct FrameData {
	int64_t pkt_pos;
} FrameData;

typedef struct AudioParams {
	//音频采样率：44.1k，48k，奈奎斯特采样原理
	int freq;
	//通道布局
	AVChannelLayout ch_layout;
	enum AVSampleFormat fmt;
	//音频缓冲区大小，字节单位
	int frame_size;
	//每秒字节数
	int bytes_per_sec;
} AudioParams;


static const struct TextureFormatEntry {
	enum AVPixelFormat format;
	int texture_fmt;
} sdl_texture_format_map[] = {
	{ AV_PIX_FMT_RGB8,           SDL_PIXELFORMAT_RGB332 },
	{ AV_PIX_FMT_RGB444,         SDL_PIXELFORMAT_RGB444 },
	{ AV_PIX_FMT_RGB555,         SDL_PIXELFORMAT_RGB555 },
	{ AV_PIX_FMT_BGR555,         SDL_PIXELFORMAT_BGR555 },
	{ AV_PIX_FMT_RGB565,         SDL_PIXELFORMAT_RGB565 },
	{ AV_PIX_FMT_BGR565,         SDL_PIXELFORMAT_BGR565 },
	{ AV_PIX_FMT_RGB24,          SDL_PIXELFORMAT_RGB24 },
	{ AV_PIX_FMT_BGR24,          SDL_PIXELFORMAT_BGR24 },
	{ AV_PIX_FMT_0RGB32,         SDL_PIXELFORMAT_RGB888 },
	{ AV_PIX_FMT_0BGR32,         SDL_PIXELFORMAT_BGR888 },
	{ AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888 },
	{ AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888 },
	{ AV_PIX_FMT_RGB32,          SDL_PIXELFORMAT_ARGB8888 },
	{ AV_PIX_FMT_RGB32_1,        SDL_PIXELFORMAT_RGBA8888 },
	{ AV_PIX_FMT_BGR32,          SDL_PIXELFORMAT_ABGR8888 },
	{ AV_PIX_FMT_BGR32_1,        SDL_PIXELFORMAT_BGRA8888 },
	{ AV_PIX_FMT_YUV420P,        SDL_PIXELFORMAT_IYUV },
	{ AV_PIX_FMT_YUYV422,        SDL_PIXELFORMAT_YUY2 },
	{ AV_PIX_FMT_UYVY422,        SDL_PIXELFORMAT_UYVY },
	{ AV_PIX_FMT_NONE,           SDL_PIXELFORMAT_UNKNOWN },
};

enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

enum ErrorCode
{
	COMMON_FAIL = 0,

	WITHOUT_FILE_NAME = 1000,
	FILE_OPEN_FAIL,
	PACKET_ALLOC_FAIL,
	CONTEXT_ALLOC_FAIL,
	INPUT_OPEN_FAIL,
	STREAM_INFO_GET_FIAL,

	PICTURE_LIST_INIT_FAIL = 1010,
	SUBTITLE_LIST_INIT_FAIL,
	AUDIO_LIST_INIT_FAIL,
	PACKET_LIST_INIT_FAIL,

	SDL_CREATE_COND_FAIL = 1020,
	SDL_CREATE_MUTEX_FAIL,
	SDL_CREATE_THREAD_FAIL,

	AVFILTER_CREATE_FAIL = 1030,

	OPTIONAL_SET_FAIL = 1040,
	FILTERGRAPH_CONFIGURE_FAIL,
	FILTER_INPUT_CRATER_FAIL,
	FILTER_PARSE_FAIL,
	FILTER_LINK_FAIL,

	DICT_GET_FAIL = 1050,
	
	
};

class PlayerException :public std::exception
{
public:
	PlayerException(const std::string& message,int errorCode)
		: message(message), errorCode(errorCode) {}

	const char* what() const noexcept override 
	{
		return message.c_str();
	}

	int getErrorCode() const 
	{
		return errorCode;
	}


private:
	std::string message;
	int errorCode;
};



#endif //__GLOBAL_STRUCT_H__
