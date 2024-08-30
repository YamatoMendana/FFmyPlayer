#include "playerManager.h"

#include <QDebug>
#include <QObject>


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

    return 0;
}

void PlayerManager::play_stop()
{
    displayPtr->stop();

}

bool PlayerManager::playStatus_toggle()
{
    return displayPtr->toggle_pause();
}

void PlayerManager::seek_forward()
{
    displayPtr->seek_forward();
}

void PlayerManager::seek_back()
{
    displayPtr->seek_back();
}


