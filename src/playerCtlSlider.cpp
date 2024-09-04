#include "playerCtlSlider.h"
#include "StyleSheet.h"

#include <QHBoxLayout>

PlayerCtlSlider::PlayerCtlSlider(QWidget* parent /*= nullptr*/)
{
	this->setAttribute(Qt::WA_StyledBackground, true);

	this->setFixedHeight(20);

	playSliderPtr = new QSlider(Qt::Horizontal,this);
	volumeSliderPtr = new QSlider(Qt::Horizontal,this);
	muteBtPtr = new QPushButton();

	playSliderPtr->setObjectName("PlaySlider");
	playSliderPtr->setValue(m_startPlayVal);
	playSliderPtr->setStyleSheet(PlayCtlSlider_SS);

	volumeSliderPtr->setObjectName("VolumeSlider");
	volumeSliderPtr->setFixedWidth(80);
	volumeSliderPtr->setValue(m_startVolVal);
	// 设置滑块的最小值和最大值
	volumeSliderPtr->setMinimum(0);
	volumeSliderPtr->setMaximum(100);
	volumeSliderPtr->setStyleSheet(PlayCtlSlider_SS);

	muteBtPtr->setObjectName("MuteButton");
	muteBtPtr->setFixedSize(20, 20);
	muteBtPtr->setProperty("status", true);
	muteBtPtr->setIcon(QIcon("ui/sound_on.png"));
	muteBtPtr->setStyleSheet(Button_normal_SS);
	muteBtPtr->installEventFilter(this);

	QHBoxLayout* pHVolLayout = new QHBoxLayout();
	pHVolLayout->addWidget(muteBtPtr);
	pHVolLayout->addWidget(volumeSliderPtr);
	pHVolLayout->setContentsMargins(0, 0, 0, 0);
	pHVolLayout->setSpacing(0);

	QHBoxLayout* pHLayout = new QHBoxLayout();
	pHLayout->addWidget(playSliderPtr);
	pHLayout->addLayout(pHVolLayout);
	pHLayout->setContentsMargins(0, 0, 0, 0);
	pHLayout->setSpacing(5);

	this->setLayout(pHLayout);

	connect(muteBtPtr, &QPushButton::clicked, this, &PlayerCtlSlider::muteBtClick);
	connect(playSliderPtr, &QSlider::valueChanged, this, &PlayerCtlSlider::playSliderValChange);
	connect(volumeSliderPtr, &QSlider::valueChanged, this, &PlayerCtlSlider::volumeSliderValChange);


}


void PlayerCtlSlider::muteBtClick()
{
	bool status = muteBtPtr->property("status").toBool();
	muteBtPtr->setProperty("status", !status);

	emit sigMute(!status);
}

void PlayerCtlSlider::playSliderValChange(int val)
{
	emit sigPlaybackProgressChange(val);
}

void PlayerCtlSlider::volumeSliderValChange(int val)
{
	emit sigVolumeValChange(val);
}

bool PlayerCtlSlider::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Enter:
	{
		if (obj == muteBtPtr)
		{
			if (muteBtPtr->property("status") == true)
			{
				muteBtPtr->setIcon(QIcon("ui/sound_on_hover.png"));
			}
			else
			{
				muteBtPtr->setIcon(QIcon("ui/sound_off_hover.png"));
			}
		}
		break;
	}
	case QEvent::Leave:
	{
		if (obj == muteBtPtr)
		{
			if (muteBtPtr->property("status") == true)
			{
				muteBtPtr->setIcon(QIcon("ui/sound_on.png"));
			}
			else
			{
				muteBtPtr->setIcon(QIcon("ui/sound_off.png"));
			}
		}
		break;
	}
	default:
		break;
	}

	return QWidget::eventFilter(obj, event);
}




