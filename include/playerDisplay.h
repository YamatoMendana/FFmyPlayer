#ifndef __PLAYER_DISPLAY_H__
#define __PLAYER_DISPLAY_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <future>
#include <QObject>
#include <QWidget>

#include "dataStruct.h"
#include "Common.h"

#define SECONDS_TO_MICROSECONDS(seconds) ((int64_t)(seconds) * 1000000)

using namespace std;
//表示处理消息的事件回调方法类型
using streamClosehandler = std::function<void(VideoState* is)>;


class PlayerDisplay :public QObject
{
	Q_OBJECT
public:
	explicit PlayerDisplay(QObject* parent = nullptr);

	~PlayerDisplay();
	int init();
	bool startPlay(QString filename, WId widId);
	VideoState* stream_open(const char* filename);
	void stream_close(VideoState* is);

	//暂停
	bool toggle_pause();
	//停止
	void stop();
	//静音切换
	bool toggle_mute();
	//修改音量
	int update_volume(int sign, double step);
	//跳指定时间帧
	void stream_seek(int64_t pos, int64_t rel, int by_bytes);
	//跳转指定时间
	void seek(double seconds);
	//向前跳转
	void seek_forward();
	//向后跳转
	void seek_back();
	//切换流类型
	void stream_cycle_channel(VideoState* is, int codec_type);

private:
	void do_exit(VideoState*& is);
	//播放状态切换
	void stream_toggle_pause(VideoState* is);
	//跳到下一帧
	void step_to_next_frame(VideoState* is);
private:
	//旋转
	double display_rotation_get(const int32_t matrix[9]);
	void display_rotation_set(int32_t matrix[9], double angle);
	double get_rotation(int32_t* displaymatrix);
	//实时流判断
	int is_realtime(AVFormatContext* s);
	double vp_duration(VideoState* is,Frame* vp, Frame* nextvp);
	double compute_target_delay(VideoState* is,double delay);
	inline int compute_mod(int a, int b) { return a < 0 ? a % b + b : a % b; }

private:
	//时钟
	double get_clock(Clock* c);
	void set_clock_at(Clock* c, double pts, int serial, double time);
	void set_clock(Clock* c, double pts, int serial);
	void set_clock_speed(Clock* c, double speed);
	void init_clock(Clock* c, int* queue_serial);
	void sync_clock_to_slave(Clock* c, Clock* slave);
	int get_master_sync_type(VideoState* is);//获取主时钟类型
	double get_master_clock(VideoState* is);
	void check_external_clock_speed(VideoState* is);
	void update_video_pts(VideoState* is,double pts, int serial);

	//窗口
	void set_default_window_size(int width, int height, AVRational sar);
	void calculate_display_rect(SDL_Rect* rect,
		int scr_xleft, int scr_ytop,
		int scr_width, int scr_height,
		int pic_width, int pic_height, AVRational pic_sar);

private:
	int stream_component_open(VideoState* is,int stream_index);
	void stream_component_close(VideoState* is,int stream_index);
	//关闭音频流
	void audioStreamClose(VideoState* is);
	//关闭视频流
	void videoStreamClose(VideoState* is);
	//关闭字幕流
	void subtitleStreamClose(VideoState* is);
	streamClosehandler getHandler(int codec_type);
	//音频帧解码
	int audio_decode_frame(VideoState* is);

	inline void fill_rectangle(int x, int y, int w, int h);
	void get_sdl_pix_fmt_and_blendmode(int format, Uint32* sdl_pix_fmt, SDL_BlendMode* sdl_blendmode);

	int realloc_texture(SDL_Texture** texture, Uint32 new_format, int new_width, int new_height, SDL_BlendMode blendmode, int init_texture);
	int upload_texture(SDL_Texture** tex, AVFrame* frame);

	void set_sdl_yuv_conversion_mode(AVFrame* frame);
	void video_image_display(VideoState* is);
	void video_audio_display(VideoState* is);

	void video_display(VideoState* is);
	int video_open(VideoState* is);
	void video_refresh(void* opaque, double* remaining_time);
	void refresh_loop_wait_event(VideoState* is, SDL_Event* event);


	
//过滤器
	//配置滤镜图
	int configure_filtergraph(AVFilterGraph* graph, const char* filtergraph,
		AVFilterContext* source_ctx, AVFilterContext* sink_ctx);
	int insert_vfilter(char* arg);
	//加入滤镜
	void insert_filter(AVFilterContext*& last_filter, AVFilterGraph* graph, const char* name, const char* arg);
	//音频过滤器配置
	int configure_audio_filters(VideoState* is,const char* afilters, int force_output_format);
	//视频过滤器配置
	int configure_video_filters(AVFilterGraph* graph, VideoState* is,const char* vfilters, AVFrame* frame);
	
