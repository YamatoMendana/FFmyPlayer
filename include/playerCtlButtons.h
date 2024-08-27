#ifndef __PLAYERCTLBUTTON_H__
#define __PLAYERCTLBUTTON_H__


#include <QPushButton>
#include <QWidget>
#pragma execution_character_set("utf-8")

class PlayerCtlButtons :public QWidget
{
    Q_OBJECT
public:
	explicit PlayerCtlButtons(QWidget* parent = nullptr);
	~PlayerCtlButtons();

protected:


signals:
	void sigPlayFile(QString str);
private slots:
	void Play_clicked();
	void Pause_clicked();
	void Stop_clicked();
	void Open_clicked();

private:
	//播放按钮
	QPushButton* pPlayButton = nullptr;
	//停止按钮
	QPushButton* pStopButton = nullptr;
	//上一集按钮
	QPushButton* pPrevEpisodeButton = nullptr;
	//下一集按钮
	QPushButton* pNextEpisodeButton = nullptr;
	//打开文件按钮
	QPushButton* pOpenFileButton = nullptr;

	QString strSupportVideoList;
	QString strSupportAudioList;
	QString strSupportListFile;
	QString strSupportAllList;
};

#endif //__PLAYERCTLBUTTON_H__
