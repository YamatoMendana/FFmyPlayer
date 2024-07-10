#include "playerWidget.h"

#include "StyleSheet.h"

PlayerWidget::PlayerWidget(QWidget* parent /*= nullptr*/)
{
	this->setObjectName("PlayerWidget");
	this->setStyleSheet(PlayerWidget_SS);


}

PlayerWidget::~PlayerWidget()
{

}
