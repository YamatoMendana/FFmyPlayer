#include "SDL_Widget.h"

#include <QDebug>




SDL_Widget::SDL_Widget()
{

}

SDL_Widget::~SDL_Widget()
{
	if (pVis_texture)
		SDL_DestroyTexture(pVis_texture);
	if (pVid_texture)
		SDL_DestroyTexture(pVid_texture);
	if (pSub_texture)
		SDL_DestroyTexture(pSub_texture);
}

int SDL_Widget::Init()
{
	flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER;
	if (Audio_disable)
		flags &= ~SDL_INIT_AUDIO;
	else {
		if (!SDL_getenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE"))
			SDL_setenv("SDL_AUDIO_ALSA_SET_BUFFER_SIZE", "1", 1);
	}
	if (Display_disable){	
		flags &= ~SDL_INIT_VIDEO;
	}
	if (SDL_Init(flags)) {
		qDebug() << QString("Could not initialize SDL - %1\n").arg(SDL_GetError());
		exit(1);
	}

	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);

}

int SDL_Widget::setWidget(void* widId)
{
#ifdef SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR
		SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
#endif
		window = SDL_CreateWindowFrom(widId);
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		if (window) {
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (!renderer) {
				qDebug() << QString("Failed to initialize a hardware accelerated renderer: %1").arg(SDL_GetError());
				renderer = SDL_CreateRenderer(window, -1, 0);
			}
			if (renderer) {
				if (!SDL_GetRendererInfo(renderer, &renderer_info))
					qDebug() << QString("Initialized %s renderer. %1").arg(renderer_info.name);
			}
		}
		if (!window || !renderer || !renderer_info.num_texture_formats) {
			qDebug()<<QString("Failed to create window or renderer: %1").arg(SDL_GetError());
			return -1;
		}
}

void SDL_Widget::fill_rectangle(int x, int y, int w, int h)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;
	if (w && h)
		SDL_RenderFillRect(renderer, &rect);
}

int SDL_Widget::realloc_texture(SDL_Texture** texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture)
{
	Uint32 format;
	int access, w, h;
	if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 || new_width != w || new_height != h || new_format != format) {
		void* pixels;
		int pitch;
		if (*texture)
			SDL_DestroyTexture(*texture);
		if (!(*texture = SDL_CreateTexture(renderer, new_format, SDL_TEXTUREACCESS_STREAMING, new_width, new_height)))
			return -1;
		if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
			return -1;
		if (init_texture) {
			if (SDL_LockTexture(*texture, NULL, &pixels, &pitch) < 0)
				return -1;
			memset(pixels, 0, pitch * new_height);
			SDL_UnlockTexture(*texture);
		}
		qDebug()<< QString("Created %1x%2 texture with %3.\n").arg(new_width).arg(new_height).arg(SDL_GetPixelFormatName(new_format));
	}
	return 0;
}

void SDL_Widget::calculate_display_rect(SDL_Rect* rect, 
	int scr_xleft, int scr_ytop, 
	int scr_width, int scr_height, 
	int pic_width, int pic_height, AVRational pic_sar)
{
	AVRational aspect_ratio = pic_sar;
	int64_t width, height, x, y;

	if (av_cmp_q(aspect_ratio, av_make_q(0, 1)) <= 0)
		aspect_ratio = av_make_q(1, 1);

	aspect_ratio = av_mul_q(aspect_ratio, av_make_q(pic_width, pic_height));

	/* XXX: we suppose the screen has a 1.0 pixel ratio */
	height = scr_height;
	width = av_rescale(height, aspect_ratio.num, aspect_ratio.den) & ~1;
	if (width > scr_width) {
		width = scr_width;
		height = av_rescale(width, aspect_ratio.den, aspect_ratio.num) & ~1;
	}
	x = (scr_width - width) / 2;
	y = (scr_height - height) / 2;
	rect->x = scr_xleft + x;
	rect->y = scr_ytop + y;
	rect->w = FFMAX((int)width, 1);
	rect->h = FFMAX((int)height, 1);
}

