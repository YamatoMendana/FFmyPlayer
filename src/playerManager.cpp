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

int PlayerManager::play(QStringList* playlist,QString filename ,WId widId)
{
    m_strPlayList = playlist;
    if (m_strPlayList->contains(filename))
    {
        m_strPlayListIndex = m_strPlayList->indexOf(filename);
    }
    else
    {
        m_strPlayListIndex = -1;
    }

    displayPtr->startPlay(filename, widId);
    m_wid = widId;
    return m_strPlayListIndex;
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


