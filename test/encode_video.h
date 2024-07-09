#ifndef ENCODE_VIDEO_H
#define ENCODE_VIDEO_H

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
    #include "libavutil/imgutils.h"
    #include "libavutil/timestamp.h"
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
        #include <libavutil/imgutils.h>
    #include <libavutil/timestamp.h>
    };
    #endif
#endif


class Encode_Video
{
public:
    Encode_Video();

    static void encode(AVCodecContext* encCtx,AVFrame* frame,AVPacket* pkt,FILE* outfile);

    int begin(const char *inputName, const char *outputName);
};

#endif // ENCODE_VIDEO_H
