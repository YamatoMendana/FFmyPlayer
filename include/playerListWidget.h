#ifndef	__PLAYER_LIST_WIDGET_H__
#define __PLAYER_LIST_WIDGET_H__

#include <QTabWidget>
#include <QTabBar>
#include <QPushButton>
#include <QListWidget>
#include <QFileInfo>
#include <set>



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

	void addItem(QListWidgetItem* item)
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
	void addItem(const QString& label)
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

	void removeItemWidget(QListWidgetItem* item){
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

signals:
	void sigOpenfile(QString path);
private:
	PlayListBar* pTabBar = nullptr;
	QListWidget* pListPtr = nullptr;
};



#endif //__PLAYER_LIST_WIDGET_H__