#ifndef __AV_FRAME_QUEUE_H__
#define __AV_FRAME_QUEUE_H__

#include <QList>
#include <mutex>
#include <condition_variable>

#include "Common.h"
#include "avPacketList.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}
#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))




class AvFrameList
{
public:
	explicit AvFrameList();
	~AvFrameList();
	int init(AvPacketList* pktq, int max_size, int keep_last);
	void destory();
	void frame_queue_unref_item(Frame* vp);
	Frame* frame_queue_peek_writable();
	Frame* frame_queue_peek_readable();

	void frame_queue_push();
	void frame_queue_next();
	void frame_queue_signal();

	int frame_queue_nb_remaining();
	int64_t frame_queue_last_pos();


public:
	void empty();
	bool isEmpty();
	int queueSize();

	Frame* frame_queue_peek();
	Frame* frame_queue_peek_next();
	Frame* frame_queue_peek_last();

	

private:
	std::mutex mutex;
	std::condition_variable cond;

public:
	int nRindex;
	int nWindex;
	int nSize;
	int nMax_size;
	int keep_last;
	int nRindex_shown;

	QList<Frame> m_frameList;
	AvPacketList* pPktList;
};



#endif // __AV_FRAME_QUEUE_H__
