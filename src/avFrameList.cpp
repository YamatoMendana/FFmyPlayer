#include "avFrameList.h"

#include <QDebug>

AVFrameList::AVFrameList()
{

}

AVFrameList::~AVFrameList()
{
	
}

int AVFrameList::init(int max_size, int keep_last)
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

	max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
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

void AVFrameList::destory()
{
	int i;
	for (i = 0; i < max_size; i++) {
		Frame* vp = &m_frameList[i];
		frame_queue_unref_item(vp);
		av_frame_free(&vp->frame);
	}
	m_frameList.clear();
	SDL_DestroyMutex(pSDL_mutex);
	SDL_DestroyCond(pSDL_cond);
}

void AVFrameList::frame_queue_unref_item(Frame* vp)
{
	av_frame_unref(vp->frame);
	avsubtitle_free(&vp->sub);
}


Frame* AVFrameList::frame_queue_peek_writable()
{
	SDL_LockMutex(pSDL_mutex);
	while (size >= max_size &&
		! pktq->abort_request) {
		SDL_CondWait(pSDL_cond, pSDL_mutex);
	}
	SDL_UnlockMutex(pSDL_mutex);

	if (pktq->abort_request)
		return NULL;

	return &m_frameList[windex];
}

Frame* AVFrameList::frame_queue_peek_readable()
{
	/* wait until we have a readable a new frame */
	SDL_LockMutex(pSDL_mutex);
	while (size - rindex_shown <= 0 &&
		!pktq->abort_request) {
		SDL_CondWait(pSDL_cond,pSDL_mutex);
	}
	SDL_UnlockMutex(pSDL_mutex);

	if (pktq->abort_request)
		return NULL;

	return &m_frameList[(rindex + rindex_shown) % max_size];
}

void AVFrameList::frame_queue_push()
{
	if (++windex == max_size)
		windex = 0;
	SDL_LockMutex(pSDL_mutex);
	size++;
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

void AVFrameList::frame_queue_next()
{
	if (keep_last && !rindex_shown) {
		rindex_shown = 1;
		return;
	}
	frame_queue_unref_item(&m_frameList[rindex]);
	if (++rindex == max_size)
		rindex = 0;
	SDL_LockMutex(pSDL_mutex);
	size--;
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

void AVFrameList::frame_queue_signal()
{
	SDL_LockMutex(pSDL_mutex);
	SDL_CondSignal(pSDL_cond);
	SDL_UnlockMutex(pSDL_mutex);
}

int AVFrameList::frame_queue_nb_remaining()
{
	return size - rindex_shown;
}

int64_t AVFrameList::frame_queue_last_pos()
{
	Frame* fp = &m_frameList[rindex];
	if (rindex_shown && fp->serial == pktq->serial)
		return fp->pos;
	else
		return -1;
}

void AVFrameList::empty()
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

bool AVFrameList::isEmpty()
{
	return m_frameList.isEmpty();
}

int AVFrameList::queueSize()
{
	return m_frameList.size();
}

Frame* AVFrameList::frame_queue_peek()
{
	return &m_frameList[(rindex + rindex_shown) % max_size];
}

Frame* AVFrameList::frame_queue_peek_next()
{
	return &m_frameList[(rindex + rindex_shown + 1) % max_size];
}

Frame* AVFrameList::frame_queue_peek_last()
{
	return &m_frameList[rindex];
}
