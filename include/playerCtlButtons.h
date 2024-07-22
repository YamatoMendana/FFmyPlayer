#ifndef __PLAYERCTLBUTTON_H__
#define __PLAYERCTLBUTTON_H__


#include <QPushButton>
#include <QWidget>
#pragma execution_character_set("utf-8")

class PlayerCtlButtons :public QWidget
{
    //Q_OBJECT
public:
	explicit PlayerCtlButtons(QWidget* parent = nullptr);
	~PlayerCtlButtons();

protected:

private slots:
	void Play_clicked();
	void Pause_clicked();
	void Stop_clicked();
	void Open_clicked();

private:
	//���Ű�ť
	QPushButton* pPlayButton = nullptr;
	//ֹͣ��ť
	QPushButton* pStopButton = nullptr;
	//��һ����ť
	QPushButton* pPrevEpisodeButton = nullptr;
	//��һ����ť
	QPushButton* pNextEpisodeButton = nullptr;
	//���ļ���ť
	QPushButton* pOpenFileButton = nullptr;

	QString strSupportVideoList;
	QString strSupportAudioList;
	QString strSupportListFile;
	QString strSupportAllList;
};

#endif //__PLAYERCTLBUTTON_H__
