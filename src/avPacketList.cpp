#include "avPacketList.h"
#include <QDebug>

AvPacketList::AvPacketList()
{

}

AvPacketList::~AvPacketList()
{

}

int AvPacketList::init()
{
	pkt_list = av_fifo_alloc2(1, sizeof(MyAVPacketList), AV_FIFO_FLAG_AUTO_GROW);
	if (!pkt_list)
		return AVERROR(ENOMEM);
	pSDL_mutex = SDL_CreateMutex();
	if (!pSDL_mutex)
	{
		QString strErr = QString("SDL_CreateMutex():%1\n").arg(SDL_GetError());
		qDebug() << strErr;
		return AVERROR(ENOMEM);
	}
	pSDL_cond = SDL_CreateCond();
	if (!pSDL_cond)
	{
		QString strErr = QString("SDL_CreateCond():%1\n").arg(SDL_GetError());
		qDebug() << strErr;
		return AVERROR(ENOMEM);
	}
	abort_request = 1;
	return 0;
}

void AvPacketList::packet_queue_flush()
{
	MyAVPacketList pkt1;

	SDL_LockMutex(pSDL_mutex);
	while (av_fifo_read(pkt_list, &pkt1, 1) >= 0)
		av_packet_free(&pkt1.pkt);
	nb_packets = 0;
	size = 0;
	duration = 0;
	serial++;
	SDL_UnlockMutex(pSDL_mutex);
}

void AvPacketList::destroy()
{
	packet_queue_flush();
	av_fifo_freep2(&pkt_list);
	SDL_DestroyMutex(pSDL_mutex);
	SDL_DestroyCond(pSDL_cond);
}

int AvPacketList::packet_queue_put_private(AVPacket* pkt)
{
	MyAVPacketList pkt1;
	int ret;

	if (abort_request)
		return -1;

	pkt1.pkt = pkt;
	pkt1.serial = serial;

	ret = av_fifo_write(pkt_list, &pkt1, 1);
	if (ret < 0)
		return ret;
	nb_packets++;
	size += pkt1.pkt->size + sizeof(pkt1);
	duration += pkt1.pkt->duration;
	SDL_CondSignal(pSDL_cond);
	return 0;
}



int AvPacketList::packet_queue_put(AVPacket* pkt)
{
	AVPacket* pkt1;
	int ret;

	pkt1 = av_packet_alloc();
	if (!pkt1) {
		av_packet_unref(pkt);
		return -1;
	}
	av_packet_move_ref(pkt1, pkt);

	SDL_LockMutex(pSDL_mutex);
	ret = packet_queue_put_private(pkt1);
	SDL_UnlockMutex(pSDL_mutex);

	if (ret < 0)
		av_packet_free(&pkt1);

	return ret;
}

int AvPacketList::packet_queue_put_nullpacket(AVPacket* pkt, int stream_index)
{
	pkt->stream_index = stream_index;
	return packet_queue_put(pkt);
}

void AvPacketList::packet_queue_abort()
{
	SDL_LockMutex(pSDL_mutex);
	abort_request = 1;
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

void AvPacketList::packet_queue_start()
{
	SDL_LockMutex(pSDL_mutex);
	abort_request = 0;
	serial++;
	SDL_UnlockMutex(pSDL_mutex);
}

int AvPacketList::packet_queue_get(AVPacket* pkt, int block, int* serial)
{
	MyAVPacketList pkt1;
	int ret;

	SDL_LockMutex(pSDL_mutex);

	for (;;) {
		if (abort_request) {
			ret = -1;
			break;
		}

		if (av_fifo_read(pkt_list, &pkt1, 1) >= 0) {
			nb_packets--;
			size -= pkt1.pkt->size + sizeof(pkt1);
			duration -= pkt1.pkt->duration;
			av_packet_move_ref(pkt, pkt1.pkt);
			if (serial)
				*serial = pkt1.serial;
			av_packet_free(&pkt1.pkt);
			ret = 1;
			break;
		}
		else if (!block) {
			ret = 0;
			break;
		}
		else {
			SDL_CondWait(pSDL_cond, pSDL_mutex);
		}
	}
	SDL_UnlockMutex(pSDL_mutex);
	return ret;
}

void AvPacketList::empty()
{
	SDL_LockMutex(pSDL_mutex);
	while (m_packetList.size() > 0)
	{
		MyAVPacketList pktList =  m_packetList.takeFirst();
		AVPacket* packet = pktList.pkt;
		av_packet_unref(packet);
	}
	SDL_UnlockMutex(pSDL_mutex);
}

bool AvPacketList::isEmpty()
{
	return m_packetList.isEmpty();
}

int AvPacketList::queueSize()
{
	return m_packetList.size();
}
