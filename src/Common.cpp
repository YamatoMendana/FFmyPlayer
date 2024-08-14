#include "Common.h"

GlobalSingleton::GlobalSingleton()
{
	//configMap = {
	//{"Audio_disable", std::make_shared<MyVariant>(false)},
	//{"Video_disable", std::make_shared<MyVariant>(false)},
	//{"Subtitle_disable", std::make_shared<MyVariant>(false)},
	//{"Display_disable", std::make_shared<MyVariant>(false)},
	//{"startup_volume", std::make_shared<MyVariant>(100)},
	//{"genpts", std::make_shared<MyVariant>(0)},
	//{"seek_by_bytes", std::make_shared<MyVariant>(-1)},
	//{"start_time", std::make_shared<MyVariant>(AV_NOPTS_VALUE)},
	//{"infinite_buffer", std::make_shared<MyVariant>(-1)},
	//{"loop", std::make_shared<MyVariant>(1)},
	//{"autoexit", std::make_shared<MyVariant>(0)},
	//{"filter_nbthreads", std::make_shared<MyVariant>(0)},
	//{"audio_callback_time", std::make_shared<MyVariant>(0LL)},
	//{"window", std::make_shared<MyVariant>(nullptr)},
	//{"renderer", std::make_shared<MyVariant>(nullptr)},
	//{"renderer_info", std::make_shared<MyVariant>(nullptr)},
	//{"audio_dev", std::make_shared<MyVariant>(0)}
	//};
}

// 静态成员变量定义
std::unique_ptr<GlobalSingleton> GlobalSingleton::instance = nullptr;
std::mutex GlobalSingleton::mutex;

