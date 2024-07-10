#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QFile>

#include "MainWindow.h"

extern "C"
{
#include "filtering_Video.h"
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    w.show();
    return a.exec();
}
