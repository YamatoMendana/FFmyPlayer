#ifndef __DECODER_H__
#define __DECODER_H__

#include <condition_variable>
#include <mutex>
#include <functional>

#include "SDL.h"
#include "AvFrameList.h"
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
	int decoder_init(AVCodecContext* avctx, AvPacketList* queue, std::condition_variable* cond);

	void decoder_destroy();
	int decoder_decode_frame(AVFrame* frame, AVSubtitle* sub);
	void decoder_abort(AvFrameList* frameList);

	template <typename T, typename... Args> int decoder_start(void (T::* fn)(Args...), T& obj, Args... args) {
		pPktList->packet_queue_start();
		std::thread t(std::bind(fn, &obj, args...));
		t.join();
		return 0;
	}
public:
	inline void set_finished(int num) { finished = num; }
	inline int get_finished() { return finished; }
	inline void set_serial(int num) { pkt_serial = num; }
	inline int get_serial() { return pkt_serial; }
	inline void set_start_pts(int pts) { start_pts = pts; }
	inline int64_t get_start_pts() { return start_pts; }
	inline void set_next_pts(int64_t pts) { next_pts = pts; }
	inline int64_t get_next_pts() { return next_pts; }
	inline void set_start_pts_tb(AVRational tb) { start_pts_tb = tb; }
	inline AVRational get_start_pts_tb() { return start_pts_tb; }
	inline void set_next_pts_tb(AVRational tb) { next_pts_tb = tb; }
	inline AVRational get_next_pts_tb() { return next_pts_tb; }

private:
	AVPacket* pkt = nullptr;
	AvPacketList* pPktList = nullptr;
	AvFrameList* pFmList = nullptr;
	AVCodecContext* avctx = nullptr;
	std::condition_variable* cond = nullptr;
	std::mutex mutex;

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
