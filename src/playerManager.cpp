#include "playerManager.h"

#include <QDebug>
#include <QObject>

#include "videoctl.h"


PlayerManager::PlayerManager()
{
    displayPtr = make_shared<PlayerDisplay>();
}

PlayerManager::~PlayerManager()
{

}

int PlayerManager::open_file(QString filename, WId widId)
{
    displayPtr->startPlay(filename, widId);
    //VideoCtl::GetInstance()->StartPlay(filename, widId);
    //PlayerDisplay::GetInstance()->startPlay(filename, widId);

    return 0;
}





