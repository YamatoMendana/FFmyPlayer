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
static const QString WidgetTitle_SS = "	\
	QTitle#WidgetTitle	\
	{	\
		background-color:transparent;	\
		border:1px;		\
	}	\
";
//按钮背景
static const QString Button_normal_SS = " \
	QPushButton	\
	{	\
		background-color:transparent;	\
		border: none;	\
	}	\
";

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






#endif // STYLESHEET_H
