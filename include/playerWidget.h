#pragma once
#ifndef __PLAYER_WIDGET_H__
#define __PLAYER_WIDGET_H__

#include <QWidget>

class PlayerWidget :public QWidget
{
public:
	explicit PlayerWidget(QWidget* parent = nullptr);
	~PlayerWidget();

protected:
	bool eventFilter(QObject* obj, QEvent* event);
};


#endif