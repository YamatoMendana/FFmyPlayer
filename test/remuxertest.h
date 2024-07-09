#ifndef REMUXERTEST_H
#define REMUXERTEST_H

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
    #include "libavutil/mem.h"
    #include "libavutil/mathematics.h"
    #include "libavcodec/packet.h"
    #include "libavutil/time.h"
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
        #include <libavutil/mem.h>
        #include <libavutil/mathematics.h>
        #include <libavcodec/packet.h>
        #include <libavutil/time.h>
    };
    #endif
#endif

class RemuxerTest
{
public:
    RemuxerTest();

    int begin(const char *inputName, const char *outputName);
};

#endif // REMUXERTEST_H
