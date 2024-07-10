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
		background-color:rgb(31 ,31 ,31);	\
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



#endif // STYLESHEET_H
