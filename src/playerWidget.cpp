#include "playerWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

#include "StyleSheet.h"

PlayerWidget::PlayerWidget(QWidget* parent /*= nullptr*/)
{
	this->setObjectName("PlayerWidget");
	this->setStyleSheet(PlayerWidget_SS);
	this->resize(640 , 480);

	setMouseTracking(true);

}

PlayerWidget::~PlayerWidget()
{

}

bool PlayerWidget::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Enter: 
	{
		qDebug() << "Enter PlayerWidget";
	}
	case QEvent::Leave:
	{
		qDebug() << "Leave PlayerWidget";
	}
	default:
		break;
	}

	return QWidget::eventFilter(obj, event);
}
