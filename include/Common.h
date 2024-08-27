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

using namespace std;

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
	DECODER_DECODE_FAIL,

	CREATE_GRAPH_FILTER_FAIL = 1070,

	ADD_FRAME_FAIL,
	FRAME_ALLOC_FAIL,
	VIDEO_FRAME_GET_FAIL,
	VIDEO_FILTER_CONFIGURE_FAIL,
	FRAMELIST_PEEK_FAIL,

	GRAPH_ALLOC_FAIL,

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

// 定义一个包含几种不同类型的 variant
using MyVariant = std::variant<bool, int, int64_t, uint32_t, double
	, char*, std::string
>;

class GlobalSingleton
{
public:
	~GlobalSingleton() {}
	// 获取单例实例
	static GlobalSingleton* getInstance() {
		std::lock_guard<std::mutex> lock(mutex);
		if (!instance) {
			instance = std::unique_ptr<GlobalSingleton>(new GlobalSingleton());
			instance->init();
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

	void print();

	int init();

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
