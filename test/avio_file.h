#ifndef AVIO_FILE_H
#define AVIO_FILE_H

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


class avio_file
{
public:
    avio_file();
    ~avio_file();

    static int read_func(void* ptr,uint8_t* buf,int buf_size);
    static int64_t seek_func(void* opaque,int64_t offset, int whence);
    int openfile(const char* filename);
    int openfile2(const char* filename);

};

#endif // AVIO_FILE_H
