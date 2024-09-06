#include "playerListBar.h"

#include <QHBoxLayout>

PlayListBar::PlayListBar(QWidget* parent /*= nullptr*/) : QTabBar(parent)
{
	//pAddTabPtr = new QPushButton(this);
	//pAddTabPtr->setText("+ ����ר��");
	//pAddTabPtr->setFixedSize(tabSizeHint(0).width(), tabSizeHint(0).height());

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->addStretch();
	layout->addWidget(pAddTabPtr);
	this->setLayout(layout);
}

void PlayListBar::resizeEvent(QResizeEvent* event)
{
	QTabBar::resizeEvent(event);
	// ������ť��λ�ã�ʹ��λ�ڱ�ǩ�����Ҳ�
	//pAddTabPtr->move(width() - pAddTabPtr->width(), (height() - pAddTabPtr->height()));
}


void PlayListBar::paintEvent(QPaintEvent* event)
{
	QTabBar::paintEvent(event);
}

