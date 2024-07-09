#ifndef DEMUXER_TEST_H
#define DEMUXER_TEST_H

#include <stdio.h>
#include <iostream>


#ifdef _WIN32
extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/opt.h"
    #include "libavutil/parseutils.h"
    #include "libavutil/avutil.h"
    #include "libavformat/avio.h"
    #include "libavutil/file.h"
    #include "libavutil/rational.h"
};
#else
    #ifdef __cplusplus
    extern "C"
    {
        #include <libavcodec/avcodec.h>
        #include <libavformat/avformat.h>
        #include <libavutil/opt.h>
        #include <libavutil/parseutils.h>
        #include <libavutil/avutil.h>
        #include <libavformat/avio.h>
        #include <libavutil/file.h>
        #include <libavutil/rational.h>
    };
    #endif
#endif

class demuxer_test
{
public:
    demuxer_test();

    int start();
private:
    inline double r2d(AVRational r){
        return r.den==0?0:(double)r.num / (double)r.den;
    }
};

#endif // DEMUXER_TEST_H
