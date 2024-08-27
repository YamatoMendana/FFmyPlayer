#include "Common.h"

GlobalSingleton::GlobalSingleton()
{

}

void GlobalSingleton::print()
{
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

int GlobalSingleton::init()
{
	return 0;
}


// 静态成员变量定义
std::unique_ptr<GlobalSingleton> GlobalSingleton::instance = nullptr;
std::mutex GlobalSingleton::mutex;

