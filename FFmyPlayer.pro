QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console fpermissive

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS \
            __STDC_FORMAT_MACROS   \
#            D__STDC_FORMAT_MACROS

TRANSLATIONS += chinese.ts \
                english.ts

CODECFORTR = UTF-8
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += $$PWD/third/ffmpeg/include   \
               $$PWD/include    \
               $$PWD/test


DESTDIR += $$PWD/bin
TARGET = FFmyPlayer

win32
{
LIBS += -L$$PWD/third/ffmpeg/lib \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat \
        -lavutil \
        -lswresample \
        -lswscale
}
unix {
LIBS += \
    -lSDL2 \
    -lavcodec \
    -lavdevice \
    -lavfilter \
    -lavformat \
    -lavutil \
    -lswresample \
    -lswscale
}
SOURCES += \
    src/MainWindow.cpp \
    src/main.cpp \
    src/playerCtlButtons.cpp \
    src/playerCtlWidget.cpp \
    src/playingInfo.cpp \
    src/title_bar.cpp \
    src/playerWidget.cpp \
    test/avio_file.cpp \
    test/avio_reading.cpp \
    test/demuxer_test.cpp \
    test/demuxing_decoding.c \
    test/encode_video.cpp \
    test/filtering_Video.c \
    test/libavutiltest.cpp \
    test/remuxertest.cpp \
    test/live_test.cpp

