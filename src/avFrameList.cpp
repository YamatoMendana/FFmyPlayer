#include "avFrameList.h"

#include <QDebug>

AvFrameList::AvFrameList()
{

}

AvFrameList::~AvFrameList()
{
	
}

int AvFrameList::init(AvPacketList* pktq,int max_size, int keep_last)
{
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
	pPktList = pktq;
	nMax_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
	keep_last = !!keep_last;
	for (int i = 0; i < max_size; i++)
	{
		Frame frame;
		m_frameList.push_back(frame);
		if (!(m_frameList[i].frame = av_frame_alloc()))
			return AVERROR(ENOMEM);
	}
	return 0;

}

void AvFrameList::destory()
{
	int i;
	for (i = 0; i < nMax_size; i++) {
		Frame* vp = &m_frameList[i];
		frame_queue_unref_item(vp);
		av_frame_free(&vp->frame);
	}
	m_frameList.clear();
	SDL_DestroyMutex(pSDL_mutex);
	SDL_DestroyCond(pSDL_cond);
}

void AvFrameList::frame_queue_unref_item(Frame* vp)
{
	av_frame_unref(vp->frame);
	avsubtitle_free(&vp->sub);
}


Frame* AvFrameList::frame_queue_peek_writable()
{
	SDL_LockMutex(pSDL_mutex);
	while (nSize >= nMax_size &&
		!pPktList->get_abort_request()) {
		SDL_CondWait(pSDL_cond, pSDL_mutex);
	}
	SDL_UnlockMutex(pSDL_mutex);

	if (pPktList->get_abort_request())
		return NULL;

	return &m_frameList[nWindex];
}

Frame* AvFrameList::frame_queue_peek_readable()
{
	/* wait until we have a readable a new frame */
	SDL_LockMutex(pSDL_mutex);
	while (nSize - nRindex_shown <= 0 &&
		!pPktList->get_abort_request()) {
		SDL_CondWait(pSDL_cond,pSDL_mutex);
	}
	SDL_UnlockMutex(pSDL_mutex);

	if (pPktList->get_abort_request())
		return NULL;

	return &m_frameList[(nRindex + nRindex_shown) % nMax_size];
}

void AvFrameList::frame_queue_push()
{
	if (++nWindex == nMax_size)
		nWindex = 0;
	SDL_LockMutex(pSDL_mutex);
	nSize++;
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

void AvFrameList::frame_queue_next()
{
	if (keep_last && !nRindex_shown) {
		nRindex_shown = 1;
		return;
	}
	frame_queue_unref_item(&m_frameList[nRindex]);
	if (++nRindex == nMax_size)
		nRindex = 0;
	SDL_LockMutex(pSDL_mutex);
	nSize--;
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

void AvFrameList::frame_queue_signal()
{
	SDL_LockMutex(pSDL_mutex);
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

int AvFrameList::frame_queue_nb_remaining()
{
	return nSize - nRindex_shown;
}

int64_t AvFrameList::frame_queue_last_pos()
{
	Frame* fp = &m_frameList[nRindex];
	if (nRindex_shown && fp->serial == pPktList->get_serial())
		return fp->pos;
	else
		return -1;
}

void AvFrameList::empty()
{
	SDL_LockMutex(pSDL_mutex);
	while (m_frameList.size() > 0)
	{
		Frame fm= m_frameList.takeFirst();
		AVFrame* frame = fm.frame;
		av_frame_unref(frame);
	}
	SDL_UnlockMutex(pSDL_mutex);

}

bool AvFrameList::isEmpty()
{
	return m_frameList.isEmpty();
}

int AvFrameList::queueSize()
{
	return m_frameList.size();
}

Frame* AvFrameList::frame_queue_peek()
{
	return &m_frameList[(nRindex + nRindex_shown) % nMax_size];
}

Frame* AvFrameList::frame_queue_peek_next()
{
	return &m_frameList[(nRindex + nRindex_shown + 1) % nMax_size];
}

Frame* AvFrameList::frame_queue_peek_last()
{
	return &m_frameList[nRindex];
}
