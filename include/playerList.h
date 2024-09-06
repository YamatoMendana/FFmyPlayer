#ifndef __PLAYER_LIST_H__
#define __PLAYER_LIST_H__

#include <QListWidget>
#include <QListWidgetItem>
#include <set>
#include <map>
#include <functional>

//表示处理消息的事件回调方法类型
using sorthandler = std::function<bool(QListWidgetItem*, QListWidgetItem*)>;


class PlayList : public QListWidget
{
	Q_OBJECT
public:
	enum sortEnum
	{
		Title = 0,
		Extension,
		Size,
		Date,
		Duration,
		SortTypeCount,
	};
public:
	explicit PlayList(QWidget* parent = nullptr);

	void addItem(QListWidgetItem* item);

	void addItem(const QString& label);

	void sortListWidget(sortEnum type);

	void removeItemWidget(QListWidgetItem* item);

	void moveUp();

	void moveDown();

	void moveTop();

	void moveBottom();

signals:
	void sigItemRemoved(QListWidgetItem* item);

private:
	// 按标题排序
	bool sortByTitle(QListWidgetItem* a, QListWidgetItem* b);
	// 按拓展名排序
	bool sortByExtension(QListWidgetItem* a, QListWidgetItem* b);
	// 按大小排序
	bool sortBySize(QListWidgetItem* a, QListWidgetItem* b);
	// 按日期排序
	bool sortByDate(QListWidgetItem* a, QListWidgetItem* b);
	// 按时长排序（假设时长信息存储在QListWidgetItem的data中）
	bool sortByDuration(QListWidgetItem* a, QListWidgetItem* b);

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
	std::map<sortEnum, sorthandler> sortFunctionsMap;

};



#endif	//__PLAYER_LIST_H__
