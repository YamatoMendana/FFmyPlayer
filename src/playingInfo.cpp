#include "playingInfo.h"

#include <QHBoxLayout>
#include <QTime>

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

void PlayingInfo::setCurrentSeconds(double seconds)
{
	QString str = convertSecondsToHMS(seconds);
	pPlayingTimeLabel->setText(str);
}

void PlayingInfo::setTotalSeconds(double seconds)
{
	QString str = convertSecondsToHMS(seconds);
	pPlayTotalTimeLabel->setText(str);
}

QString PlayingInfo::convertSecondsToHMS(double seconds)
{
	// 使用 QTime 类将秒数转换为时间格式
	QTime time = QTime::fromMSecsSinceStartOfDay((int)seconds * 1000);

	// 将时间格式化为 "00:00:00" 形式的 QString
	strCurrentSeconds = time.toString("hh:mm:ss");

	return strCurrentSeconds;
}
