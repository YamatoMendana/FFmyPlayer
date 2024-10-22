﻿#include "title_bar.h"
#include "StyleSheet.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>


TitleBar::TitleBar(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	this->setFixedHeight(30);
	//指示小部件应该使用有样式的背景绘制。
	this->setAttribute(Qt::WA_StyledBackground, true);

	//控件初始化
	pMinimize_bt = new QPushButton(this);
	pZoom_bt = new QPushButton(this);
	pClose_bt = new QPushButton(this);
	pMenu_bt = new QPushButton(this);
	pTitle = new QLabel(this);
	pMenu = new QMenu();

	QHBoxLayout* pHlayout = new QHBoxLayout();
	QHBoxLayout* pHlayout_bt = new QHBoxLayout();
	pHlayout_bt->addWidget(pMinimize_bt);
	pHlayout_bt->addWidget(pZoom_bt);
	pHlayout_bt->addWidget(pClose_bt);
	pHlayout_bt->setContentsMargins(0, 0, 0, 0);
	pHlayout_bt->setSpacing(0);

	pHlayout->addWidget(pMenu_bt);
	pHlayout->addWidget(pTitle);
	pHlayout->addLayout(pHlayout_bt);
	pHlayout->setContentsMargins(10, 0, 10, 0);
	pHlayout->setSpacing(20);

	this->setLayout(pHlayout);

	//设置唯一标识符
	this->setObjectName("TitleBar");
	pMenu_bt->setObjectName("MenuButton");
	pMinimize_bt->setObjectName("MinimizeButton");
	pZoom_bt->setObjectName("ZoomButton");
	pClose_bt->setObjectName("CloseButton");
	pTitle->setObjectName("WidgetTitle");

	pMenu_bt->setToolTip(tr("显示主菜单"));
	pMenu_bt->setText("FFmyPlayer");
	pMenu_bt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pMenu_bt->installEventFilter(this);

	QAction* openFileAction = new QAction(tr("打开文件"), pMenu);
	QAction* openAction = new QAction(tr("打开"), pMenu);
	QAction* exitAction = new QAction(tr("退出"), pMenu);
	pMenu->addAction(openFileAction);
	pMenu->addAction(openAction);
	pMenu->addSeparator();
	pMenu->addAction(exitAction);
	pMenu_bt->setMenu(pMenu);

	pMinimize_bt->setToolTip(tr("最小化"));
	pMinimize_bt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pMinimize_bt->setIcon(QIcon("ui/MinimizeButton.png"));
	pMinimize_bt->installEventFilter(this);
	
	pZoom_bt->setToolTip(tr("最大化"));
	pZoom_bt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pZoom_bt->setIcon(QIcon("ui/maximizeWindow.png"));
	pZoom_bt->installEventFilter(this);

	pClose_bt->setToolTip(tr("关闭"));
	pClose_bt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pClose_bt->setIcon(QIcon("ui/close.png"));
	pClose_bt->installEventFilter(this);

	this->setStyleSheet(TitleWidget_SS);
	pTitle->setStyleSheet(WidgetTitle_SS);
	pMenu_bt->setStyleSheet(Button_normal_SS);
	pMinimize_bt->setStyleSheet(Button_normal_SS);
	pZoom_bt->setStyleSheet(Button_normal_SS);
	pClose_bt->setStyleSheet(Button_normal_SS);
	pMenu->setStyleSheet(Menu_normal_SS);

	//标题栏大小
	pTitle->setMinimumHeight(25);
	pTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pTitle->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	//设置按钮大小
	pMinimize_bt->setFixedSize(30, 25);
	pZoom_bt->setFixedSize(30, 25);
	pClose_bt->setFixedSize(30, 25);



	//关联函数
    connect(pMinimize_bt, &QPushButton::clicked, this, &TitleBar::onClicked);
    connect(pZoom_bt, &QPushButton::clicked, this, &TitleBar::onClicked);
	connect(pClose_bt, &QPushButton::clicked, this, &TitleBar::onClicked);

}

TitleBar::~TitleBar()
{

}

