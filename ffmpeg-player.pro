TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        audiooutput.cpp \
        avframequeue.cpp \
        avpacketqueue.cpp \
        decodethread.cpp \
        demuxthread.cpp \
        main.cpp \
        thread.cpp \
        videooutput.cpp


win32 {
INCLUDEPATH += $$PWD/ffmpeg-n6.0-22/include
LIBS += $$PWD/ffmpeg-n6.0-22/lib/avformat.lib   \
        $$PWD/ffmpeg-n6.0-22/lib/avcodec.lib    \
        $$PWD/ffmpeg-n6.0-22/lib/avdevice.lib   \
        $$PWD/ffmpeg-n6.0-22/lib/avfilter.lib   \
        $$PWD/ffmpeg-n6.0-22/lib/avutil.lib     \
        $$PWD/ffmpeg-n6.0-22/lib/postproc.lib   \
        $$PWD/ffmpeg-n6.0-22/lib/swresample.lib \
        $$PWD/ffmpeg-n6.0-22/lib/swscale.lib
INCLUDEPATH += $$PWD/SDL2-2.0.10/include
LIBS += $$PWD/SDL2-2.0.10/lib/x64/SDL2.lib
}

HEADERS += \
    AVSync.h \
    audiooutput.h \
    avframequeue.h \
    avpacketqueue.h \
    decodethread.h \
    demuxthread.h \
    queue.h \
    thread.h \
    videooutput.h
