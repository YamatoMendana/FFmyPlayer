﻿#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>
#include <QtGlobal>
#include <QWidget>

#include "StyleSheet.h"


MainWindow::MainWindow(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    QSize m_size(1280,720);
    this->resize(m_size);
    this->setObjectName("MainWidget");
    this->setStyleSheet(MainWidget_SS);
	//鼠标追踪
	setMouseTracking(true);
	//加入过滤器
	installEventFilter(this);
	////设置悬停
	//setAttribute(Qt::WA_Hover, true);

    pTitleBar = new TitleBar(this);
    pTitleBar->resize(this->width(), 30);
    installEventFilter(pTitleBar);

    //设置窗口标题
    setWindowTitle("FFmyPlayer");
    //设置窗口图标
	//something

	//主窗口 = 播放窗口+播放列表
	QWidget* pWidget1 = new QWidget();//左侧
	QWidget* pWidget2 = new QWidget();//右侧

    //创建播放窗口
    pPlayWidget = new PlayerWidget(pWidget1);
	playWinId = pPlayWidget->winId();

	//创建播放控制按钮窗口
	pPlayCtlWidget = new PlayerCtlWidget(pWidget1);

	//创建播放列表
	pPlayListWidget = new playerListWidget();

	QVBoxLayout* pVlayout1 = new QVBoxLayout();
	pVlayout1->addWidget(pPlayWidget);
	pVlayout1->addWidget(pPlayCtlWidget);
	pVlayout1->setContentsMargins(0, 0, 0, 0);
	pVlayout1->setSpacing(1);
	pWidget1->setLayout(pVlayout1);

	QHBoxLayout* pHlayout1 = new QHBoxLayout();
	pHlayout1->addWidget(pWidget1);
	pHlayout1->addWidget(pPlayListWidget);
	pHlayout1->setContentsMargins(5,5,5,5);
	pWidget2->setLayout(pHlayout1);

	QVBoxLayout* pVlayout = new QVBoxLayout();
	pVlayout->addWidget(pTitleBar);
	pVlayout->addWidget(pWidget2);
	pVlayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(pVlayout);

	


	pPlayManager = new PlayerManager();

	connect(pPlayCtlWidget->getPlayerCtlButtonsPtr(), &PlayerCtlButtons::sigPlayFile, this, [&](QString filename) {
		pPlayManager->open_file(filename, playWinId);
		});
	connect(pPlayCtlWidget->getPlayerCtlButtonsPtr(), &PlayerCtlButtons::sigPlayStatusChange, this, [&](){
		bool playstatus = pPlayManager->playStatus_toggle();
		pPlayCtlWidget->setPlayIcon(playstatus);
		});
	connect(pPlayCtlWidget->getPlayerCtlButtonsPtr(), &PlayerCtlButtons::sigPlayStop, this, [&](){
		pPlayManager->play_stop();
		pPlayCtlWidget->setPlayIcon(true);
		pPlayWidget->update();
		});
	connect(pPlayCtlWidget->getPlayerCtlButtonsPtr(), &PlayerCtlButtons::sigSeekForword, pPlayManager, &PlayerManager::seek_forward);
	connect(pPlayCtlWidget->getPlayerCtlButtonsPtr(), &PlayerCtlButtons::sigSeekBack, pPlayManager, &PlayerManager::seek_back);

	connect(pPlayManager->getDisplayPtr().get(), &PlayerDisplay::sigVideoPlaySeconds, pPlayCtlWidget, &PlayerCtlWidget::setPlayingSeconds);
	connect(pPlayManager->getDisplayPtr().get(), &PlayerDisplay::sigVideoTotalSeconds, pPlayCtlWidget, &PlayerCtlWidget::setPlayingTotalSeconds);
	connect(pPlayManager->getDisplayPtr().get(), &PlayerDisplay::sigFileOpen, this, [&](QString filename) {
		//加入播放列表
		QWidget* tabwidget = pPlayListWidget->widget(0);
		qDebug() << tabwidget->children();
		PlayList* list = tabwidget->findChild<PlayList*>("DefaultAlbumList");
		list->addItem(filename);
		//改变播放按钮
		pPlayCtlWidget->setPlayIcon(false);
		});
	connect(pPlayListWidget, &playerListWidget::sigOpenfile, this, [&](QString filename) {
		pPlayManager->open_file(filename, playWinId);
		});
}

