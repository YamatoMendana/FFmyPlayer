#ifndef __SDL_DISPLAY_H__
#define __SDL_DISPLAY_H__

#include "SDL.h"
#include "Common.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

class SDL_Widget
{
public:
	explicit SDL_Widget();
	~SDL_Widget();

	int Init();
	int setWidget(void* widId);
	inline void fill_rectangle(int x, int y, int w, int h);
	int realloc_texture(SDL_Texture** texture, Uint32 new_format,
		int new_width, int new_height, 
		SDL_BlendMode blendmode, int init_texture);
	void calculate_display_rect(SDL_Rect* rect,
		int scr_xleft, int scr_ytop, 
		int scr_width, int scr_height,
		int pic_width, int pic_height, AVRational pic_sar);

	void get_sdl_pix_fmt_and_blendmode(int format, Uint32* sdl_pix_fmt, SDL_BlendMode* sdl_blendmode);
	int upload_texture(SDL_Texture** tex, AVFrame* frame);
	void set_sdl_yuv_conversion_mode(AVFrame* frame);
	void set_default_window_size(int width, int height, AVRational sar);

	void CloseAudioDevice() { SDL_CloseAudioDevice(audio_dev); }


public:
	
private:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_RendererInfo renderer_info = { 0 };
	SDL_AudioDeviceID audio_dev;

	SDL_Texture* pVis_texture;	// 可视化纹理
	SDL_Texture* pSub_texture;	// 字幕纹理
	SDL_Texture* pVid_texture;	// 视频纹理

	int default_width;
	int default_height;
	int screen_width;
	int screen_height;
	int screen_left;
	int screen_top;

	int flags;
};



#endif // __SDL_DISPLAY_H__
