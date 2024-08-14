#ifndef __AV_PACKET_QUEUE_H__
#define __AV_PACKET_QUEUE_H__

#include <QList>
#include <mutex>
#include <condition_variable>

#include "Common.h"


extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/fifo.h"
}

using namespace std;

class AvPacketList
{
public:
	explicit AvPacketList();
	~AvPacketList();
	int init();
	void packet_queue_flush();
	void destroy();

public:
	int packet_queue_put_private(AVPacket* pkt);
	int packet_queue_put(AVPacket* pkt);
	int packet_queue_put_nullpacket(AVPacket* pkt, int stream_index);

	void packet_queue_abort();
	void packet_queue_start();


public:
	int packet_queue_get(AVPacket* pkt, int block, int* serial);

	void empty();
	bool isEmpty();
	int queueSize();

public:
	inline void set_serial(int serial) { nSerial = serial; };
	inline int get_serial() { return nSerial; }

	inline void set_abort_request(int abort_request) { nAbort_request = abort_request; }
	inline int get_abort_request() { return nAbort_request; }

	inline void set_duration(int64_t duration) { nDuration = duration; }
	inline int64_t get_duration() { return nDuration; }

	inline void set_size(int size) { nSize = size; }
	inline int get_size() { return nSize; }

	inline void set_nb_packets(int nb_packets) { nNb_packets = nb_packets; }
	inline int get_nb_packets() { return nNb_packets; }

private:
	std::mutex mutex;
	std::condition_variable cond;

	AVFifo* pkt_list;

	int nNb_packets;
	int nSize;
	int64_t nDuration;
	int nAbort_request;
	int nSerial;

	QList<MyAVPacketList> m_packetList;

};





#endif // __AV_PACKET_QUEUE_H__
