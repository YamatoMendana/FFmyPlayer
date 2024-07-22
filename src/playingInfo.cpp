#include "playingInfo.h"

#include <QHBoxLayout>
#include "StyleSheet.h"

PlayingInfo::PlayingInfo(QWidget* parent /*= nullptr*/)
{
	this->setObjectName("PlayerCtlWidget");
	this->setStyleSheet(PlayerCtlWidget_SS);
	this->setFixedHeight(42);
	this->setAttribute(Qt::WA_StyledBackground, true);

	pPlayTotalTimeLabel = new QLineEdit("00:00:00");
	pSplitterLabel = new QLabel("/");
	pPlayingTimeLabel= new QLineEdit("00:00:00");

	QHBoxLayout* pHLayput = new QHBoxLayout();
	pHLayput->addWidget(pPlayingTimeLabel);
	pHLayput->addWidget(pSplitterLabel);
	pHLayput->addWidget(pPlayTotalTimeLabel);
	pHLayput->setContentsMargins(5, 5, 5, 5);
	pHLayput->setSpacing(0);
	this->setLayout(pHLayput);

	//设置唯一标识符
	this->setObjectName("PlayingInfoWidget");
	pPlayTotalTimeLabel->setObjectName("PlayTotalTime");
	pPlayingTimeLabel->setObjectName("PlayingTime");
	pSplitterLabel->setObjectName("SplitMark");

	pPlayTotalTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pPlayingTimeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pSplitterLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	pPlayTotalTimeLabel->setReadOnly(true);

	this->setStyleSheet(PlayingInfoWidget_SS);
	pPlayTotalTimeLabel->setStyleSheet(PlayingInfoEdit_SS);
	pPlayingTimeLabel->setStyleSheet(PlayingInfoEdit_SS);
	pSplitterLabel->setStyleSheet(PlayingInfoLabel_SS);

	//设置按钮大小
	pPlayTotalTimeLabel->setFixedSize(60, 40);
	pPlayingTimeLabel->setFixedSize(60, 40);
	pSplitterLabel->setFixedSize(10, 40);


}

PlayingInfo::~PlayingInfo()
{

}
