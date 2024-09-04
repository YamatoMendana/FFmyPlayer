#ifndef __PLAYER_CTL_SLIDER_H__
#define __PLAYER_CTL_SLIDER_H__

#include <QObject>
#include <QWidget>
#include <QSlider>
#include <QPushButton>
#include <QEvent>


class PlayerCtlSlider : public QWidget
{
	Q_OBJECT
public:
	explicit PlayerCtlSlider(QWidget* parent = nullptr);

	void muteBtClick();
	void playSliderValChange(int val);
	void volumeSliderValChange(int val);

signals:
	void sigMute(bool status);
	void sigPlaybackProgressChange(int val);
	void sigVolumeValChange(int val);
protected:
	bool eventFilter(QObject* obj, QEvent* event);
private:
	QSlider* playSliderPtr;
	QSlider* volumeSliderPtr;

	QPushButton* muteBtPtr;

	int m_startVolVal = 100;
	int m_startPlayVal = 0;
};






#endif	//__PLAYER_CTL_SLIDER_H__
