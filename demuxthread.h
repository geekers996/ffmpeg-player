#ifndef DEMUXTHREAD_H
#define DEMUXTHREAD_H
#include <iostream>
#include "thread.h"
#include "avpacketqueue.h"
#ifdef __cplusplus
extern "C" {
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
}
#endif

class DemuxThread : public Thread
{
public:
    DemuxThread(AVPacketQueue *audio_queue, AVPacketQueue *video_queue);
    virtual ~DemuxThread();
    int Init(const char *url);
    virtual int Start();
    virtual int Stop();
    virtual void Run();

    AVCodecParameters *AudioCodecParameters();
    AVCodecParameters *VideoCodecParameters();

    AVRational AudioStreamTimebase()
    {
        return ifmt_ctx_->streams[audio_stream_]->time_base;
    }

    AVRational VideoStreamTimebase()
    {
        return ifmt_ctx_->streams[video_stream_]->time_base;
    }
private:
    std::string url_;
    AVFormatContext *ifmt_ctx_ = NULL;
    char err2str[256] = {0};
    int audio_stream_ = -1;
    int video_stream_ = -1;
    AVPacketQueue *audio_queue_ = NULL;
    AVPacketQueue *video_queue_ = NULL;
};

#endif // DEMUXTHREAD_H
