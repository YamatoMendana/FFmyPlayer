#ifndef __PLAYERCTLWIDGET_H__
#define __PLAYERCTLWIDGET_H__

#include <QWidget>

#include "playerCtlButtons.h"
#include "playingInfo.h"

class PlayerCtlWidget:public QWidget
{
    //Q_OBJECT
public:
    explicit PlayerCtlWidget(QWidget* parent = nullptr);
    ~PlayerCtlWidget();

private:
    PlayerCtlButtons* pCtlBts = nullptr;
    PlayingInfo* pInfo = nullptr;

};

#endif // __PLAYERCTLWIDGET_H__
