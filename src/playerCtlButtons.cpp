#include "playerCtlButtons.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QFileDialog>

#include "StyleSheet.h"

PlayerCtlButtons::PlayerCtlButtons(QWidget* parent /*= nullptr*/)
{
	this->resize(206, 42);
	this->setAttribute(Qt::WA_StyledBackground, true);

	setMouseTracking(true);

	pPlayButton = new QPushButton(this);
	pStopButton = new QPushButton(this);
	pPrevEpisodeButton = new QPushButton(this);
	pNextEpisodeButton = new QPushButton(this);
	pOpenFileButton = new QPushButton(this);

	QHBoxLayout* pHlayout = new QHBoxLayout();
	pHlayout->addWidget(pPlayButton);
	pHlayout->addWidget(pStopButton);
	pHlayout->addWidget(pPrevEpisodeButton);
	pHlayout->addWidget(pNextEpisodeButton);
	pHlayout->addWidget(pOpenFileButton);
	pHlayout->setContentsMargins(1, 1, 1, 1);
	pHlayout->setSpacing(0);
	this->setLayout(pHlayout);

	//设置唯一标识符
	this->setObjectName("PlayerCtlBtWidget");
	pPlayButton->setObjectName("PlayButton");
	pStopButton->setObjectName("StopButton");
	pPrevEpisodeButton->setObjectName("PrevEpisodeButton");
	pNextEpisodeButton->setObjectName("NextEpisodeButton");
	pOpenFileButton->setObjectName("OpenFileButton");

	pPlayButton->setToolTip(tr("播放"));
	pPlayButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pPlayButton->setIcon(QIcon("ui/play.png"));
	
	pStopButton->setToolTip(tr("停止"));
	pStopButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pStopButton->setIcon(QIcon("ui/stop.png"));

	pPrevEpisodeButton->setToolTip(tr("上一集"));
	pPrevEpisodeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pPrevEpisodeButton->setIcon(QIcon("ui/Previous_episode.png"));

	pNextEpisodeButton->setToolTip(tr("下一集"));
	pNextEpisodeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pNextEpisodeButton->setIcon(QIcon("ui/next_episode.png"));

	pOpenFileButton->setToolTip(tr("打开文件"));
	pOpenFileButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pOpenFileButton->setIcon(QIcon("ui/openfile.png"));

	this->setStyleSheet(PlayerCtlBtWidget_SS);
	pPlayButton->setStyleSheet(PlayerCtlBt_SS);
	pStopButton->setStyleSheet(PlayerCtlBt_SS);
	pPrevEpisodeButton->setStyleSheet(PlayerCtlBt_SS);
	pNextEpisodeButton->setStyleSheet(PlayerCtlBt_SS);
	pOpenFileButton->setStyleSheet(PlayerCtlBt_SS);

	//设置按钮大小
	pPlayButton->setFixedSize(40, 40);
	pStopButton->setFixedSize(40, 40);
	pPrevEpisodeButton->setFixedSize(40, 40);
	pNextEpisodeButton->setFixedSize(40, 40);
	pOpenFileButton->setFixedSize(40, 40);

	connect(pPlayButton, &QPushButton::clicked, this, &PlayerCtlButtons::Play_clicked);
	connect(pPlayButton, &QPushButton::clicked, this, &PlayerCtlButtons::Pause_clicked);
	connect(pStopButton, &QPushButton::clicked, this, &PlayerCtlButtons::Stop_clicked);
	connect(pOpenFileButton, &QPushButton::clicked, this, &PlayerCtlButtons::Open_clicked);
}

PlayerCtlButtons::~PlayerCtlButtons()
{

}

void PlayerCtlButtons::Play_clicked()
{

}

void PlayerCtlButtons::Pause_clicked()
{

}

void PlayerCtlButtons::Stop_clicked()
{

}

void PlayerCtlButtons::Open_clicked()
{
	strSupportVideoList = "*.avi,*.mp4,*.rmvb,*.mov,*.mkv,*.flv,*.ts";
	strSupportAudioList = "*.wav,*.mp3,*.aac,*.flac";
	strSupportListFile = "*.m3u,*.m3u8";
	strSupportAllList = strSupportVideoList + strSupportAudioList + strSupportListFile;
	QString strFilter = QString("支持的所有文件(%1);;视频文件(%2);;音频文件(%3);;播放列表文件(%4);;所有文件(*.*)")
		.arg(strSupportAllList)
		.arg(strSupportVideoList)
		.arg(strSupportAudioList)
		.arg(strSupportListFile);

	QStringList filePaths = QFileDialog::getOpenFileNames(this,
		tr("Open File"),
		".", 
		tr(strFilter.toStdString().c_str())
	);

	if (!filePaths.isEmpty())
	{
		for (auto filePath : filePaths)
		{
			if (!filePath.isNull())
			{
				//播放文件

				//获取文件名
				QString filename = filePath.left(filePath.lastIndexOf("/") + 1);
				//添加到播放列表

			}
		}
	}

}

