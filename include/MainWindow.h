#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <shared_mutex>
#include <memory>


#include "title_bar.h"
#include "playerWidget.h"
#include "playerCtlWidget.h"
#include "playerManager.h"
#include "playerListWidget.h"
#include "playerCtlSlider.h"

class MainWindow:public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();


protected:
    void resizeEvent(QResizeEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	bool eventFilter(QObject* obj, QEvent* event) override;

    

private:
    TitleBar* pTitleBar = nullptr;
    PlayerWidget* pPlayWidget = nullptr;
    PlayerCtlWidget* pPlayCtlWidget = nullptr;
    PlayerCtlSlider* pPlayCtlSlider = nullptr;
    playerListWidget* pPlayListWidget = nullptr;
    PlayerManager* pPlayManager = nullptr;
    
    
    
    WId playWinId;

    int mousePosX = 0;
    int mousePosY = 0;
    int cornerX = 10;
    int cornerY = 10;
    
    //可视化范围
    QRect m_availableWindowRect;
    //当前相对坐标
    QPoint m_currentPot; 
    //当前全局坐标
    QPoint m_gCurrentPot;
    //移动前全局坐标
    QPoint m_gPrevPot;
    //窗口位置大小
    QRect m_Rect;

    QRect left_MouseChange_Rect;
    QRect right_MouseChange_Rect;
    QRect bottom_MouseChange_Rect;

    QRect left_Corner_MouseChange_Rect;
    QRect right_Corner_MouseChange_Rect;
    bool m_bMoveFlagLX = false;
    bool m_bMoveFlagRX = false;
    bool m_bMoveFlagDY = false;
    bool m_bMoveFlagLDY = false;
    bool m_bMoveFlagRDY = false;
    bool m_bDragging = false;

};





#endif // MAINWINDOW_H
