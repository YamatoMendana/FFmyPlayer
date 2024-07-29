#include "playerWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QPainter>

#include "StyleSheet.h"

PlayerWidget::PlayerWidget(QWidget* parent /*= nullptr*/)
{
	this->setObjectName("PlayerWidget");
	this->setStyleSheet(PlayerWidget_SS);
	this->resize(640 , 480);

	m_image = QImage("ui/FFmyPlayer.png");

	setMouseTracking(true);

}

PlayerWidget::~PlayerWidget()
{

}

void PlayerWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	int width = this->width();
	int height = this->height();

	painter.setBrush(Qt::black);
	painter.drawRect(0, 0, width, height);

	if (isKeepAspectRatio)
	{
		QImage img = m_image.scaled(QSize(width, height), Qt::KeepAspectRatioByExpanding);
		int x = (this->width() - img.width()) / 2;
		int y = (this->height() - img.height()) / 2;

		painter.drawImage(QPoint(x, y), img);
	}
	else
	{
		QImage img = m_image.scaled(QSize(width, height));
		painter.drawImage(QPoint(0, 0), img);
	}

	
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
