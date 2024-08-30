#ifndef __PLAYERCTLWIDGET_H__
#define __PLAYERCTLWIDGET_H__

#include <QWidget>
#include <QEvent>
#include <shared_mutex>
#include <memory.h>


#include "playerCtlButtons.h"
#include "playingInfo.h"

using namespace std;

class PlayerCtlWidget:public QWidget
{
    Q_OBJECT
public:
    explicit PlayerCtlWidget(QWidget* parent = nullptr);
    ~PlayerCtlWidget();

    void setPlayingSeconds(double seconds);
    void setPlayingTotalSeconds(double seconds);
    void setPlayIcon(bool status);

	inline PlayerCtlButtons* getPlayerCtlButtonsPtr() const {
		return pControlBtsPtr;
	}

	inline PlayingInfo* getPlayingInfoPtr() const {
		return pInfoPtr;
	}

protected:
    bool eventFilter(QObject* obj, QEvent* event);

private:
    PlayerCtlButtons* pControlBtsPtr;
    PlayingInfo* pInfoPtr;
    QPushButton* pPlayListBtPtr;

};

#endif // __PLAYERCTLWIDGET_H__
