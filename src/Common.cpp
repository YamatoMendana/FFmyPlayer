#include "Common.h"

GlobalSingleton::GlobalSingleton()
{

}

void GlobalSingleton::print()
{
	// �� map �ж�ȡ����ӡֵ
	for (const auto& pair : configMap) {
		std::cout << "Key: " << pair.first << ", Value: ";

		// ʹ�� std::visit ������ variant �еĲ�ͬ����
		std::visit([](const auto& value) {
			std::cout << value;
			}, *(pair.second));

		std::cout << std::endl;
	}
}

int GlobalSingleton::init()
{
	configMap["Audio_disable"] = std::make_shared<MyVariant>(false);
	configMap["Video_disable"] = std::make_shared<MyVariant>(false);
	configMap["Subtitle_disable"] = std::make_shared<MyVariant>(false);
	configMap["Display_disable"] = std::make_shared<MyVariant>(false);
	configMap["startup_volume"] = std::make_shared<MyVariant>(100);
	configMap["genpts"] = std::make_shared<MyVariant>(0);
	configMap["seek_by_bytes"] = std::make_shared<MyVariant>(-1);
	configMap["start_time"] = std::make_shared<MyVariant>(AV_NOPTS_VALUE);
	configMap["infinite_buffer"] = std::make_shared<MyVariant>(-1);
	configMap["loop"] = std::make_shared<MyVariant>(1);
	configMap["autoexit"] = std::make_shared<MyVariant>(0);
	configMap["filter_nbthreads"] = std::make_shared<MyVariant>(0);
	configMap["audio_callback_time"] = std::make_shared<MyVariant>(0LL);


	return 0;
}


// ��̬��Ա��������
std::unique_ptr<GlobalSingleton> GlobalSingleton::instance = nullptr;
std::mutex GlobalSingleton::mutex;

