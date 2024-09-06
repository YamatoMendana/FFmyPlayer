#include "playerList.h"
#include "QFileInfo"



PlayList::PlayList(QWidget* parent /*= nullptr*/) : QListWidget(parent)
{
	sortFunctionsMap.insert({ sortEnum::Title, std::bind(&PlayList::sortByTitle, this, std::placeholders::_1, std::placeholders::_2) });
	sortFunctionsMap.insert({ sortEnum::Extension, std::bind(&PlayList::sortByExtension, this, std::placeholders::_1, std::placeholders::_2) });
	sortFunctionsMap.insert({ sortEnum::Size, std::bind(&PlayList::sortBySize, this, std::placeholders::_1, std::placeholders::_2) });
	sortFunctionsMap.insert({ sortEnum::Date, std::bind(&PlayList::sortByDate, this, std::placeholders::_1, std::placeholders::_2) });
	sortFunctionsMap.insert({ sortEnum::Duration, std::bind(&PlayList::sortByDuration, this, std::placeholders::_1, std::placeholders::_2) });

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

void PlayList::sortListWidget(sortEnum type)
{
	QList<QListWidgetItem*> items = this->findItems(QString("*"), Qt::MatchWildcard);

	auto it = sortFunctionsMap.find(type);
	if (it != sortFunctionsMap.end())
	{
		sorthandler func = sortFunctionsMap[type];
		std::sort(items.begin(), items.end(), func);
		this->clear();
		for (QListWidgetItem* item : items) {
			this->addItem(item);
		}
	}
	
}

void PlayList::removeItemWidget(QListWidgetItem* item)
{
	QListWidgetItem* pItem = item;
	QString path = pItem->text();
	if (addUniqueItem(path))
	{
		QListWidget::removeItemWidget(item);
		itemSet.erase(path);
		emit sigItemRemoved(item);
	}
	else
		return;
}

void PlayList::moveUp()
{
	int row = this->currentRow();
	if (row > 0) {
		QListWidgetItem* item = this->takeItem(row);
		this->insertItem(row - 1, item);
		this->setCurrentRow(row - 1);
	}
}

void PlayList::moveDown()
{
	int row = this->currentRow();
	if (row < this->count() - 1) {
		QListWidgetItem* item = this->takeItem(row);
		this->insertItem(row + 1, item);
		this->setCurrentRow(row + 1);
	}
}

void PlayList::moveTop()
{
	int row = this->currentRow();
	if (row > 0) {
		QListWidgetItem* item = this->takeItem(row);
		this->insertItem(0, item);
		this->setCurrentRow(0);
	}
}

void PlayList::moveBottom()
{
	int row = this->currentRow();
	if (row < this->count() - 1) {
		QListWidgetItem* item = this->takeItem(row);
		this->insertItem(this->count(), item);
		this->setCurrentRow(this->count() - 1);
	}
}

bool PlayList::sortByTitle(QListWidgetItem* a, QListWidgetItem* b)
{
	return a->text() < b->text();
}

bool PlayList::sortByExtension(QListWidgetItem* a, QListWidgetItem* b)
{
	QString extA = QFileInfo(a->text()).suffix();
	QString extB = QFileInfo(b->text()).suffix();
	return extA < extB;
}

bool PlayList::sortBySize(QListWidgetItem* a, QListWidgetItem* b)
{
	QFileInfo infoA(a->text());
	QFileInfo infoB(b->text());
	return infoA.size() < infoB.size();
}

bool PlayList::sortByDate(QListWidgetItem* a, QListWidgetItem* b)
{
	QFileInfo infoA(a->text());
	QFileInfo infoB(b->text());
	return infoA.lastModified() < infoB.lastModified();
}

bool PlayList::sortByDuration(QListWidgetItem* a, QListWidgetItem* b)
{
	int durationA = a->data(Qt::UserRole).toInt();
	int durationB = b->data(Qt::UserRole).toInt();
	return durationA < durationB;
}
