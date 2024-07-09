#ifndef FILE_MAP_H
#define FILE_MAP_H

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
    };
    #endif
#endif


class avio_reading
{
public:
    avio_reading();

    struct buffer_data {
        uint8_t* ptr;
        size_t size; ///< size left in the buffer
    };

    static int read_packet(void* opaque, uint8_t* buf, int buf_size);
    int begin(const char* filename);
};

#endif // FILE_MAP_H
