#include "filtering_Video.h"

const char* filter_descr = "scale=1280:720";
/* other way:
   scale=78:24 [scl]; [scl] transpose=cclock // assumes "[in]" and "[out]" to be input output pads respectively
 */
/**
 * 打开输入文件,和音频处理类似. 解码出帧后.放入过滤器中处理
 * 过滤器有一个输入过滤器,一个输出过滤器.中又混合了string组成的过滤器.通过 AVFilterGraph 连接起来
 * @param filename
 * @return
 */
static int open_input_file(const char* filename)
{
    int ret;
    AVCodec* dec;
    //打开文件.创建ftm_ctx格式上下文
    if ((ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }
    //找到合适的流信息
    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    av_dump_format(fmt_ctx,0, filename,0);

    /* select the video stream */
    //选择视频流,得到索引,获取解码器
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &dec, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot find a video stream in the input file\n");
        return ret;
    }
    video_stream_index = ret;
    //创建解码器上下文
        /* create decoding context */
    dec_ctx = avcodec_alloc_context3(dec);
    if (!dec_ctx)
        return AVERROR(ENOMEM);
    //把视频流中的解码参数拷贝到解码器上下文中
    avcodec_parameters_to_context(dec_ctx, fmt_ctx->streams[video_stream_index]->codecpar);

    /* init the video decoder */
    //用解码器来设置解码器上下文的参数
    if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot open video decoder\n");
        return ret;
    }

    return 0;
}
/**
 * 初始化过滤器
 * @param filters_descr
 * @return
 */
static int init_filters(const char* filters_descr)
{
    char args[512];
    int ret = 0;
    //数据输入输出过滤器以及 分配的输入输出buf
    const AVFilter* buffersrc = avfilter_get_by_name("buffer");
    const AVFilter* buffersink = avfilter_get_by_name("buffersink");
    AVFilterInOut* outputs = avfilter_inout_alloc();
    AVFilterInOut* inputs = avfilter_inout_alloc();
    AVRational time_base = fmt_ctx->streams[video_stream_index]->time_base;
    enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };

    //初始化过滤器图形.用来连接多个过滤器
    filter_graph = avfilter_graph_alloc();
    if (!outputs || !inputs || !filter_graph) {
        ret = AVERROR(ENOMEM);
        goto end;
    }
    //把参数都写到 args里
    /* buffer video source: the decoded frames from the decoder will be inserted here. */
    snprintf(args, sizeof(args),
        "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
        dec_ctx->width, dec_ctx->height, dec_ctx->pix_fmt,
        time_base.num, time_base.den,
        dec_ctx->sample_aspect_ratio.num, dec_ctx->sample_aspect_ratio.den);
    //用args 参数和buufersrc初始化过滤器上下文,并添加到 过滤器图形中, 返回改过滤器上下文
    ret = avfilter_graph_create_filter(&buffersrc_ctx, buffersrc, "in",
        args, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer source\n");
        goto end;
    }
    //初始化另一个过滤器上下文,添加到过滤器图形汇总,
        /* buffer video sink: to terminate the filter chain. */
    ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
        NULL, NULL, filter_graph);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot create buffer sink\n");
        goto end;
    }

    //设置 pix_fmts属性
    ret = av_opt_set_int_list(buffersink_ctx, "pix_fmts", pix_fmts,
        AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Cannot set output pixel format\n");
        goto end;
    }

    /*
     * Set the endpoints for the filter graph. The filter_graph will
     * be linked to the graph described by filters_descr.
     */

     /*
      * The buffer source output must be connected to the input pad of
      * the first filter described by filters_descr; since the first
      * filter input label is not specified, it is set to "in" by
      * default.
      */
      //把两个输入输出的 上下文绑定到这两个 inout的结构体上,
      // 这里我理解是,因为有个字符串传入的过滤器.这里是一种连接字符串产生的过滤器和上边创建过滤器的方式
    outputs->name = av_strdup("in");
    outputs->filter_ctx = buffersrc_ctx;
    outputs->pad_idx = 0;
    outputs->next = NULL;

    /*
     * The buffer sink input must be connected to the output pad of
     * the last filter described by filters_descr; since the last
     * filter output label is not specified, it is set to "out" by
     * default.
     */
     //这个输出的input结构体必须和 desrc 里的最好一个过滤器相连
    inputs->name = av_strdup("out");
    inputs->filter_ctx = buffersink_ctx;
    inputs->pad_idx = 0;
    inputs->next = NULL;
    //把字符串解析出的过滤器和 inout 中的输入输出过滤器连接起来
    if ((ret = avfilter_graph_parse_ptr(filter_graph, filters_descr,
        &inputs, &outputs, NULL)) < 0)
        goto end;
    //进行配置
    if ((ret = avfilter_graph_config(filter_graph, NULL)) < 0)
        goto end;

