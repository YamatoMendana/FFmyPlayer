#ifndef STYLESHEET_H
#define STYLESHEET_H

#include <QString>

//主界面窗口
static const QString MainWidget_SS = "	\
	QWidget#MainWidget	\
	{	\
		background-color:rgb(0 ,0 ,0);	\
		border:1px;		\
	}	\
";
//播放窗口
static const QString PlayerWidget_SS = "	\
	QWidget#PlayerWidget	\
	{	\
		background-color:transparent	\
		border:1px;		\
	}	\
";
//标题栏窗口
static const QString TitleWidget_SS = "	\
	QWidget#TitleBar	\
	{	\
		background-color:rgb(31 ,31 ,31);	\
		border:1px;		\
	}	\
";
//标题栏窗口-标题部分
static const QString WidgetTitle_SS = ""
                                      "QLabel#WidgetTitle{"
                                      "background-color: transparent;"
                                      "border-width: 1px;"
                                      "color: rgb(240,240,240);"
                                      "}";
//按钮背景
static const QString Button_normal_SS = ""
                                        "QPushButton{"
                                        "background-color:transparent;"
                                        "border: none;"
                                        "color:rgb(240,240,240);"
                                        "}";

//菜单栏背景
static const QString Menu_normal_SS = ""
                                      "QMenu{"
                                      "background-color: rgb(68, 68, 68);"
                                      "}"
                                      "QMenu::item {"
                                      "background-color: rgb(68, 68, 68);"
                                      "color: rgb(187, 187, 187);"
                                      "}"
                                      "QMenu::item:selected{"
                                      "background-color: rgb(24,24,24);"
                                      "color: rgb(250,225,0);"
                                      "border: 1px;"
                                      "}";

//播放控制栏窗口
static const QString PlayerCtlWidget_SS = " \
	QWidget#PlayerCtlWidget	\
	{	\
		background-color:rgb(31 ,31 ,31);	\
		border: none;	\
	}	\
";
//播放控制栏窗口->按钮窗口
static const QString PlayerCtlBtWidget_SS = " \
	QWidget#PlayerCtlBtWidget	\
	{	\
		background-color:transparent;	\
		border: none;	\
	}	\
";

//播放控制栏窗口->按钮窗口->播放按钮
static const QString PlayerCtlBt_SS = ""
	"QPushButton{"
	"background-color:transparent;"
	"border: 1px solid black; }"
	"QPushButton:hover{"
	"background-color:rgb(39, 39, 39);"
	"border: 1px solid black; }"
	"";

//播放时长显示窗口
static const QString PlayingInfoWidget_SS = " \
	QWidget#PlayingInfoWidget	\
	{	\
		background-color:transparent;	\
		border: none;	\
	}	\
";
//播放时长显示窗口->显示标签
static const QString PlayingInfoEdit_SS = ""
    "QLineEdit{"
	"background-color:transparent;"
	"border: none;"
    "color: rgb(240,240,240);"
    "}"
    "QLineEdit:read-only{"
    "background-color:transparent;"
    "border: none;"
    "color: rgb(120,120,90);"
    "}";
//播放时长显示窗口->分割标签
static const QString PlayingInfoLabel_SS = ""
"QLabel{"
"background-color:transparent;"
"border: none;"
"color: rgb(120,120,90);"
"}";


static const QString PlayList_TabWidget_SS = ""
"QTabWidget#PlayListTabWidget::pane{"
"background-color:rgb(59, 59, 59);"
"border:none;"
"}"
"QTabBar{"
"background-color:rgb(59, 59, 59);"
"}"
"QTabBar::tab{"
"background-color:transparent;"
"color:rgb(136, 136, 136);"
"}"
"QTabBar::tab:selected{"
"background - color:transparent;"
"color:rgb(233, 233, 233);"
"}"
"QTabBar::tab:!selected{"
"background - color:transparent;"
"color:rgb(136, 136, 136);"
"}";

//static const QString PlayList_TarBar_SS = ""
//"QTabBar{"
//"background-color:rgb(59, 59, 59);"
//"}"
//"QTabBar::tab{"
//"background-color:transparent;"
//"color:rgb(136, 136, 136);"
//"}"
//"QTabBar::tab:selected{"
//"background - color:transparent;"
//"color:rgb(233, 233, 233);"
//"}"
//"QTabBar::tab:!selected{"
//"background - color:transparent;"
//"color:rgb(136, 136, 136);"
//"}";

static const QString PlayList_ListWidget_SS = ""
"QListWidget{"
"background-color:transparent;"
"}"
"QListWidget::item:selected{"
"color:rgb(233,233,233);"
"}"
"QListWidget::item:!selected{"
"color:rgb(136,136,136);"
"}";

//滑动条
static const QString PlayCtlSlider_SS = ""
"QSlider::groove:horizontal{"
"	border: 1px solid #4A708B;"
"	background: #C0C0C0;"
"	height: 3px;"
"	border - radius: 2px;"
"	padding - left:-1px;"
"	padding - right:-1px;"
"}"
/*已经划过的从地方*/
"QSlider::sub-page:horizontal{"
"	background: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 : 1,"
"		stop : 0 #B1B1B1, stop:1 #c4c4c4);"
"	background: qlineargradient(x1 : 0, y1 : 0.2, x2 : 1, y2 : 1,"
"		stop : 0 #5DCCFF, stop: 1 #1874CD);"
"	border: 1px solid #4A708B;"
"	border - radius: 2px;"
"}"
/*还没有滑上去的地方*/
"QSlider::add-page:horizontal{"
"	background: #575757;"
"	border: 0px solid #777;"
"	border - radius: 2px;"
"}"
/*中间滑块*/
"QSlider::handle:horizontal"
"{"
/*背景颜色设置为辐射聚焦*/
"	background: qradialgradient(spread : pad, cx : 0.5, cy : 0.5, radius : 0.5, fx : 0.5, fy : 0.5,"
"	stop : 0.6 #45ADED, stop:0.8 rgba(255, 255, 255, 255));"

/*形成圆*/
"width: 8px;"
"border - radius: 4px;"

/*上沿、下沿超出滑竿*/
"margin - top: -3px;"
"margin - bottom: -2px;"
"}"

"QSlider::handle:horizontal:hover{"
"	background: qradialgradient(spread : pad, cx : 0.5, cy : 0.5, radius : 0.5, fx : 0.5, fy : 0.5, stop : 0 #2A8BDA,"
"	stop:0.8 rgba(255, 255, 255, 255));"

/*形成圆*/
"width: 8px;"
"border - radius: 4px;"

/*上沿、下沿超出滑竿*/
"margin - top: -3px;"
"margin - bottom: -2px;"
"}";

#endif // STYLESHEET_H
