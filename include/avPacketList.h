#ifndef __AV_PACKET_QUEUE_H__
#define __AV_PACKET_QUEUE_H__

#include <QList>
#include "SDL.h"
#include "globalStruct.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/fifo.h"
}



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

private:
	SDL_mutex* pSDL_mutex;
	SDL_cond* pSDL_cond;

public:
	AVFifo* pkt_list;
	int nb_packets;
	int size;
	int64_t duration;
	int abort_request;
	int serial;

	QList<MyAVPacketList> m_packetList;


};





#endif // __AV_PACKET_QUEUE_H__
