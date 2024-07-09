#ifndef LIBAVUTILTEST_H
#define LIBAVUTILTEST_H


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
    };
    #endif
#endif

class cAVutil
{
public:
    void list_obj_test(void* obj);
    void test_opt();
    void test_log();
    void custom_outopt(void* ptr,int level,const char* fmt,va_list vl);
    void test_parseutil();
    void test_avdictionary();
    void print_opt(const AVOption* opt_test);
};





#endif // LIBAVUTILTEST_H
