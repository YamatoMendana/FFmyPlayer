#include "live_test.h"


int live_test::begin(const char* filename)
{
    AVOutputFormat* pOfmt = nullptr;
    AVFormatContext* pIfmt_ctx = nullptr,*pOfmt_ctx = nullptr;

    AVPacket* pkt;
    const char* in_filename = nullptr, * out_filename = nullptr;


    int ret = 0, i = 0;
    int videoindex = 0;
    int frameindex = 0;

    int64_t start_time = 0;

    in_filename = filename;
    out_filename = "rtmp://192.168.22.128/live/test";

    avformat_network_init();

    //输入
    if ((ret = avformat_open_input(&pIfmt_ctx, in_filename, 0, 0)) < 0)
    {
        printf("could not open input file.\n");
        goto end;
    }
    if ((ret = avformat_find_stream_info(pIfmt_ctx, 0)) < 0)
    {
        printf("Failed to retrieve input stream information.\n");
        goto end;
    }

    for (i = 0; i < pIfmt_ctx->nb_streams; i++)
    {
        if (pIfmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    }
    av_dump_format(pIfmt_ctx, 0, in_filename, 0);

    //输出
    avformat_alloc_output_context2(&pOfmt_ctx, nullptr, "flv", out_filename);

    if (!pOfmt_ctx)
    {
        printf("Could not create output context.\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    pOfmt = const_cast<AVOutputFormat*>( pOfmt_ctx->oformat );
    for (i = 0; i < pIfmt_ctx->nb_streams; i++)
    {
        AVCodecContext* pCodec_ctx;
        AVStream* in_Stream = pIfmt_ctx->streams[i];
        //找到输入流对应的编码器
        AVCodec* pCodec = const_cast<AVCodec*>(avcodec_find_decoder(in_Stream->codecpar->codec_id));
        //创建输出流
        AVStream* out_stream = avformat_new_stream(pOfmt_ctx, pCodec);
        if (!out_stream)
        {
            printf("Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        //创建编解码上下文
        pCodec_ctx = avcodec_alloc_context3(pCodec);
        //新版本中FFmpeg的avcodec_copy_context被avcodec_parameters_to_context和avcodec_parameters_from_context所替代
        // avcodec_parameters_to_context。该函数用于将流里面的参数复制到AVCodecContext的上下文当中
        //
        // avcodec_parameters_from_context.将编解码上下文(AVCodecContext)中的所有相关参数
        //	（如编解码器ID、分辨率、比特率、像素格式等）复制到AVCodecParameters结构体中
        //
        //将输入流的参数 复制到 上下文中
        ret = avcodec_parameters_to_context(pCodec_ctx, in_Stream->codecpar);
        if (ret < 0)
        {
            avcodec_free_context(&pCodec_ctx);
            printf("Failed to copy in_stream codecpar to codec context\n");
            goto end;
        }
        pCodec_ctx->codec_tag = 0;
        if (pOfmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            pCodec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        ret = avcodec_parameters_from_context(out_stream->codecpar,pCodec_ctx);
        if (ret < 0)
        {
            avcodec_free_context(&pCodec_ctx);
            printf("Failed to copy codec context codecpar to out_stream\n");
            goto end;
        }

        avcodec_free_context(&pCodec_ctx);

    }
    av_dump_format(pOfmt_ctx, 0, out_filename, 1);

    //打开输出url
    if (!(pOfmt->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&pOfmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            printf("Could not open output URL '%s'", out_filename);
            goto end;
        }
    }
    ret = avformat_write_header(pOfmt_ctx,nullptr);
    if(ret < 0)
    {
        printf("Error occurred when opening output URL\n");
        goto end;
    }

    start_time = av_gettime();
    while (1)
    {
        AVStream* in_stream, *out_stream;
        pkt = av_packet_alloc();
        ret = av_read_frame(pIfmt_ctx, pkt);
        if(ret < 0)
            break;
        if (pkt->pts == AV_NOPTS_VALUE)
        {
            AVRational time_base = pIfmt_ctx->streams[videoindex]->time_base;
            int64_t calc_duration = (double)AV_TIME_BASE / av_q2d(pIfmt_ctx->streams[videoindex]->r_frame_rate);

            pkt->pts = (double)(frameindex * calc_duration) / (double)(av_q2d(time_base) * AV_TIME_BASE);
            pkt->dts = pkt->pts;
            pkt->duration = (double)calc_duration / (double)(av_q2d(time_base) * AV_TIME_BASE);

        }
        if (pkt->stream_index == videoindex)
        {
            AVRational time_base = pIfmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q = { 1,AV_TIME_BASE };

            int64_t pts_time = av_rescale_q(pkt->pts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);

        }

        in_stream = pIfmt_ctx->streams[pkt->stream_index];
        out_stream = pOfmt_ctx->streams[pkt->stream_index];

        //转换PTS/DTS（Convert PTS/DTS）
        pkt->pts = av_rescale_q_rnd(pkt->pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->dts = av_rescale_q_rnd(pkt->dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt->duration = av_rescale_q(pkt->duration, in_stream->time_base, out_stream->time_base);
        pkt->pos = -1;

        if (pkt->stream_index == videoindex) {
            printf("Send %8d video frames to output URL\n", frameindex);
            frameindex++;
        }
        ret = av_interleaved_write_frame(pOfmt_ctx, pkt);

        if (ret < 0) {
            printf("Error muxing packet\n");
            break;
        }

        av_packet_free(&pkt);

    }
    av_write_trailer(pOfmt_ctx);

end:
    avformat_close_input(&pIfmt_ctx);
    avformat_network_deinit();
    if (pOfmt_ctx && (pOfmt->flags & AVFMT_NOFILE))
        avio_close(pOfmt_ctx->pb);
    avformat_free_context(pOfmt_ctx);

    if (ret < 0 && ret != AVERROR_EOF)
    {
        printf("Error occurred.\n");
        return -1;
    }

    return 0;

}
