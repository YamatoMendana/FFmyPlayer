#ifndef __Common_H__
#define __Common_H__

#include <iostream>
#include <stdexcept>
#include <shared_mutex>
#include <mutex>
#include <variant>
#include <map>
#include <memory>
#include <typeinfo>

#include "SDL.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0
#define SAMPLE_ARRAY_SIZE (8 * 65536)
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define AUDIO_DIFF_AVG_NB   20
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
//每秒音频回调的最大次数
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30
//最大的音频速度变化，以获得正确的同步
#define SAMPLE_CORRECTION_PERCENT_MAX 10

typedef struct Frame {
	//帧数据
	AVFrame* frame;
	//字幕数据
	AVSubtitle sub;
	
	int serial;
	double pts;
	double duration; 

	int64_t pos;          
	int width;
	int height;
	int format;
	AVRational sar;
	int uploaded;
	int flip_v;
} Frame;

typedef struct MyAVPacketList {
	AVPacket* pkt;
	int serial;
} MyAVPacketList;

typedef struct FrameData {
	int64_t pkt_pos;
} FrameData;

typedef struct AudioParams {
	//音频采样率：44.1k，48k，奈奎斯特采样原理
	int freq;
	//通道布局
	AVChannelLayout ch_layout;
	enum AVSampleFormat fmt;
	//音频缓冲区大小，字节单位
	int frame_size;
	//每秒字节数
	int bytes_per_sec;
} AudioParams;


static const struct TextureFormatEntry {
	enum AVPixelFormat format;
	int texture_fmt;
} sdl_texture_format_map[] = {
	{ AV_PIX_FMT_RGB8,           SDL_PIXELFORMAT_RGB332 },
	{ AV_PIX_FMT_RGB444,         SDL_PIXELFORMAT_RGB444 },
	{ AV_PIX_FMT_RGB555,         SDL_PIXELFORMAT_RGB555 },
	{ AV_PIX_FMT_BGR555,         SDL_PIXELFORMAT_BGR555 },
	{ AV_PIX_FMT_RGB565,         SDL_PIXELFORMAT_RGB565 },
	{ AV_PIX_FMT_BGR565,         SDL_PIXELFORMAT_BGR565 },
	{ AV_PIX_FMT_RGB24,          SDL_PIXELFORMAT_RGB24 },
	{ AV_PIX_FMT_BGR24,          SDL_PIXELFORMAT_BGR24 },
	{ AV_PIX_FMT_0RGB32,         SDL_PIXELFORMAT_RGB888 },
	{ AV_PIX_FMT_0BGR32,         SDL_PIXELFORMAT_BGR888 },
	{ AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888 },
	{ AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888 },
	{ AV_PIX_FMT_RGB32,          SDL_PIXELFORMAT_ARGB8888 },
	{ AV_PIX_FMT_RGB32_1,        SDL_PIXELFORMAT_RGBA8888 },
	{ AV_PIX_FMT_BGR32,          SDL_PIXELFORMAT_ABGR8888 },
	{ AV_PIX_FMT_BGR32_1,        SDL_PIXELFORMAT_BGRA8888 },
	{ AV_PIX_FMT_YUV420P,        SDL_PIXELFORMAT_IYUV },
	{ AV_PIX_FMT_YUYV422,        SDL_PIXELFORMAT_YUY2 },
	{ AV_PIX_FMT_UYVY422,        SDL_PIXELFORMAT_UYVY },
	{ AV_PIX_FMT_NONE,           SDL_PIXELFORMAT_UNKNOWN },
};

enum {
	AV_SYNC_AUDIO_MASTER, /* default choice */
	AV_SYNC_VIDEO_MASTER,
	AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

enum ErrorCode
{
	COMMON_FAIL = 0,

	WITHOUT_FILE_NAME = 1000,
	FILE_OPEN_FAIL,
	PACKET_ALLOC_FAIL,
	CONTEXT_ALLOC_FAIL,
	INPUT_OPEN_FAIL,
	STREAM_INFO_GET_FIAL,

