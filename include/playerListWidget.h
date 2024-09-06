#ifndef	__PLAYER_LIST_WIDGET_H__
#define __PLAYER_LIST_WIDGET_H__

#include <QTabWidget>
#include <QPushButton>
#include <QListWidget>
#include <QFileInfo>

#include "playerListBar.h"
#include "playerList.h"
#include "playerListCtlButtons.h"

#include "Common.h"


class playerListWidget :public QTabWidget
{
	Q_OBJECT
public:
	explicit playerListWidget(QWidget* parent = nullptr);
	~playerListWidget();

	void init();
	void setCurrentRow(int index);

	void moveTop();
	void moveBottom();
	void moveUp();
	void moveDown();

	// 正序排序
	void setAscendingOrder();
	// 反序排序
	void setDescendingOrder();
	//其他排序
	void setSortbyType(int type);

public:
	QStringList strplaylist;
signals:
	void sigOpenfile(QString path);

private:
	PlayListBar* pTabBar = nullptr;
	PlayList* pListPtr = nullptr;
	PlayerListCtlButtons* pCtlBtnPtr = nullptr;
};



#endif //__PLAYER_LIST_WIDGET_H__