	//流是否有足够的包进行播放
	int stream_has_enough_packets(AVStream* st, int stream_id, PacketQueue* queue);

//视频部分
	//视频帧加入队列
	int queue_picture(VideoState* is, AVFrame* src_frame, double pts, double duration, int64_t pos, int serial);
	//获取视频帧
	int get_video_frame(VideoState* is, AVFrame* frame);

//音频部分
	//调整音频帧的时间戳
	int synchronize_audio(VideoState* is,int nb_samples);
	//比较音频格式
	inline int cmp_audio_fmts(enum AVSampleFormat fmt1, int64_t channel_count1,
		enum AVSampleFormat fmt2, int64_t channel_count2);

	void update_sample_display(VideoState* is,short* samples, int samples_size);
	void audio_callback(Uint8* stream, int len);
	static void sdl_audio_callback(void* opaque, Uint8* stream, int len);
	//打开音频
	int audio_open(AVChannelLayout* wanted_channel_layout, int wanted_sample_rate,
		struct AudioParams* audio_hw_params);

	//解码回调
	static int decode_interrupt_cb(void* ctx);

	int audio_thread(void* arg);
	int video_thread(void* arg);
	int subtitle_thread(void* arg);

	void read_thread(VideoState* is);
	void LoopThread(VideoState* cur_stream);
	

signals:
	void sigStop();
	void sigStopFinish();
	void sigVideoPlaySeconds(double seconds);
	void sigVideoTotalSeconds(int seconds);
	void sigFileOpen(QString filename);
	void sigVideoVolume(double val);
	void sigPauseState(bool state);
public slots:

public:
	// 线程相关
	std::mutex mutex;	//互斥锁
	std::condition_variable cond;	//条件变量
	std::thread m_tPlayLoopThread;

	VideoState* pCurStream = nullptr;
	WId m_widId;//播放窗口
	//字典
	AVDictionary* sws_dict = nullptr;		//存储与图像缩放选项
	AVDictionary* swr_opts = nullptr;		//存储音频重采样选项
	AVDictionary* format_opts = nullptr;

	const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = { 0 };	//存储指定的流规范

	bool bMuted = false;						// 是否静音
	int nStartup_volume = 100;					//初始音量
	int nCurrent_volume = 0;

	double rdftspeed = 0.02; //快速傅里叶变换速度

private:
	SDL_Texture* pVis_texture = nullptr;	// 可视化纹理
	SDL_Texture* pSub_texture = nullptr;	// 字幕纹理
	SDL_Texture* pVid_texture = nullptr;	// 视频纹理

	SDL_Renderer* renderer = nullptr;
	SDL_RendererInfo renderer_info = { 0 };
	SDL_Window* window = nullptr;
	SDL_AudioDeviceID audio_dev = 0;

	int default_width;
	int default_height;
	int screen_width;
	int screen_height;
	int screen_left;
	int screen_top;

	int flags;
	bool bFull_screen = false;

//杂类
	//函数管理句柄
	unordered_map<int, streamClosehandler> um_stCloseHandlerMap;

public:
	int filter_nbthreads = 0;	//过滤器线程数
	int64_t audio_callback_time = 0;	//记录音频回调函数被调用时的时间戳
	int genpts = 1;	//更新PTS
	int64_t start_time = AV_NOPTS_VALUE;	//播放开始时间
	bool Audio_disable = false;//是否禁用音频播放
	bool Video_disable = false;//是否禁用视频播放
	bool Subtitle_disable = false;//禁用字幕显示
	bool Display_disable = false;//是否禁用显示功能

	int infinite_buffer = -1;//控制输入缓冲区行为
	bool m_bloop = true;	//播放循环
	int framedrop = -1;//丢帧
	int autorotate = 1;//旋转

	char** vfilters_list = nullptr;
	int nNb_vfilters = 0;

	char* afilters = nullptr;
	bool bInit = false;//初始化标志

	int incr = 5;//跳转秒数

};








#endif // __PLAYER_DISPLAY_H__
