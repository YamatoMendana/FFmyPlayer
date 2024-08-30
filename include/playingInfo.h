#ifndef __PLAYINGINFO_H__
#define __PLAYINGINFO_H__

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

class PlayingInfo :public QWidget
{
    Q_OBJECT
public:
	explicit PlayingInfo(QWidget* parent = nullptr);
	~PlayingInfo();

	void setCurrentSeconds(double seconds);
	void setTotalSeconds(double seconds);

private:
	QString convertSecondsToHMS(double seconds);

private:
	QLineEdit* pPlayingTimeLabel;
	QLineEdit* pPlayTotalTimeLabel;
	QLabel* pSplitterLabel;
	
	QString strCurrentSeconds;
};









#endif	//__PLAYINGINFO_H__


