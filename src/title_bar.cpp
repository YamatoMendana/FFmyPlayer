#include "title_bar.h"
#include "StyleSheet.h"

#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>


TitleBar::TitleBar(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	this->setFixedHeight(30);

	//控件初始化
	pMinimize_bt = new QPushButton(this);
	pZoom_bt = new QPushButton(this);
	pClose_bt = new QPushButton(this);
	pTitle = new QLabel(this);
	pIcon = new QLabel(this);

	QHBoxLayout* pHlayout = new QHBoxLayout();
	QHBoxLayout* pHlayout_bt = new QHBoxLayout();
	pHlayout_bt->addWidget(pMinimize_bt);
	pHlayout_bt->addWidget(pZoom_bt);
	pHlayout_bt->addWidget(pClose_bt);
	pHlayout_bt->setContentsMargins(0, 0, 0, 0);
	pHlayout_bt->setSpacing(0);

	pHlayout->addWidget(pIcon);
	pHlayout->addWidget(pTitle);
	pHlayout->addLayout(pHlayout_bt);
	pHlayout->setContentsMargins(5, 0, 5, 0);
	pHlayout->setSpacing(0);

	this->setLayout(pHlayout);

	//设置唯一标识符
	this->setObjectName("TitleBar");
	pMinimize_bt->setObjectName("MinimizeButton");
	pZoom_bt->setObjectName("ZoomButton");
	pClose_bt->setObjectName("CloseButton");
	pTitle->setObjectName("WidgetTitle");
	pIcon->setObjectName("IconButton");

	pMinimize_bt->setToolTip(tr("最小化"));
	pTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//配置图标
	pMinimize_bt->setIcon(QIcon("ui/MinimizeButton.png"));
	pMinimize_bt->installEventFilter(this);
	

	pZoom_bt->setToolTip(tr("最大化"));
	pZoom_bt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//配置图标
	pZoom_bt->setIcon(QIcon("ui/maximizeWindow.png"));
	pZoom_bt->installEventFilter(this);

	pClose_bt->setToolTip(tr("关闭"));
	pClose_bt->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	//配置图标
	pClose_bt->setIcon(QIcon("ui/close.png"));
	pClose_bt->installEventFilter(this);

	this->setStyleSheet(TitleWidget_SS);
	pTitle->setStyleSheet(WidgetTitle_SS);
	pMinimize_bt->setStyleSheet(Button_normal_SS);
	pZoom_bt->setStyleSheet(Button_normal_SS);
	pClose_bt->setStyleSheet(Button_normal_SS);

	//图片自适应控件大小
	pIcon->setFixedSize(20, 20);
	//配置图标

	pIcon->setScaledContents(true);

	//标题栏大小
	pTitle->setMinimumHeight(25);
	pTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	pTitle->setAlignment(Qt::AlignCenter);
	//标题颜色
	QPalette palette = pTitle->palette();
	palette.setColor(QPalette::WindowText, QColor(120,120,120)); // 设置文字颜色为红色
	pTitle->setPalette(palette);
	//标题加粗
	QFont font = pTitle->font();
	font.setBold(true); // 设置字体加粗
	pTitle->setFont(font);

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
			pIcon->setPixmap(icon.pixmap(pTitle->size()));
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

