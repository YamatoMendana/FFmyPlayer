#ifndef __PLAYER_LIST_CTL_BTS_H__
#define __PLAYER_LIST_CTL_BTS_H__

#include <QWidget>
#include <QPushButton>
#include <QEvent>


class PlayerListCtlButtons :public QWidget
{
	Q_OBJECT
public:
	explicit PlayerListCtlButtons(QWidget* parent = nullptr);

	void init();

	void moveUpClicked();
	void moveDownClicked();
	void moveTopClicked();
	void moveBottomClicked();
signals:
	void sigMoveUp();
	void sigMoveDown();
	void sigMoveTop();
	void sigMoveBottom();
private:
	QPushButton* pAddBtPtr = nullptr;
	QPushButton* pRemoveBtPtr = nullptr;
	QPushButton* pSortBtPtr = nullptr;
	QPushButton* pTopBtPtr = nullptr;
	QPushButton* pBottomBtPtr = nullptr;
	QPushButton* pUpBtPtr= nullptr;
	QPushButton* pDownBtPtr = nullptr;


};





#endif	//__PLAYER_LIST_CTL_BTS_H__