void TitleBar::setTitleText(QString str)
{
	pTitle->setText(str);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
	Q_UNUSED(event);

	emit pZoom_bt->clicked();
}

void TitleBar::mousePressEvent(QMouseEvent* event)
{
	m_mousePosition = event->pos(); //鼠标在控件中的位置

	m_isMousePressed = true;
}

void TitleBar::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isMousePressed)
	{
		QWidget* pWid = this->window();

		if (pWid->isMaximized())
		{
			pWid->showNormal();

			m_mousePosition = QPoint(200, 100);
			pWid->move(event->globalPos().x() - 200, event->globalPos().y());
		}
		else
		{
			QPoint movePot = event->globalPos() - m_mousePosition;
			pWid->move(movePot);
		}
	}

}

void TitleBar::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);

	m_isMousePressed = false;
}

bool TitleBar::eventFilter(QObject* obj, QEvent* event)
{
	switch (event->type())
	{
	case QEvent::WindowTitleChange:
	{
		QWidget* pWid = qobject_cast<QWidget*>(obj);
		if (pWid)
		{
			pTitle->setText(pWid->windowTitle());
		}
		
	}
	break;
	case QEvent::WindowIconChange:
	{
		QWidget* pWid = qobject_cast<QWidget*>(obj);
		if (pWid)
		{
			QIcon icon = pWid->windowIcon();
		}
	}
	break;
	case QEvent::Resize:
	{
		
	}
	break;
	case QEvent::Enter:
	{
		if (obj == pMinimize_bt)
		{
			pMinimize_bt->setIcon(QIcon("ui/MinimizeButton_hover.png"));
		}
		else if (obj == pZoom_bt)
		{
			if (pZoom_bt->property("ZoomProperty") == "向下还原")
			{
				pZoom_bt->setIcon(QIcon("ui/RestoreWindow_hover.png"));
			}
			else
			{
				pZoom_bt->setIcon(QIcon("ui/maximizeWindow_hover.png"));
			}
		}
		else if (obj == pClose_bt)
		{

			pClose_bt->setIcon(QIcon("ui/close_Hover.png"));
		}

	}
	break;
	case QEvent::Leave:
	{
		if (obj == pMinimize_bt)
		{
			pMinimize_bt->setIcon(QIcon("ui/MinimizeButton.png"));

		}
		else if (obj == pZoom_bt)
		{
			if (pZoom_bt->property("ZoomProperty") == "向下还原")
			{
				pZoom_bt->setIcon(QIcon("ui/RestoreWindow.png"));

			}
			else
			{
				pZoom_bt->setIcon(QIcon("ui/maximizeWindow.png"));

			}
		}
		else if (obj == pClose_bt)
		{
			pClose_bt->setIcon(QIcon("ui/close_Hover.png"));
		}
	}
	break;
	default:
		break;
	}

	return QWidget::eventFilter(obj, event);
}

void TitleBar::onClicked()
{
	QPushButton* bt = qobject_cast<QPushButton*>(sender());
	QWidget* pWid = this->window();

	if (pWid->isTopLevel())
	{
		if (bt == pMinimize_bt)
		{
			pWid->showMinimized();
		}
		else if (bt == pZoom_bt)
		{
			pWid->isMaximized() ? pWid->showNormal() : pWid->showMaximized();
			updateMaximize();

		}
		else if (bt == pClose_bt)
		{
			pWid->close();
		}
	}

}

void TitleBar::updateMaximize()
{
	QWidget* pWid = this->window();
	if (pWid->isTopLevel())
	{
		if (pWid->isMaximized())
		{
			pZoom_bt->setToolTip(tr("向下还原"));
			pZoom_bt->setProperty("ZoomProperty", "向下还原");
			//更改图标
			pZoom_bt->setIcon(QIcon("ui/RestoreWindow.png"));

		}
		else
		{
			pZoom_bt->setToolTip(tr("向下还原"));
			pZoom_bt->setProperty("ZoomProperty", "最大化");
			//更改图标
			pZoom_bt->setIcon(QIcon("ui/maximizeWindow.png"));

		}
		pZoom_bt->setStyle(QApplication::style());
	}
}

