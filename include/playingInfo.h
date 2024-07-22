#ifndef __PLAYINGINFO_H__
#define __PLAYINGINFO_H__

#include <QWidget>
#include <QLabel>
#include <QLineEdit>

class PlayingInfo :public QWidget
{
    //Q_OBJECT
public:
	explicit PlayingInfo(QWidget* parent = nullptr);
	~PlayingInfo();


private:
	QLineEdit* pPlayingTimeLabel;
	QLineEdit* pPlayTotalTimeLabel;
	QLabel* pSplitterLabel;
	
	
};









#endif	//__PLAYINGINFO_H__


