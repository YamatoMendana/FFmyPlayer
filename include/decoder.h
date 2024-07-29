#ifndef __DECODER_H__
#define __DECODER_H__

#include "SDL.h"
#include "avFrameList.h"
#include "avPacketList.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

class Decoder 
{
public:
	explicit Decoder();
	~Decoder();
	int decoder_init(AVCodecContext* avctx, AvPacketList* queue, SDL_cond* empty_queue_cond);

	void decoder_destroy(Decoder* d);
	int decoder_decode_frame(AVFrame* frame, AVSubtitle* sub);

	void decoder_abort();

private:
	AVPacket* pkt = nullptr;
	AvPacketList* pPktList = nullptr;
	AVFrameList* pFmList = nullptr;
	AVCodecContext* avctx = nullptr;
	SDL_cond* empty_queue_cond = nullptr;
	SDL_Thread* decoder_tid = nullptr;

	int pkt_serial;
	int finished;
	int packet_pending;
	
	int64_t start_pts;
	AVRational start_pts_tb;
	int64_t next_pts;
	AVRational next_pts_tb;
	
	int decoder_reorder_pts = -1;
	
};




#endif // __DECODER_H__