	PICTURE_LIST_INIT_FAIL = 1010,
	SUBTITLE_LIST_INIT_FAIL,
	AUDIO_LIST_INIT_FAIL,
	PACKET_LIST_INIT_FAIL,

	SDL_CREATE_COND_FAIL = 1020,
	SDL_CREATE_MUTEX_FAIL,
	SDL_CREATE_THREAD_FAIL,

	AVFILTER_CREATE_FAIL = 1030,

	OPTIONAL_SET_FAIL = 1040,
	FILTERGRAPH_CONFIGURE_FAIL,
	FILTER_INPUT_CRATER_FAIL,
	FILTER_OUTPUT_CRATER_FAIL,
	FILTER_INOUT_CRATER_FAIL,
	FILTER_PARSE_FAIL,
	FILTER_LINK_FAIL,

	DICT_GET_FAIL = 1050,

	PARAMETERS_TO_CONTEXT_FAIL = 1060,
	DECODEC_BY_NAME_FAIL,
	DECODEC_OPEN_FAIL,
	CHANNAL_LAYOUT_COPY_FAIL,
	AUDIO_FILTER_CONFIGURE_FAIL,
	GET_CHANNAL_LAYOUT_FAIL,

	AUDIO_OPEN_FAIL,

	DECODER_INIT_FAIL,
	DECODER_START_FAIL,

	CREATE_GRAPH_FILTER_FAIL = 1070,

};

class PlayerException :public std::exception
{
public:
	PlayerException(const std::string& message,int errorCode)
		: message(message), errorCode(errorCode) {}

	const char* what() const noexcept override 
	{
		return message.c_str();
	}

	int getErrorCode() const 
	{
		return errorCode;
	}
private:
	std::string message;
	int errorCode;
};


//// 定义一个包含所有可能类型的 variant
//using ConfigVariant = std::variant<
//	bool, int, int64_t, float, double, char*,
//	SDL_Window*, SDL_Renderer*, SDL_RendererInfo*, SDL_AudioDeviceID
//>;
//
//class GlobalSingleton
//{
//public:
//	// 获取单例实例
//	static GlobalSingleton& getInstance() {
//		static GlobalSingleton instance;
//		return instance;
//	}
//
//	// 获取 map 中成员的函数
//	template<typename T>
//	T getConfigValue(const std::string& key) const {
//		std::shared_lock lock(rwlock); // 读锁
//		auto it = configMap.find(key);
//		if (it != configMap.end()) {
//			try {
//				return std::get<T>(*(it->second));
//			}
//			catch (const std::bad_variant_access&) {
//				std::cerr << "Type mismatch for key: " << key << std::endl;
//			}
//		}
//		else {
//			std::cerr << "Key not found: " << key << std::endl;
//		}
//		return T(); // 返回默认值
//	}
//
//	// 修改 map 中成员的函数
//	template<typename T>
//	void setConfigValue(const std::string& key, const T& value) {
//		std::unique_lock lock(rwlock); // 写锁
//		auto it = configMap.find(key);
//		if (it != configMap.end()) {
//			try {
//				*(it->second) = value;
//			}
//			catch (const std::bad_variant_access&) {
//				std::cerr << "Type mismatch for key: " << key << std::endl;
//			}
//		}
//		else {
//			configMap[key] = make_shared<ConfigVariant>(value);
//			std::cerr << "Key insert: " << key << std::endl;
//		}
//	}
//
//	void print() {
//		// 从 map 中读取并打印值
//		for (const auto& pair : configMap) {
//			std::cout << "Key: " << pair.first << ", Value: ";
//
//			// 使用 std::visit 来处理 variant 中的不同类型
//			std::visit([](const auto& value) {
//				std::cout << value;
//				}, *(pair.second));
//
//			std::cout << std::endl;
//		}
//	}
//
//private:
//
//
//private:
//	GlobalSingleton() {
//		configMap = {
//			{"Audio_disable", std::make_shared<ConfigVariant>(false)},
//			{"Video_disable", std::make_shared<ConfigVariant>(false)},
//			{"Subtitle_disable", std::make_shared<ConfigVariant>(false)},
//			{"Display_disable", std::make_shared<ConfigVariant>(false)},
//			{"startup_volume", std::make_shared<ConfigVariant>(100)},
//			{"genpts", std::make_shared<ConfigVariant>(0)},
//			{"seek_by_bytes", std::make_shared<ConfigVariant>(-1)},
//			{"start_time", std::make_shared<ConfigVariant>(AV_NOPTS_VALUE)},
//			{"infinite_buffer", std::make_shared<ConfigVariant>(-1)},
//			{"loop", std::make_shared<ConfigVariant>(1)},
//			{"autoexit", std::make_shared<ConfigVariant>(0)},
//			{"filter_nbthreads", std::make_shared<ConfigVariant>(0)},
//			{"audio_callback_time", std::make_shared<ConfigVariant>(0LL)},
//			{"window", std::make_shared<ConfigVariant>(nullptr)},
//			{"renderer", std::make_shared<ConfigVariant>(nullptr)},
//			{"renderer_info", std::make_shared<ConfigVariant>(nullptr)},
//			{"audio_dev", std::make_shared<ConfigVariant>(0)}
//		};
//	};
//
//	// 删除拷贝构造函数和赋值操作符，防止拷贝
//	GlobalSingleton(const GlobalSingleton&) = delete;
//	GlobalSingleton& operator=(const GlobalSingleton&) = delete;
//
//	mutable std::shared_mutex rwlock;
//	std::map<std::string, std::shared_ptr<ConfigVariant>> configMap;
//};


// 定义一个包含几种不同类型的 variant
using MyVariant = std::variant<bool, int, int64_t, double, std::string, SDL_Renderer*, SDL_RendererInfo*, SDL_Window*, SDL_AudioDeviceID>;
#undef main;

class GlobalSingleton
{
public:
	~GlobalSingleton() {}
	// 获取单例实例
	static GlobalSingleton* getInstance() {
		std::lock_guard<std::mutex> lock(mutex);
		if (!instance) {
			instance = std::unique_ptr<GlobalSingleton>(new GlobalSingleton());
		}
		return instance.get();
	}

