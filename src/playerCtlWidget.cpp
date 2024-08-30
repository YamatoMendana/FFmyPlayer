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

	pControlBtsPtr = new PlayerCtlButtons();
	pInfoPtr = new PlayingInfo();
	pPlayListBtPtr = new QPushButton();

	pPlayListBtPtr->setObjectName("OpenPlayList");
	pPlayListBtPtr->setToolTip(tr("打开菜单"));
	pPlayListBtPtr->setFixedSize(40, 40);
	pPlayListBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pPlayListBtPtr->setIcon(QIcon("ui/menu.png"));
	pPlayListBtPtr->installEventFilter(this);

	QHBoxLayout* pHLayout = new QHBoxLayout();
	pHLayout->addWidget(pControlBtsPtr);
	pHLayout->addWidget(pInfoPtr);
	pHLayout->addStretch(1);
	pHLayout->addWidget(pPlayListBtPtr);
	pHLayout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(pHLayout);
	

}

PlayerCtlWidget::~PlayerCtlWidget()
{

}

void PlayerCtlWidget::setPlayingSeconds(double seconds)
{
	pInfoPtr->setCurrentSeconds(seconds);
}

void PlayerCtlWidget::setPlayingTotalSeconds(double seconds)
{
	pInfoPtr->setTotalSeconds(seconds);
}

void PlayerCtlWidget::setPlayIcon(bool status)
{
	pControlBtsPtr->setPlayIcon(status);
}

bool PlayerCtlWidget::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Enter:
	{
		if (obj == pPlayListBtPtr)
		{
			pPlayListBtPtr->setIcon(QIcon("ui/menu_hover.png"));
		}
		break;
	}
	case QEvent::Leave:
	{
		if (obj == pPlayListBtPtr)
		{
			pPlayListBtPtr->setIcon(QIcon("ui/menu.png"));
		}
		break;
	}
	default:
		break;
	}
	return QWidget::eventFilter(obj, event);
}
