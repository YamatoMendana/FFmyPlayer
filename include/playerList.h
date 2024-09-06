#ifndef __PLAYER_LIST_H__
#define __PLAYER_LIST_H__

#include <QListWidget>
#include <QListWidgetItem>
#include <set>
#include <map>
#include <functional>

//��ʾ������Ϣ���¼��ص���������
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
	// ����������
	bool sortByTitle(QListWidgetItem* a, QListWidgetItem* b);
	// ����չ������
	bool sortByExtension(QListWidgetItem* a, QListWidgetItem* b);
	// ����С����
	bool sortBySize(QListWidgetItem* a, QListWidgetItem* b);
	// ����������
	bool sortByDate(QListWidgetItem* a, QListWidgetItem* b);
	// ��ʱ�����򣨼���ʱ����Ϣ�洢��QListWidgetItem��data�У�
	bool sortByDuration(QListWidgetItem* a, QListWidgetItem* b);

private:
	bool addUniqueItem(const QString& itemText) {
		// ����Ƿ��Ѿ�������ͬ���Ƶ���
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
