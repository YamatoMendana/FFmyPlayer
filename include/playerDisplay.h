#ifndef __PLAYER_DISPLAY_H__
#define __PLAYER_DISPLAY_H__

#include <thread>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

#include "SDL.h"

#include "playerClock.h"
#include "avFrameList.h"
#include "decoder.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"
#include "libavutil/pixfmt.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/bprint.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libswresample/swresample.h"

}
using namespace std;
//表示处理消息的事件回调方法类型
using streamClosehandler = std::function<void()>;

void sigterm_handler(int sig) { exit(1); }

class PlayerDisplay 
{
public:
	explicit PlayerDisplay();
	~PlayerDisplay();

	int stream_open(const char* filename);

	void stream_close();

	void get_default_windowSize(int* width,int* height, AVRational* sar);
private:
	void stream_component_close(int stream_index);
	int stream_component_open(int stream_index);

	
	int check_stream_specifier(AVFormatContext* s, AVStream* st, const char* spec);
	AVDictionary* filter_codec_opts(AVDictionary* opts, enum AVCodecID codec_id,
		AVFormatContext* s, AVStream* st, const AVCodec* codec);
	AVDictionary** setup_find_stream_info_opts(AVFormatContext* s, AVDictionary* codec_opts);

	int is_realtime(AVFormatContext* s);

	
	void audioStreamClose();
	void videoStreamClose();
	void subtitleStreamClose();
	streamClosehandler getHandler(int codec_type);

	int stream_has_enough_packets(AVStream* st, int stream_id, AvPacketList* list);
	void stream_seek(int64_t pos, int64_t rel, int by_bytes);
	void stream_toggle_pause();
	void step_to_next_frame();

	void read_thread();

public:
	inline int get_Abort_request() { return bAbort_request; }
private:
	enum ShowMode {
		SHOW_MODE_NONE = -1, 
		SHOW_MODE_VIDEO = 0, 
		SHOW_MODE_WAVES, 
		SHOW_MODE_RDFT, 
		SHOW_MODE_NB
	} show_mode;

	// 同步器，用于音视频同步
	PlayerClock audclk;
	PlayerClock vidclk;
	PlayerClock extclk;

	// 线程相关
	std::mutex mutex;	//互斥锁
	std::condition_variable cond;	//条件变量

	bool bAbort_request;			//线程中断标志

	//字典
	AVDictionary* sws_dict;		//存储与图像缩放选项
	AVDictionary* swr_opts;		//存储音频重采样选项
	AVDictionary* format_opts;	//存储输入/输出格式选项
	AVDictionary* codec_opts;	//存储编解码器选项

	// 文件相关
	AVFormatContext* pFmtCtx;	// 格式上下文
	AVInputFormat* pIformat;	//存储输入文件的格式信息
	int nEof;					// 文件结束标志
	char* pFilename;			// 文件名

	// 流相关
	AVStream* audio_st;
	AVStream* video_st;
	AVStream* subtitle_st;
	int nVideo_stream;     // 视频流索引
	int nAudio_stream;     // 音频流索引
	int nSubtitle_stream;  // 字幕流索引

	const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = { 0 };	//存储指定的流规范
	int nSt_index[5];

	// 解码器相关
	Decoder auddec;		// 音频解码器上下文
	Decoder viddec;		// 视频解码器上下文
	Decoder subdec;		// 字幕解码器上下文

	// 帧队列
	AvFrameList pictq;  // 视频帧队列
	AvFrameList sampq;  // 音频帧队列
	AvFrameList subpq;  // 字幕帧队列

	// 包队列
	AvPacketList videoq;	// 视频包队列
	AvPacketList audioq;	// 音频包队列
	AvPacketList subtitleq; // 字幕包队列

	// 播放速度控制
	int64_t seek_pos;	// 跳转位置
	int seek_req;		// 跳转请求标志
//定位标志：AVSEEK_FLAG_BACKWARD: 向后查找最近的键帧
// AVSEEK_FLAG_BYTE: 按字节定位
// AVSEEK_FLAG_ANY: 定位到任意帧(不一定是键帧）
// AVSEEK_FLAG_FRAME: 按帧定位。
	int seek_flags;		// 跳转标志
	int64_t nSeek_rel;	//播放跳转偏移量