end:
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    return ret;
}

static void display_frame(const AVFrame* frame, AVRational time_base)
{
	//int x, y;
	//uint8_t* p0, * p;
	//int64_t delay;

    //if (frame->pts != av_nopts_value) {
    //    if (last_pts != av_nopts_value) {
    //        /* sleep roughly the right amount of time;
    //         * usleep is in microseconds, just like av_time_base. */
    //        delay = av_rescale_q(frame->pts - last_pts,
    //            time_base, av_time_base_q);
    //        if (delay > 0 && delay < 1000000)
    //            usleep(delay);
    //    }
    //    last_pts = frame->pts;
    //}

    ///* trivial ascii grayscale display. */
    //p0 = frame->data[0];
    //puts("\033c");
    //for (y = 0; y < frame->height; y++) {
    //    p = p0;
    //    for (x = 0; x < frame->width; x++)
    //        putchar(" .-+#"[*(p++) / 52]);
    //    putchar('\n');
    //    p0 += frame->linesize[0];
    //}
    //fflush(stdout);

}

FILE* fd = NULL;
static void write_frame(const AVFrame* frame)
{
    static int printf_flag = 0;
    size_t ret = 0;
    if (!printf_flag)
    {
        printf_flag = 1;
        printf("frame width = %d, frame weight = %d\n", frame->width, frame->height);

        if (frame->format == AV_PIX_FMT_YUV420P)
        {
            printf("format is yuv420p\n");
        }
        else
        {
            printf("format is = %d\n", frame->format);
        }
    }
    int ySize = frame->width * frame->height;
	int uSize = ySize / 4;
	int vSize = ySize / 4;
	ret = fwrite(frame->data[0], 1, ySize, fd);
	ret = fwrite(frame->data[1], 1, uSize, fd);
	ret = fwrite(frame->data[2], 1, vSize, fd);


}

int filtering_video_main(const char* filename)
{
    int ret;
    AVPacket packet;
    AVFrame* frame;
    AVFrame* filt_frame;

    fd = fopen("yuv10s.yuv", "w");
    if(fd)
        printf("fd is opened\n");
    frame = av_frame_alloc();
    filt_frame = av_frame_alloc();
    if (!frame || !filt_frame) {
        perror("Could not allocate frame");
        exit(1);
    }

    if ((ret = open_input_file(filename)) < 0)
        goto end;
    if ((ret = init_filters(filter_descr)) < 0)
        goto end;

    /* read all packets */
    while (1) {
        //读取packet
        if ((ret = av_read_frame(fmt_ctx, &packet)) < 0)
            break;
        //处理视频
        if (packet.stream_index == video_stream_index) {
            //数据发送到解码器
            ret = avcodec_send_packet(dec_ctx, &packet);
            if (ret < 0) {
                av_log(NULL, AV_LOG_ERROR, "Error while sending a packet to the decoder\n");
                break;
            }

            while (ret >= 0) {

                //解码器中取出帧
                ret = avcodec_receive_frame(dec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                else if (ret < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while receiving a frame from the decoder\n");
                    goto end;
                }

                frame->pts = frame->best_effort_timestamp;

                /* push the decoded frame into the filtergraph */
                //把解码后的帧数据放入过滤器图中处理,
                if (av_buffersrc_add_frame_flags(buffersrc_ctx, frame, AV_BUFFERSRC_FLAG_KEEP_REF) < 0) {
                    av_log(NULL, AV_LOG_ERROR, "Error while feeding the filtergraph\n");
                    break;
                }

                /* pull filtered frames from the filtergraph */
                while (1) {//的到过滤器处理完的结果.输出帧
                    ret = av_buffersink_get_frame(buffersink_ctx, filt_frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    if (ret < 0)
                        goto end;
                    //展示帧数据.释放帧
                    //display_frame(filt_frame, buffersink_ctx->inputs[0]->time_base);
                    write_frame(filt_frame);
                    av_frame_unref(filt_frame);
                }
                av_frame_unref(frame);
            }
        }
        av_packet_unref(&packet);
    }
end:
    avfilter_graph_free(&filter_graph);
    avcodec_free_context(&dec_ctx);
    avformat_close_input(&fmt_ctx);
    av_frame_free(&frame);
    av_frame_free(&filt_frame);
    fclose(fd);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        exit(1);
    }

    exit(0);
}
