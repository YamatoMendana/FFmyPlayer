#ifndef __GLOBAL_STRUCT_H__
#define __GLOBAL_STRUCT_H__

extern "C"
{
#include "libavcodec/avcodec.h"
}

typedef struct Frame {
	AVFrame* frame;
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

typedef struct Clock {
	double pts;           /* clock base */
	double pts_drift;     /* clock base minus time at which we updated the clock */
	double last_updated;
	double speed;
	int serial;           /* clock is based on a packet with this serial */
	int paused;
	int* queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;



#endif //__GLOBAL_STRUCT_H__