MainWindow::~MainWindow()
{

}

void MainWindow::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event);

    pTitleBar->resize(this->width(), 30);
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
	if(event->button() == Qt::LeftButton)
	{
		this->setFocus();
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		QPoint currentPot = mouseEvent->globalPos();
		if (left_MouseChange_Rect.contains(currentPot))
		{
			m_gPrevPot = event->globalPos();
			m_Rect = this->geometry();
			m_bDragging = true;
			m_bMoveFlagLX = true;
		}
		else if (right_MouseChange_Rect.contains(currentPot))
		{
			m_gPrevPot = event->globalPos();
			m_Rect = this->geometry();
			m_bDragging = true;
			m_bMoveFlagRX = true;
		}
		else if (bottom_MouseChange_Rect.contains(currentPot))
		{
			m_gPrevPot = event->globalPos();
			m_Rect = this->geometry();
			m_bDragging = true;
			m_bMoveFlagDY = true;
		}
		else if (left_Corner_MouseChange_Rect.contains(currentPot))
		{
			m_gPrevPot = event->globalPos();
			m_Rect = this->geometry();
			m_bDragging = true;
			m_bMoveFlagLDY = true;
		}
		else if (right_Corner_MouseChange_Rect.contains(currentPot))
		{
			m_gPrevPot = event->globalPos();
			m_Rect = this->geometry();
			m_bDragging = true;
			m_bMoveFlagRDY = true;
		}
	}

}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
	m_gCurrentPot = mouseEvent->globalPos();

	if (left_MouseChange_Rect.contains(m_gCurrentPot))
	{
		QPixmap pix("ui/horizontal_stretching.png");
		QSize size(15, 15);
		pix = pix.scaled(size, Qt::KeepAspectRatio);
		this->setCursor(QCursor(pix, pix.height() / 8, pix.width() / 8));
	}
	else if (right_MouseChange_Rect.contains(m_gCurrentPot))
	{
		QPixmap pix("ui/horizontal_stretching.png");
		QSize size(15, 15);
		pix = pix.scaled(size, Qt::KeepAspectRatio);
		this->setCursor(QCursor(pix, pix.height() / 8, pix.width() / 8));
	}
	else if (bottom_MouseChange_Rect.contains(m_gCurrentPot))
	{
		QPixmap pix("ui/vertical_stretching.png");
		QSize size(15, 15);
		pix = pix.scaled(size, Qt::KeepAspectRatio);
		this->setCursor(QCursor(pix, pix.height() / 8, pix.width() / 8));


	}
	else if (left_Corner_MouseChange_Rect.contains(m_gCurrentPot))
	{
		QPixmap pix("ui/left_diagonal_stretch.png");
		QSize size(15, 15);
		pix = pix.scaled(size, Qt::KeepAspectRatio);
		this->setCursor(QCursor(pix, pix.height() / 8, pix.width() / 8));


	}
	else if (right_Corner_MouseChange_Rect.contains(m_gCurrentPot))
	{
		QPixmap pix("ui/right_diagonal_stretch.png");
		QSize size(15, 15);
		pix = pix.scaled(size, Qt::KeepAspectRatio);
		this->setCursor(QCursor(pix, pix.height() / 8, pix.width() / 8));


	}
	else
	{
		this->setCursor(Qt::ArrowCursor);
	}

	if (event->buttons() & Qt::LeftButton)
	{
		int offsetX = m_gCurrentPot.x() - m_gPrevPot.x();
		int offsetY = m_gCurrentPot.y() - m_gPrevPot.y();


		if (m_bDragging)
		{
			int RectX = m_Rect.x();
			int RectY = m_Rect.y();
			int RectW = m_Rect.width();
			int RectH = m_Rect.height();
			
			if (m_bMoveFlagLX)
			{
				int ResizeX = RectX + offsetX;
				int ResizeW = RectW - offsetX;
				if(this->minimumWidth() <= ResizeW)
				{
					QRect offsetRect(ResizeX, RectY, ResizeW, RectH);
					this->setGeometry(offsetRect);
				}
				
			}
			else if (m_bMoveFlagRX)
			{
				int ResizeW = RectW + offsetX;
				QRect offsetRect(RectX, RectY, ResizeW, RectH);
				this->setGeometry(offsetRect);
			}
			else if (m_bMoveFlagDY)
			{
				int ResizeH = RectH + offsetY;
				this->setGeometry(RectX, RectY, RectW, ResizeH);
			}
			else if (m_bMoveFlagLDY)
			{
				int ResizeX = RectX + offsetX;
				int ResizeW = RectW - offsetX;
				int ResizeH = RectH + offsetY;

				if (this->minimumWidth() <= ResizeW && ResizeX !=RectX)
				{
					QRect offsetRect(ResizeX, RectY, ResizeW, RectH);
					this->setGeometry(offsetRect);
				}
				if (this->minimumHeight() <= ResizeH && ResizeH != RectY) {
					this->setGeometry(RectX, RectY, ResizeW, ResizeH);
				}
				if (this->minimumWidth() <= ResizeW && 
					this->minimumHeight() <= ResizeH && 
					ResizeX != RectX &&
					ResizeH != RectY)
				{
					this->setGeometry(ResizeX, RectY, ResizeW, ResizeH);
				}
			}
			else if (m_bMoveFlagRDY)
			{
				int ResizeW = RectW + offsetX;
				int ResizeH = RectH + offsetY;
				
				this->setGeometry(RectX, RectY, ResizeW, ResizeH);
			}
		}
	}

}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
	m_bMoveFlagLX = false;
	m_bMoveFlagRX = false;
	m_bMoveFlagDY = false;
	m_bMoveFlagLDY = false;
	m_bMoveFlagRDY = false;
	m_bDragging = false;
	this->setCursor(Qt::ArrowCursor);

}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::Resize:
	{
		//左边 拉伸图标区域
		left_MouseChange_Rect = QRect(this->x(), this->y() + 25, cornerX, this->height() - (cornerX + cornerY));
		//右边 拉伸图标区域
		right_MouseChange_Rect = QRect(this->x() + this->width() - cornerX, this->y() + 25, cornerX, this->height() - (cornerX + cornerY));
		//下方 拉伸图标区域
		bottom_MouseChange_Rect = QRect(this->x() + cornerX, this->y() + this->height() - cornerY, this->width() - (cornerX + cornerY), cornerY);
		//左上角 拉伸图标区域
		left_Corner_MouseChange_Rect = QRect(this->x(), this->y() + this->height() - cornerX, cornerX, cornerY);
		//右上角 拉伸图标区域
		right_Corner_MouseChange_Rect = QRect(this->x() + this->width() - cornerX, this->y() + this->height() - cornerY, cornerX, cornerY);

	}
	break;
	case QEvent::Move:
	{
		//左边 拉伸图标区域
		left_MouseChange_Rect = QRect(this->x(), this->y() + cornerY, cornerX, this->height() - (cornerX + cornerY));
		//右边 拉伸图标区域
		right_MouseChange_Rect = QRect(this->x() + this->width() - cornerX, this->y() + cornerY, cornerX, this->height() - (cornerX + cornerY));
		//下方 拉伸图标区域
		bottom_MouseChange_Rect = QRect(this->x() + cornerX, this->y() + this->height() - cornerY, this->width() - (cornerX + cornerY), cornerY);
		//左上角 拉伸图标区域
		left_Corner_MouseChange_Rect = QRect(this->x(), this->y() + this->height() - cornerX, cornerX, cornerY);
		//右上角 拉伸图标区域
		right_Corner_MouseChange_Rect = QRect(this->x() + this->width() - cornerX, this->y() + this->height() - cornerY, cornerX, cornerY);

	}
	break;
	}
    return QWidget::eventFilter(obj, event);
}
