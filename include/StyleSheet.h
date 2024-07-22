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


#endif // STYLESHEET_H
