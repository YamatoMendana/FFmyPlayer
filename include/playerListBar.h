#ifndef __PLAYER_LIST_BAR_H__
#define __PLAYER_LIST_BAR_H__

#include <QTabBar>
#include <QEvent>
#include <QPushButton>

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


#endif	//__PLAYER_LIST_BAR_H__


