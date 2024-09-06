#include "playerListCtlButtons.h"

#include <QHBoxLayout>

#include "StyleSheet.h"

PlayerListCtlButtons::PlayerListCtlButtons(QWidget* parent /*= nullptr*/)
{
	init();

	connect(pTopBtPtr, &QPushButton::clicked, this, &PlayerListCtlButtons::moveTopClicked);
	connect(pBottomBtPtr, &QPushButton::clicked, this, &PlayerListCtlButtons::moveBottomClicked);
	connect(pUpBtPtr, &QPushButton::clicked, this, &PlayerListCtlButtons::moveUpClicked);
	connect(pDownBtPtr, &QPushButton::clicked, this, &PlayerListCtlButtons::moveDownClicked);

}

void PlayerListCtlButtons::init()
{
	pAddBtPtr = new QPushButton();
	pRemoveBtPtr = new QPushButton();
	pSortBtPtr = new QPushButton();
	pTopBtPtr = new QPushButton();
	pBottomBtPtr = new QPushButton();
	pUpBtPtr = new QPushButton();
	pDownBtPtr = new QPushButton();


	pAddBtPtr->setObjectName("AddItemButton");
	QString strAdd = tr("添加");
	pAddBtPtr->setText(strAdd);
	pAddBtPtr->setToolTip(strAdd);
	pAddBtPtr->setFixedSize(35, 20);
	pAddBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pAddBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pAddBtPtr->installEventFilter(this);


	pRemoveBtPtr->setObjectName("RemoveItemButton");
	QString strRemove = tr("删除");
	pRemoveBtPtr->setText(strRemove);
	pRemoveBtPtr->setToolTip(strRemove);
	pRemoveBtPtr->setFixedSize(35, 20);
	pRemoveBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pRemoveBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pRemoveBtPtr->installEventFilter(this);


	pSortBtPtr->setObjectName("SortItemButton");
	QString strSort = tr("排序");
	pSortBtPtr->setText(strSort);
	pSortBtPtr->setToolTip(strSort);
	pSortBtPtr->setFixedSize(35, 20);
	pSortBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pSortBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pSortBtPtr->installEventFilter(this);


	pTopBtPtr->setObjectName("TopItemButton");
	QString strTop = tr("置顶");
	pTopBtPtr->setToolTip(strTop);
	pTopBtPtr->setFixedSize(20, 20);
	pTopBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pTopBtPtr->setIcon(QIcon("ui/top.png"));
	pTopBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pTopBtPtr->installEventFilter(this);


	pBottomBtPtr->setObjectName("BottomItemButton");
	QString strBottom = tr("置底");
	pBottomBtPtr->setToolTip(strBottom);
	pBottomBtPtr->setFixedSize(20, 20);
	pBottomBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pBottomBtPtr->setIcon(QIcon("ui/bottom.png"));
	pBottomBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pBottomBtPtr->installEventFilter(this);


	pUpBtPtr->setObjectName("UpItemButton");
	QString strUp = tr("上移");
	pUpBtPtr->setToolTip(strUp);
	pUpBtPtr->setFixedSize(20, 20);
	pUpBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pUpBtPtr->setIcon(QIcon("ui/moveUp.png"));
	pUpBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pUpBtPtr->installEventFilter(this);

	pDownBtPtr->setObjectName("DownItemButton");
	QString strDown = tr("下移");
	pDownBtPtr->setToolTip(strDown);
	pDownBtPtr->setFixedSize(20, 20);
	pDownBtPtr->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pDownBtPtr->setIcon(QIcon("ui/moveDown.png"));
	pDownBtPtr->setStyleSheet(PlayerCtlBt_SS);
	pDownBtPtr->installEventFilter(this);


	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(pTopBtPtr);
	layout->addWidget(pUpBtPtr);
	layout->addWidget(pDownBtPtr);
	layout->addWidget(pBottomBtPtr);
	layout->addWidget(pAddBtPtr);
	layout->addWidget(pRemoveBtPtr);
	layout->addWidget(pSortBtPtr);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(1);
	this->setLayout(layout);

}

void PlayerListCtlButtons::moveUpClicked()
{
	emit sigMoveUp();
}

void PlayerListCtlButtons::moveDownClicked()
{
	emit sigMoveDown();
}

void PlayerListCtlButtons::moveTopClicked()
{
	emit sigMoveTop();
}

void PlayerListCtlButtons::moveBottomClicked()
{
	emit sigMoveBottom();
}


