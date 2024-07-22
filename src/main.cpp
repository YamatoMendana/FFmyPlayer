#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QTranslator>

#include "MainWindow.h"

#include "title_bar.h"
#include "playerCtlButtons.h"



extern "C"
{

}

enum UseingLanguage
{
    CHINESE_LANGUAGE = 0,
    ENGLISH_LANGUAGE
};
UseingLanguage g_enLanguage;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //翻译
    QTranslator* trans = new QTranslator();
    int ret = 0;
    //以后再改到vector里面搭配对应函数，目前将就一下
    g_enLanguage = CHINESE_LANGUAGE;
    switch (g_enLanguage)
    {
    case CHINESE_LANGUAGE:
    {
        ret = trans->load("language/chinese.qm");
    }
    break;
    case ENGLISH_LANGUAGE:
    {
        ret = trans->load("language/english.qm");
    }
    break;
    }
    if (ret)
    {
		qDebug("The language pack is loaded");
		a.installTranslator(trans);
    }

    MainWindow w;
    
    w.show();

    return a.exec();
}