HEADERS += \
    include/MainWindow.h \
    include/StyleSheet.h \
    include/playerCtlButtons.h \
    include/playingInfo.h \
    include/title_bar.h \
    include/playerWidget.h \
    include/playerCtlWidget.h \
    test/avio_file.h \
    test/avio_reading.h \
    test/demuxer_test.h \
    test/encode_video.h \
    test/filtering_Video.h \
    test/libavutiltest.h \
    test/remuxertest.h \
    test/live_test.h \
    third/ffmpeg/include/libavcodec/ac3_parser.h \
    third/ffmpeg/include/libavcodec/adts_parser.h \
    third/ffmpeg/include/libavcodec/avcodec.h \
    third/ffmpeg/include/libavcodec/avdct.h \
    third/ffmpeg/include/libavcodec/avfft.h \
    third/ffmpeg/include/libavcodec/bsf.h \
    third/ffmpeg/include/libavcodec/codec.h \
    third/ffmpeg/include/libavcodec/codec_desc.h \
    third/ffmpeg/include/libavcodec/codec_id.h \
    third/ffmpeg/include/libavcodec/codec_par.h \
    third/ffmpeg/include/libavcodec/d3d11va.h \
    third/ffmpeg/include/libavcodec/defs.h \
    third/ffmpeg/include/libavcodec/dirac.h \
    third/ffmpeg/include/libavcodec/dv_profile.h \
    third/ffmpeg/include/libavcodec/dxva2.h \
    third/ffmpeg/include/libavcodec/jni.h \
    third/ffmpeg/include/libavcodec/mediacodec.h \
    third/ffmpeg/include/libavcodec/packet.h \
    third/ffmpeg/include/libavcodec/qsv.h \
    third/ffmpeg/include/libavcodec/vdpau.h \
    third/ffmpeg/include/libavcodec/version.h \
    third/ffmpeg/include/libavcodec/version_major.h \
    third/ffmpeg/include/libavcodec/videotoolbox.h \
    third/ffmpeg/include/libavcodec/vorbis_parser.h \
    third/ffmpeg/include/libavcodec/xvmc.h \
    third/ffmpeg/include/libavdevice/avdevice.h \
    third/ffmpeg/include/libavdevice/version.h \
    third/ffmpeg/include/libavdevice/version_major.h \
    third/ffmpeg/include/libavfilter/avfilter.h \
    third/ffmpeg/include/libavfilter/buffersink.h \
    third/ffmpeg/include/libavfilter/buffersrc.h \
    third/ffmpeg/include/libavfilter/version.h \
    third/ffmpeg/include/libavfilter/version_major.h \
    third/ffmpeg/include/libavformat/avformat.h \
    third/ffmpeg/include/libavformat/avio.h \
    third/ffmpeg/include/libavformat/version.h \
    third/ffmpeg/include/libavformat/version_major.h \
    third/ffmpeg/include/libavutil/adler32.h \
    third/ffmpeg/include/libavutil/aes.h \
    third/ffmpeg/include/libavutil/aes_ctr.h \
    third/ffmpeg/include/libavutil/ambient_viewing_environment.h \
    third/ffmpeg/include/libavutil/attributes.h \
    third/ffmpeg/include/libavutil/audio_fifo.h \
    third/ffmpeg/include/libavutil/avassert.h \
    third/ffmpeg/include/libavutil/avconfig.h \
    third/ffmpeg/include/libavutil/avstring.h \
    third/ffmpeg/include/libavutil/avutil.h \
    third/ffmpeg/include/libavutil/base64.h \
    third/ffmpeg/include/libavutil/blowfish.h \
    third/ffmpeg/include/libavutil/bprint.h \
    third/ffmpeg/include/libavutil/bswap.h \
    third/ffmpeg/include/libavutil/buffer.h \
    third/ffmpeg/include/libavutil/camellia.h \
    third/ffmpeg/include/libavutil/cast5.h \
    third/ffmpeg/include/libavutil/channel_layout.h \
    third/ffmpeg/include/libavutil/common.h \
    third/ffmpeg/include/libavutil/cpu.h \
    third/ffmpeg/include/libavutil/crc.h \
    third/ffmpeg/include/libavutil/csp.h \
    third/ffmpeg/include/libavutil/des.h \
    third/ffmpeg/include/libavutil/detection_bbox.h \
    third/ffmpeg/include/libavutil/dict.h \
    third/ffmpeg/include/libavutil/display.h \
    third/ffmpeg/include/libavutil/dovi_meta.h \
    third/ffmpeg/include/libavutil/downmix_info.h \
    third/ffmpeg/include/libavutil/encryption_info.h \
    third/ffmpeg/include/libavutil/error.h \
    third/ffmpeg/include/libavutil/eval.h \
    third/ffmpeg/include/libavutil/ffversion.h \
    third/ffmpeg/include/libavutil/fifo.h \
    third/ffmpeg/include/libavutil/file.h \
    third/ffmpeg/include/libavutil/film_grain_params.h \
    third/ffmpeg/include/libavutil/frame.h \
    third/ffmpeg/include/libavutil/hash.h \
    third/ffmpeg/include/libavutil/hdr_dynamic_metadata.h \
    third/ffmpeg/include/libavutil/hdr_dynamic_vivid_metadata.h \
    third/ffmpeg/include/libavutil/hmac.h \
    third/ffmpeg/include/libavutil/hwcontext.h \
    third/ffmpeg/include/libavutil/hwcontext_cuda.h \
    third/ffmpeg/include/libavutil/hwcontext_d3d11va.h \
    third/ffmpeg/include/libavutil/hwcontext_drm.h \
    third/ffmpeg/include/libavutil/hwcontext_dxva2.h \
    third/ffmpeg/include/libavutil/hwcontext_mediacodec.h \
    third/ffmpeg/include/libavutil/hwcontext_opencl.h \
    third/ffmpeg/include/libavutil/hwcontext_qsv.h \
    third/ffmpeg/include/libavutil/hwcontext_vaapi.h \
    third/ffmpeg/include/libavutil/hwcontext_vdpau.h \
    third/ffmpeg/include/libavutil/hwcontext_videotoolbox.h \
    third/ffmpeg/include/libavutil/hwcontext_vulkan.h \
    third/ffmpeg/include/libavutil/imgutils.h \
    third/ffmpeg/include/libavutil/intfloat.h \
    third/ffmpeg/include/libavutil/intreadwrite.h \
    third/ffmpeg/include/libavutil/lfg.h \
    third/ffmpeg/include/libavutil/log.h \
    third/ffmpeg/include/libavutil/lzo.h \
    third/ffmpeg/include/libavutil/macros.h \
    third/ffmpeg/include/libavutil/mastering_display_metadata.h \
    third/ffmpeg/include/libavutil/mathematics.h \
    third/ffmpeg/include/libavutil/md5.h \
    third/ffmpeg/include/libavutil/mem.h \
    third/ffmpeg/include/libavutil/motion_vector.h \
    third/ffmpeg/include/libavutil/murmur3.h \
    third/ffmpeg/include/libavutil/opt.h \
    third/ffmpeg/include/libavutil/parseutils.h \
    third/ffmpeg/include/libavutil/pixdesc.h \
    third/ffmpeg/include/libavutil/pixelutils.h \
    third/ffmpeg/include/libavutil/pixfmt.h \
    third/ffmpeg/include/libavutil/random_seed.h \
    third/ffmpeg/include/libavutil/rational.h \
    third/ffmpeg/include/libavutil/rc4.h \
    third/ffmpeg/include/libavutil/replaygain.h \
    third/ffmpeg/include/libavutil/ripemd.h \
    third/ffmpeg/include/libavutil/samplefmt.h \
    third/ffmpeg/include/libavutil/sha.h \
    third/ffmpeg/include/libavutil/sha512.h \
    third/ffmpeg/include/libavutil/spherical.h \
    third/ffmpeg/include/libavutil/stereo3d.h \
    third/ffmpeg/include/libavutil/tea.h \
    third/ffmpeg/include/libavutil/threadmessage.h \
    third/ffmpeg/include/libavutil/time.h \
    third/ffmpeg/include/libavutil/timecode.h \
    third/ffmpeg/include/libavutil/timestamp.h \
    third/ffmpeg/include/libavutil/tree.h \
    third/ffmpeg/include/libavutil/twofish.h \
    third/ffmpeg/include/libavutil/tx.h \
    third/ffmpeg/include/libavutil/uuid.h \
    third/ffmpeg/include/libavutil/version.h \
    third/ffmpeg/include/libavutil/video_enc_params.h \
    third/ffmpeg/include/libavutil/xtea.h \
    third/ffmpeg/include/libpostproc/postprocess.h \
    third/ffmpeg/include/libpostproc/version.h \
    third/ffmpeg/include/libpostproc/version_major.h \
    third/ffmpeg/include/libswresample/swresample.h \
    third/ffmpeg/include/libswresample/version.h \
    third/ffmpeg/include/libswresample/version_major.h \
    third/ffmpeg/include/libswscale/swscale.h \
    third/ffmpeg/include/libswscale/version.h \
    third/ffmpeg/include/libswscale/version_major.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

