QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TRANSLATIONS += chinese.ts \
                english.ts

CODECFORTR = UTF-8

TARGET = FFmyPlayer

INCLUDEPATH += $$PWD/third/ffmpeg/include   \
               $$PWD/third/SDL2/include     \
               $$PWD/third/boost/include    \
               $$PWD/include                \
               $$PWD/test

DESTDIR += $$PWD/bin

win32
{
LIBS += -L$$PWD/third/ffmpeg/lib \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat \
        -lavutil \
        -lswresample \
        -lswscale   \
        -L$$PWD/third/SDL2/lib/x64  \
        -lSDL2  \
        -L$$PWD/third/boost/lib/    \
        -lboost_system  \
        -lboost_filesystem
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
    -lswscale   \
    -lSDL2  \
    -lboost_system  \
    -lboost_filesystem
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/Common.cpp \
    src/MainWindow.cpp \
    src/main.cpp \
    src/playerCtlButtons.cpp \
    src/playerCtlWidget.cpp \
    src/playerDisplay.cpp \
    src/playerManager.cpp \
    src/playerWidget.cpp \
    src/playingInfo.cpp \
    src/title_bar.cpp


HEADERS += \
    include/Common.h \
    include/MainWindow.h \
    include/StyleSheet.h \
    include/configurationFile.h \
    include/dataStruct.h \
    include/playerCtlButtons.h \
    include/playerCtlWidget.h \
    include/playerDisplay.h \
    include/playerManager.h \
    include/playerWidget.h \
    include/playingInfo.h \
    include/title_bar.h \
    third/SDL2/include/SDL.h \
    third/SDL2/include/SDL_assert.h \
    third/SDL2/include/SDL_atomic.h \
    third/SDL2/include/SDL_audio.h \
    third/SDL2/include/SDL_bits.h \
    third/SDL2/include/SDL_blendmode.h \
    third/SDL2/include/SDL_clipboard.h \
    third/SDL2/include/SDL_config.h \
    third/SDL2/include/SDL_cpuinfo.h \
    third/SDL2/include/SDL_egl.h \
    third/SDL2/include/SDL_endian.h \
    third/SDL2/include/SDL_error.h \
    third/SDL2/include/SDL_events.h \
    third/SDL2/include/SDL_filesystem.h \
    third/SDL2/include/SDL_gamecontroller.h \
    third/SDL2/include/SDL_gesture.h \
    third/SDL2/include/SDL_guid.h \
    third/SDL2/include/SDL_haptic.h \
    third/SDL2/include/SDL_hidapi.h \
    third/SDL2/include/SDL_hints.h \
    third/SDL2/include/SDL_joystick.h \
    third/SDL2/include/SDL_keyboard.h \
    third/SDL2/include/SDL_keycode.h \
    third/SDL2/include/SDL_loadso.h \
    third/SDL2/include/SDL_locale.h \
    third/SDL2/include/SDL_log.h \
    third/SDL2/include/SDL_main.h \
    third/SDL2/include/SDL_messagebox.h \
    third/SDL2/include/SDL_metal.h \
    third/SDL2/include/SDL_misc.h \
    third/SDL2/include/SDL_mouse.h \
    third/SDL2/include/SDL_mutex.h \
    third/SDL2/include/SDL_name.h \
    third/SDL2/include/SDL_opengl.h \
    third/SDL2/include/SDL_opengl_glext.h \
    third/SDL2/include/SDL_opengles.h \
    third/SDL2/include/SDL_opengles2.h \
    third/SDL2/include/SDL_opengles2_gl2.h \
    third/SDL2/include/SDL_opengles2_gl2ext.h \
    third/SDL2/include/SDL_opengles2_gl2platform.h \
    third/SDL2/include/SDL_opengles2_khrplatform.h \
    third/SDL2/include/SDL_pixels.h \
    third/SDL2/include/SDL_platform.h \
    third/SDL2/include/SDL_power.h \
    third/SDL2/include/SDL_quit.h \
    third/SDL2/include/SDL_rect.h \
    third/SDL2/include/SDL_render.h \
    third/SDL2/include/SDL_revision.h \
    third/SDL2/include/SDL_rwops.h \
    third/SDL2/include/SDL_scancode.h \
    third/SDL2/include/SDL_sensor.h \
    third/SDL2/include/SDL_shape.h \
    third/SDL2/include/SDL_stdinc.h \
    third/SDL2/include/SDL_surface.h \
    third/SDL2/include/SDL_system.h \
    third/SDL2/include/SDL_syswm.h \
    third/SDL2/include/SDL_test.h \
    third/SDL2/include/SDL_test_assert.h \
    third/SDL2/include/SDL_test_common.h \
    third/SDL2/include/SDL_test_compare.h \
    third/SDL2/include/SDL_test_crc32.h \
    third/SDL2/include/SDL_test_font.h \
    third/SDL2/include/SDL_test_fuzzer.h \
    third/SDL2/include/SDL_test_harness.h \
    third/SDL2/include/SDL_test_images.h \
    third/SDL2/include/SDL_test_log.h \
    third/SDL2/include/SDL_test_md5.h \
    third/SDL2/include/SDL_test_memory.h \
    third/SDL2/include/SDL_test_random.h \
    third/SDL2/include/SDL_thread.h \
    third/SDL2/include/SDL_timer.h \
    third/SDL2/include/SDL_touch.h \
    third/SDL2/include/SDL_types.h \
    third/SDL2/include/SDL_version.h \
    third/SDL2/include/SDL_video.h \
    third/SDL2/include/SDL_vulkan.h \
    third/SDL2/include/begin_code.h \
    third/SDL2/include/close_code.h \
    third/boost/include/boost/property_tree/ini_parser.hpp \
    third/boost/include/boost/property_tree/ptree.hpp \
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