void SDL_Widget::get_sdl_pix_fmt_and_blendmode(int format, Uint32* sdl_pix_fmt, SDL_BlendMode* sdl_blendmode)
{
	int i;
	*sdl_blendmode = SDL_BLENDMODE_NONE;
	*sdl_pix_fmt = SDL_PIXELFORMAT_UNKNOWN;
	if (format == AV_PIX_FMT_RGB32 ||
		format == AV_PIX_FMT_RGB32_1 ||
		format == AV_PIX_FMT_BGR32 ||
		format == AV_PIX_FMT_BGR32_1)
		*sdl_blendmode = SDL_BLENDMODE_BLEND;
	for (i = 0; i < FF_ARRAY_ELEMS(sdl_texture_format_map) - 1; i++) {
		if (format == sdl_texture_format_map[i].format) {
			*sdl_pix_fmt = sdl_texture_format_map[i].texture_fmt;
			return;
		}
	}
}

int SDL_Widget::upload_texture(SDL_Texture** tex, AVFrame* frame)
{
	int ret = 0;
	Uint32 sdl_pix_fmt;
	SDL_BlendMode sdl_blendmode;
	get_sdl_pix_fmt_and_blendmode(frame->format, &sdl_pix_fmt, &sdl_blendmode);
	if (realloc_texture(tex, sdl_pix_fmt == SDL_PIXELFORMAT_UNKNOWN ? SDL_PIXELFORMAT_ARGB8888 : sdl_pix_fmt, frame->width, frame->height, sdl_blendmode, 0) < 0)
		return -1;
	switch (sdl_pix_fmt) {
	case SDL_PIXELFORMAT_IYUV:
		if (frame->linesize[0] > 0 && frame->linesize[1] > 0 && frame->linesize[2] > 0) {
			ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0], frame->linesize[0],
				frame->data[1], frame->linesize[1],
				frame->data[2], frame->linesize[2]);
		}
		else if (frame->linesize[0] < 0 && frame->linesize[1] < 0 && frame->linesize[2] < 0) {
			ret = SDL_UpdateYUVTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0],
				frame->data[1] + frame->linesize[1] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[1],
				frame->data[2] + frame->linesize[2] * (AV_CEIL_RSHIFT(frame->height, 1) - 1), -frame->linesize[2]);
		}
		else {
			av_log(NULL, AV_LOG_ERROR, "Mixed negative and positive linesizes are not supported.\n");
			return -1;
		}
		break;
	default:
		if (frame->linesize[0] < 0) {
			ret = SDL_UpdateTexture(*tex, NULL, frame->data[0] + frame->linesize[0] * (frame->height - 1), -frame->linesize[0]);
		}
		else {
			ret = SDL_UpdateTexture(*tex, NULL, frame->data[0], frame->linesize[0]);
		}
		break;
	}
	return ret;
}

void SDL_Widget::set_sdl_yuv_conversion_mode(AVFrame* frame)
{
#if SDL_VERSION_ATLEAST(2,0,8)
	SDL_YUV_CONVERSION_MODE mode = SDL_YUV_CONVERSION_AUTOMATIC;
	if (frame && (frame->format == AV_PIX_FMT_YUV420P || frame->format == AV_PIX_FMT_YUYV422 || frame->format == AV_PIX_FMT_UYVY422)) {
		if (frame->color_range == AVCOL_RANGE_JPEG)
			mode = SDL_YUV_CONVERSION_JPEG;
		else if (frame->colorspace == AVCOL_SPC_BT709)
			mode = SDL_YUV_CONVERSION_BT709;
		else if (frame->colorspace == AVCOL_SPC_BT470BG || frame->colorspace == AVCOL_SPC_SMPTE170M)
			mode = SDL_YUV_CONVERSION_BT601;
	}
	SDL_SetYUVConversionMode(mode); /* FIXME: no support for linear transfer */
#endif
}

void SDL_Widget::set_default_window_size(int width, int height, AVRational sar)
{
	SDL_Rect rect;
	int max_width = screen_width ? screen_width : INT_MAX;
	int max_height = screen_height ? screen_height : INT_MAX;
	if (max_width == INT_MAX && max_height == INT_MAX)
		max_height = height;
	calculate_display_rect(&rect, 0, 0, max_width, max_height, width, height, sar);
	default_width = rect.w;
	default_height = rect.h;
}

