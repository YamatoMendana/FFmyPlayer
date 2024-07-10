#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

#include "title_bar.h"
#include "playerWidget.h"


class MainWindow:public QWidget
{
public:
    MainWindow();
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	bool eventFilter(QObject* obj, QEvent* event);

private:
    TitleBar* pTitleBar;
    PlayerWidget* pPlayWidget;

    int mousePosX = 0;
    int mousePosY = 0;
    int cornerX = 10;
    int cornerY = 10;

    QRect left_MouseChange_Point;
    QRect right_MouseChange_Point;
    QRect up_MouseChange_Point;
    QRect down_MouseChange_Point;
    QRect leftUp_Corner_MouseChange_Point;
    QRect rightUp_Corner_MouseChange_Point;
    QRect leftDown_Corner_MouseChange_Point;
    QRect rightDown_Corner_MouseChange_Point;
    bool moveFlagX = false;
    bool moveFlagY = false;
};





#endif // MAINWINDOW_H
