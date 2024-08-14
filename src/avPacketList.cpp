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

	nAbort_request = 1;
	return 0;
}

void AvPacketList::packet_queue_flush()
{
	MyAVPacketList pkt1;

	std::unique_lock<std::mutex> lck(mutex);
	while (av_fifo_read(pkt_list, &pkt1, 1) >= 0)
		av_packet_free(&pkt1.pkt);
	nNb_packets = 0;
	nSize = 0;
	nDuration = 0;
	nSerial++;
}

void AvPacketList::destroy()
{
	packet_queue_flush();
	av_fifo_freep2(&pkt_list);
}

int AvPacketList::packet_queue_put_private(AVPacket* pkt)
{
	MyAVPacketList pkt1;
	int ret;

	if (nAbort_request)
		return -1;

	pkt1.pkt = pkt;
	pkt1.serial = nSerial;

	ret = av_fifo_write(pkt_list, &pkt1, 1);
	if (ret < 0)
		return ret;
	nNb_packets++;
	nSize += pkt1.pkt->size + sizeof(pkt1);
	nDuration += pkt1.pkt->duration;
	cond.notify_all();
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

	std::unique_lock<std::mutex> lck(mutex);
	ret = packet_queue_put_private(pkt1);
	lck.unlock();

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
	std::unique_lock<std::mutex> lck(mutex);
	nAbort_request = 1;
	cond.notify_all();
}

void AvPacketList::packet_queue_start()
{
	std::unique_lock<std::mutex> lck(mutex);
	nAbort_request = 0;
	nSerial++;
}

int AvPacketList::packet_queue_get(AVPacket* pkt, int block, int* serial)
{
	MyAVPacketList pkt1;
	int ret;

	std::unique_lock<std::mutex> lck(mutex);

	for (;;) {
		if (nAbort_request) {
			ret = -1;
			break;
		}

		if (av_fifo_read(pkt_list, &pkt1, 1) >= 0) {
			nNb_packets--;
			nSize -= pkt1.pkt->size + sizeof(pkt1);
			nDuration -= pkt1.pkt->duration;
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
			cond.wait(lck);
		}
	}
	return ret;
}

void AvPacketList::empty()
{
	std::unique_lock<std::mutex> lck(mutex);
	while (m_packetList.size() > 0)
	{
		MyAVPacketList pktList =  m_packetList.takeFirst();
		AVPacket* packet = pktList.pkt;
		av_packet_unref(packet);
	}
}

bool AvPacketList::isEmpty()
{
	return m_packetList.isEmpty();
}

int AvPacketList::queueSize()
{
	return m_packetList.size();
}

