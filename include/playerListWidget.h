#ifndef	__PLAYER_LIST_WIDGET_H__
#define __PLAYER_LIST_WIDGET_H__

#include <QTabWidget>
#include <QTabBar>
#include <QPushButton>
#include <QListWidget>
#include <QFileInfo>
#include <set>

#include "Common.h"



class PlayListBar :public QTabBar
{
	Q_OBJECT
public:
	explicit PlayListBar(QWidget* parent = nullptr);

protected:
	void resizeEvent(QResizeEvent* event) override;
	void paintEvent(QPaintEvent* event) override;

private:
	QPushButton* pAddTabPtr = nullptr;

};

class PlayList : public QListWidget
{
	Q_OBJECT
public:
	PlayList(QWidget* parent = nullptr) : QListWidget(parent) {}

	void addItem(QListWidgetItem* item);

	void addItem(const QString& label);

	void removeItemWidget(QListWidgetItem* item);

private:
	bool addUniqueItem(const QString& itemText) {
		// 检查是否已经存在相同名称的项
		QString path = itemText;
		auto it = itemSet.find(path);
		if (it != itemSet.end())
			return true;
		else
			return false;
	}
private:
	std::set<QString> itemSet;

};


class playerListWidget :public QTabWidget
{
	Q_OBJECT
public:
	explicit playerListWidget(QWidget* parent = nullptr);
	~playerListWidget();

	void init();
	void setCurrentRow(int index);

public:
	QStringList strplaylist;
signals:
	void sigOpenfile(QString path);

private:
	PlayListBar* pTabBar = nullptr;
	PlayList* pListPtr = nullptr;
};



#endif //__PLAYER_LIST_WIDGET_H__