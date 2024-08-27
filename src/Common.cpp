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
	return 0;
}


// ��̬��Ա��������
std::unique_ptr<GlobalSingleton> GlobalSingleton::instance = nullptr;
std::mutex GlobalSingleton::mutex;

