#include "playerListWidget.h"
#include <QHBoxLayout>

#include "StyleSheet.h"



playerListWidget::playerListWidget(QWidget* parent /*= nullptr*/) :QTabWidget(parent)
{
	init();

	connect(pListPtr, &PlayList::itemDoubleClicked, this, [&](QListWidgetItem* item) {
		QString strPath = item->data(Qt::UserRole).toString();
		emit sigOpenfile(strPath);
		});

	connect(pListPtr, &PlayList::sigItemRemoved, this, [&](QListWidgetItem* item) {
		QString path = item->data(Qt::UserRole).toString();
		int ret = strplaylist.indexOf(path);
		if (ret != -1)
			strplaylist.removeAt(ret);
		});

	connect(pCtlBtnPtr, &PlayerListCtlButtons::sigMoveTop, this, &playerListWidget::moveTop);
	connect(pCtlBtnPtr, &PlayerListCtlButtons::sigMoveBottom, this, &playerListWidget::moveBottom);
	connect(pCtlBtnPtr, &PlayerListCtlButtons::sigMoveUp, this, &playerListWidget::moveUp);
	connect(pCtlBtnPtr, &PlayerListCtlButtons::sigMoveDown, this, &playerListWidget::moveDown);

}



playerListWidget::~playerListWidget()
{
	int size = pListPtr->count();
	QStringList strplaylist;
	for (int i = 0; i < size; i++)
	{
		QListWidgetItem* item = pListPtr->item(i);
		QString strPath = item->data(Qt::UserRole).toString();
		strplaylist.append(strPath);
	}
	GlobalSingleton::getInstance()->savePlayList(strplaylist);
}

void playerListWidget::init()
{
	this->setObjectName("PlayListTabWidget");
	this->setStyleSheet(PlayList_TabWidget_SS);
	this->setAttribute(Qt::WA_StyledBackground, true);
	this->setFixedWidth(320);

	pTabBar = new PlayListBar();
	pTabBar->setObjectName("PlayListTabBar");
	this->setTabBar(pTabBar);

	pListPtr = new PlayList();
	pListPtr->setObjectName("DefaultAlbumList");
	pListPtr->setStyleSheet(PlayList_ListWidget_SS);

	pCtlBtnPtr = new PlayerListCtlButtons();
	pCtlBtnPtr->setObjectName("PlayListCtlButtoms");

	QWidget* tabContent = new QWidget();
	tabContent->setAttribute(Qt::WA_TranslucentBackground);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(pListPtr);
	layout->addWidget(pCtlBtnPtr);
	layout->setContentsMargins(0, 0, 0, 0);
	tabContent->setLayout(layout);

	addTab(tabContent, "默认专辑");

	GlobalSingleton::getInstance()->getPlaylist(strplaylist);
	for (QString filename : strplaylist)
	{
		pListPtr->addItem(filename);
	}
}

void playerListWidget::setCurrentRow(int index)
{
	int size = pListPtr->count();
	if (index < size && index>0)
	{
		pListPtr->setCurrentRow(index);
	}
	
}



void playerListWidget::moveTop()
{
	pListPtr->moveTop();
}

void playerListWidget::moveBottom()
{
	pListPtr->moveBottom();
}

void playerListWidget::moveUp()
{
	pListPtr->moveUp();
}

void playerListWidget::moveDown()
{
	pListPtr->moveDown();
}

void playerListWidget::setAscendingOrder()
{
	pListPtr->sortItems(Qt::DescendingOrder);
}

void playerListWidget::setDescendingOrder()
{
	pListPtr->sortItems(Qt::AscendingOrder);
}

void playerListWidget::setSortbyType(int type)
{
	PlayList::sortEnum sort_type = static_cast<PlayList::sortEnum>(type);
	pListPtr->sortListWidget(sort_type);
}


