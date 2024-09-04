#include "playerListWidget.h"
#include <QHBoxLayout>

#include "StyleSheet.h"

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

playerListWidget::playerListWidget(QWidget* parent /*= nullptr*/) :QTabWidget(parent)
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


	QWidget* tabContent = new QWidget();
	tabContent->setAttribute(Qt::WA_TranslucentBackground);
	QVBoxLayout* layout = new QVBoxLayout();
	layout->addWidget(pListPtr);
	layout->setContentsMargins(0, 0, 0, 0);
	tabContent->setLayout(layout);

	addTab(tabContent, "默认专辑");

	init();

	connect(pListPtr, &PlayList::itemDoubleClicked, this, [&](QListWidgetItem* item) {
		QString strPath = item->data(Qt::UserRole).toString();
		emit sigOpenfile(strPath);
		});

	connect(pListPtr, &PlayList::removeItemWidget, this, [&](QListWidgetItem* item) {
		QString path = item->data(Qt::UserRole).toString();
		int ret = strplaylist.indexOf(path);
		if (ret != -1)
			strplaylist.removeAt(ret);
		});
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

void PlayList::addItem(QListWidgetItem* item)
{
	QListWidgetItem* pItem = item;
	QString path = pItem->text();
	if (!addUniqueItem(path))
	{
		QFileInfo fileInfo(path);
		QString fileName = fileInfo.fileName();
		pItem->setText(fileName);
		pItem->setData(Qt::UserRole, QVariant(path));
		QListWidget::addItem(pItem);
		itemSet.insert(path);
	}
	else
		return;
}

void PlayList::addItem(const QString& label)
{

	QString path = label;
	if (!addUniqueItem(path))
	{
		QFileInfo fileInfo(path);
		QString fileName = fileInfo.fileName();
		QListWidgetItem* pItem = new QListWidgetItem(fileName);;
		pItem->setData(Qt::UserRole, QVariant(path));
		QListWidget::addItem(pItem);
		itemSet.insert(path);
	}
	else
		return;
}

void PlayList::removeItemWidget(QListWidgetItem* item)
{
	QListWidgetItem* pItem = item;
	QString path = pItem->text();
	if (addUniqueItem(path))
	{
		QListWidget::removeItemWidget(item);
		itemSet.erase(path);
	}
	else
		return;
}
