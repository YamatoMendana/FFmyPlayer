#pragma once
#define _XOPEN_SOURCE 600 /* for usleep */
#include <stdio.h>
#include <stdlib.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>

static AVFormatContext* fmt_ctx;
static AVCodecContext* dec_ctx;
AVFilterContext* buffersink_ctx;//出参的过滤器上下文
AVFilterContext* buffersrc_ctx;//入参的过滤器上下文
AVFilterGraph* filter_graph;
static int video_stream_index = -1;
static int64_t last_pts = AV_NOPTS_VALUE;

static int open_input_file(const char* filename);
static int init_filters(const char* filters_descr);
static void display_frame(const AVFrame* frame, AVRational time_base);
static void write_frame(const AVFrame* frame);
int filtering_video_main(const char* filename);
