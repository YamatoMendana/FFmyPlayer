#include "playerCtlWidget.h"

#include <QHBoxLayout>

#include "StyleSheet.h"

PlayerCtlWidget::PlayerCtlWidget(QWidget* parent /*= nullptr*/)
{
	this->setObjectName("PlayerCtlWidget");
	this->setStyleSheet(PlayerCtlWidget_SS);
	this->setFixedHeight(42);
	this->setAttribute(Qt::WA_StyledBackground, true);

	setMouseTracking(true);

	pCtlBts = new PlayerCtlButtons();
	pInfo = new PlayingInfo();

	QHBoxLayout* pHLayout = new QHBoxLayout();
	pHLayout->addWidget(pCtlBts);
	pHLayout->addWidget(pInfo);
	pHLayout->addStretch(1);
	pHLayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(pHLayout);
	

}

PlayerCtlWidget::~PlayerCtlWidget()
{

}
