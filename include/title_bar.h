#ifndef TITLE_BAR_H
#define TITLE_BAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMenu>

#pragma execution_character_set("utf-8")

class TitleBar:public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    void setTitleText(QString str);

protected:
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    bool eventFilter(QObject* obj, QEvent* event);

private slots:
    void onClicked();
    void updateMaximize();

private:
    //最小化按钮
    QPushButton* pMinimize_bt = nullptr;
    //缩放按钮
    QPushButton* pZoom_bt = nullptr;
    //关闭按钮
    QPushButton* pClose_bt = nullptr;
    //菜单按钮
    QPushButton* pMenu_bt = nullptr;
    //标题栏
    QLabel* pTitle = nullptr;
    //菜单栏
    QMenu* pMenu = nullptr;
    //鼠标按下时的位置
	QPoint m_mousePosition; 
    //鼠标是否摁下
	bool m_isMousePressed; 
};









#endif // TITLE_BAR_H
