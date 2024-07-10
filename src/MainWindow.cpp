#include "MainWindow.h"
#include <QVBoxLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QDebug>

#include "StyleSheet.h"


MainWindow::MainWindow()
{
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
    QSize m_size(1280,720);
    this->setFixedSize(m_size);
    this->setObjectName("MainWidget");
    this->setStyleSheet(MainWidget_SS);

	left_MouseChange_Point = QRect(this->x(), this->y() + cornerY, cornerX, this->height() - (cornerX + cornerY));
	right_MouseChange_Point = QRect(this->x() + this->width() - cornerX, this->y() + cornerY, cornerX, this->height() - (cornerX + cornerY));
	up_MouseChange_Point = QRect(this->x()+cornerX, this->y(), cornerX, this->width() - (cornerX + cornerY));
	down_MouseChange_Point = QRect(this->x()+cornerX, this->y()+this->height() - cornerY, cornerX, this->width() - (cornerX + cornerY));
    //左上角 拉伸图标区域
    leftUp_Corner_MouseChange_Point = QRect(this->x(),this->y(),cornerX,cornerY);
    //右上角 拉伸图标区域
    rightUp_Corner_MouseChange_Point = QRect(this->x() + width() - cornerX,this->y(),cornerX,cornerY);

    
    QVBoxLayout* pVlayout = new QVBoxLayout();


    pTitleBar = new TitleBar(this);
    pTitleBar->resize(this->width(), 30);
    installEventFilter(pTitleBar);

    //设置窗口标题
    setWindowTitle("FFmyPlayer");
    //设置窗口图标
    
    //创建播放窗口
    pPlayWidget = new PlayerWidget();

    pVlayout->addWidget(pTitleBar);
    pVlayout->addWidget(pPlayWidget);
    pVlayout->setContentsMargins(2, 2, 2, 2);
    this->setLayout(pVlayout);



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

}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
	QWidget* pWid = this->window();
	QPoint currentPot = event->pos();

}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{

}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{


    return QWidget::eventFilter(obj, event);
}
