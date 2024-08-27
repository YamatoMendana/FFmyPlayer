#pragma once
#ifndef __PLAYER_WIDGET_H__
#define __PLAYER_WIDGET_H__

#include <QWidget>


class PlayerWidget :public QWidget
{
    //Q_OBJECT
public:
	explicit PlayerWidget(QWidget* parent = nullptr);
	~PlayerWidget();

protected:
	void paintEvent(QPaintEvent* event);
	bool eventFilter(QObject* obj, QEvent* event);

private:
	bool isKeepAspectRatio = true;

	QImage m_image;
};


#endif
