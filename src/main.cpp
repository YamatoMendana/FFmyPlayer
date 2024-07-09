#include "mainwindow.h"

#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QFile>

#include "libavutiltest.h"
#include "avio_file.h"
#include "avio_reading.h"
#include "demuxer_test.h"
#include "remuxertest.h"
#include "encode_video.h"
#include "live_test.h"

extern "C"
{
#include "filtering_Video.h"
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;


//    cAVutil util;
//    util.test_log();
//    util.test_avdictionary();
//    util.test_parseutil();
//    util.test_opt();
//    util.print_opt();

    QString infileName = "10s.mp4";
    QString outfileName = "10s.flv";
//    if (!QFile::exists(fileName))
//    {
//       QFile file(fileName);
//       if (file.open(QIODevice::ReadWrite))
//           file.close();
//    }

//    /* 文件绝对路径获取 */
//    QFileInfo info(fileName);
//    qDebug() << "absoluteFilePath: " << info.absoluteFilePath();
//    qDebug() << "absolutePath: " << info.absolutePath();

//    avio_file avio;
//    avio.openfile(fileName.toStdString().c_str());
//    avio.openfile2(fileName.toStdString().c_str());

//    avio_reading avread;
//    avread.begin(fileName.toStdString().c_str());

//    demuxer_test demuxer;
//    demuxer.start();

    //RemuxerTest remuxer;
    //remuxer.begin(infileName.toStdString().c_str(),outfileName.toStdString().c_str());

    //Encode_Video encodev;
    //encodev.begin("testencode.mp4","mpeg1video");

//    Demuxing_Decoding dexdec;
//    dexdec.begin("10s.mp4","10sv.yuv","10sa.pcm");

    //filtering_video_main("10s.mp4");

    live_test live;
    live.begin("1.mp4");

    w.show();
    return a.exec();
}
