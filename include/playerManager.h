#ifndef __PLAYER_MANAGER_H__
#define __PLAYER_MANAGER_H__

#include "SDL.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
}


class PlayerManager
{
public:
	explicit PlayerManager();
	~PlayerManager();



};





#endif // __PLAYER_MANAGER_H__