	// 播放状态
	int nPaused;       // 暂停状态
	int nLast_paused;	// 上一次暂停状态
	int nAttachments_req; // 请求队列附件
	int64_t duration = AV_NOPTS_VALUE;

	//音频相关
	int nAv_sync_type;					// 音视频同步类型
	int nAudio_hw_buf_size;				// 音频硬件缓冲区大小
	uint8_t* pAudio_buf;				// 音频缓冲区
	uint8_t* pAudio_buf1;				// 备用音频缓冲区
	unsigned int nAudio_buf_size;		// 音频缓冲区大小（字节）
	unsigned int nAudio_buf1_size;		// 备用音频缓冲区大小
	int nAudio_buf_index;				// 音频缓冲区索引（字节）
	int nAudio_write_buf_size;			// 音频写入缓冲区大小
	int nAudio_volume;					// 音频音量
	bool bMuted;						// 是否静音

	struct AudioParams struAudio_src;			// 音频源参数
	struct AudioParams struAudio_filter_src;	// 音频滤波器源参数
	struct AudioParams struAudio_tgt;			// 音频目标参数
	struct SwrContext* pSwr_ctx;				// 音频重采样上下文

	int nFrame_drops_early;	// 早期丢帧数
	int nFrame_drops_late;	// 晚期丢帧数

	//音频时钟相关
	double nAudio_clock;				// 音频时钟
	int nAudio_clock_serial;			// 音频时钟序列号
	double nAudio_diff_cum;				// 音频差异累积
	double nAudio_diff_avg_coef;		// 音频差异平均系数
	double nAudio_diff_threshold;		// 音频差异阈值
	int nAudio_diff_avg_count;			// 音频差异平均计数

	//视频相关
	int nForce_refresh;		// 强制刷新
	int nRead_pause_return;	// 读取暂停返回值
	int nRealtime;			// 是否实时流
	int nLast_i_start;		// 最后一个 I 帧的起始位置

	double nFrame_timer;					// 帧计时器
	double nFrame_last_returned_time;	// 上一帧返回时间
	double nFrame_last_filter_delay;		// 上一帧滤波延迟
	double nMax_frame_duration;			// 最大帧持续时间

	int nVfilter_idx;
	AVFilterContext* pIn_video_filter;   // 视频链的第一个滤波器
	AVFilterContext* pOut_video_filter;  // 视频链的最后一个滤波器
	AVFilterContext* pIn_audio_filter;   // 音频链的第一个滤波器
	AVFilterContext* pOut_audio_filter;  // 音频链的最后一个滤波器
	AVFilterGraph* pAgraph;				// 音频滤波器图

	int nLast_video_stream;		//最后一个视频流
	int nLast_audio_stream;		//最后一个音频流
	int nLast_subtitle_stream;	//最后一个字幕流

	int nStep;	// 步进

	//字幕相关
	struct SwsContext* pSub_convert_ctx;	// 字幕转换上下文

	//杂类
	int16_t sample_array[SAMPLE_ARRAY_SIZE];	// 样本数组
	int nSample_array_index;

	RDFTContext* pRdft;		// 快速傅里叶变换上下文
	int nRdft_bits;			// 快速傅里叶变换位数
	FFTSample* pRdft_data;	// 快速傅里叶变换数据

	bool bFind_stream_info = true;
	


	int nXpos;	// X 坐标位置
	// 视频宽度、高度、左上角 X 和 Y 坐标
	int nWidth, nHeight, nXleft, nYtop;
	double nLast_vis_time;	// 最后可视化时间

	//函数管理句柄
	unordered_map<int, streamClosehandler> um_stCloseHandlerMap;


};

int decode_interrupt_cb(void* ctx)
{
	PlayerDisplay* is = (PlayerDisplay*)ctx;
	return is->get_Abort_request();
}






#endif // __PLAYER_DISPLAY_H__
