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

};

#endif //__PLAYERCTLBUTTON_H__