	// 获取 map 中成员的函数
	template<typename T>
	T getConfigValue(const std::string& key) const {
		std::shared_lock lock(rwlock); // 读锁
		auto it = configMap.find(key);
		if (it != configMap.end()) {
			try {
				return std::get<T>(*(it->second));
			}
			catch (const std::bad_variant_access&) {
				std::cerr << "Type mismatch for key: " << key << std::endl;
			}
		}
		else {
			std::cerr << "Key not found: " << key << std::endl;
		}
		return T(); // 返回默认值
	}

	// 修改 map 中成员的函数
	template<typename T>
	void setConfigValue(const std::string& key, const T& value) {
		std::unique_lock lock(rwlock); // 写锁
		auto it = configMap.find(key);
		if (it != configMap.end()) {
			try {
				*(it->second) = value;
			}
			catch (const std::bad_variant_access&) {
				std::cerr << "Type mismatch for key: " << key << std::endl;
			}
		}
		else {
			configMap[key] = make_shared<MyVariant>(value);
			std::cerr << "Key insert: " << key << std::endl;
		}
	}

	void print() {
		// 从 map 中读取并打印值
		for (const auto& pair : configMap) {
			std::cout << "Key: " << pair.first << ", Value: ";

			// 使用 std::visit 来处理 variant 中的不同类型
			std::visit([](const auto& value) {
				std::cout << value;
				}, *(pair.second));

			std::cout << std::endl;
		}
	}

private:
	GlobalSingleton();

	GlobalSingleton(const GlobalSingleton&) = delete; // 禁用拷贝构造函数
	GlobalSingleton& operator=(const GlobalSingleton&) = delete; // 禁用赋值运算符
private:
	static std::unique_ptr<GlobalSingleton> instance;
	static std::mutex mutex;

	mutable std::shared_mutex rwlock;
	std::map<std::string, std::shared_ptr<MyVariant>> configMap;
};




#endif //__GLOBAL_STRUCT_H__
