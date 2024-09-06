#include "playerListBar.h"

#include <QHBoxLayout>

PlayListBar::PlayListBar(QWidget* parent /*= nullptr*/) : QTabBar(parent)
{
	//pAddTabPtr = new QPushButton(this);
	//pAddTabPtr->setText("+ 增加专辑");
	//pAddTabPtr->setFixedSize(tabSizeHint(0).width(), tabSizeHint(0).height());

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addStretch();
	layout->addWidget(pAddTabPtr);
	this->setLayout(layout);
}

void PlayListBar::resizeEvent(QResizeEvent* event)
{
	QTabBar::resizeEvent(event);
	// 调整按钮的位置，使其位于标签栏的右侧
	//pAddTabPtr->move(width() - pAddTabPtr->width(), (height() - pAddTabPtr->height()));
}


void PlayListBar::paintEvent(QPaintEvent* event)
{
	QTabBar::paintEvent(event);
}

