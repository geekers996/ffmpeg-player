#include "demuxthread.h"

DemuxThread::DemuxThread(AVPacketQueue *audio_queue, AVPacketQueue *video_queue):
    audio_queue_(audio_queue), video_queue_(video_queue)
{
    printf("DemuxThread\n");
}

DemuxThread::~DemuxThread()
{
    printf("~DemuxThread\n");
}

int DemuxThread::Init(const char *url)
{
    if(!url) {
        printf("%s(%d) url is null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    if(!audio_queue_ || !video_queue_) {
        printf("%s(%d) audio_queue_ or video_queue_  null\n", __FUNCTION__, __LINE__);
        return -1;
    }
    url_ = url;
    ifmt_ctx_ = avformat_alloc_context();
    int ret = avformat_open_input(&ifmt_ctx_, url_.c_str(), NULL, NULL);
    if(ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        printf("%s(%d) avformat_open_input failed:%d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
        return -1;
    }
    ret = avformat_find_stream_info(ifmt_ctx_, NULL);
    if(ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        printf("%s(%d) avformat_find_stream_info failed:%d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
        return -1;
    }
    av_dump_format(ifmt_ctx_, 0, url_.c_str(), 0);
    audio_stream_ = av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    video_stream_ = av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    printf("%s(%d) audio_stream_:%d, video_stream_:%d\n", __FUNCTION__, __LINE__, audio_stream_, video_stream_);
    if(audio_stream_ < 0 || video_stream_) {
        printf("no audio  or no video\n");
        return -1;
    }
    return 0;
}

int DemuxThread::Start()
{
    thread_ = new std::thread(&DemuxThread::Run, this);
    if(!thread_) {
        printf("new DemuxThread failed\n");
        return -1;
    }
    return 0;
}

int DemuxThread::Stop()
{
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    Thread::Stop();
    return 0;
}

void DemuxThread::Run()
{
    printf("DemuxThread::Run() into\n");
    AVPacket packet;
    int ret = 0;
    while(1) {
        if(abort_ == 1) {
            break;
        }
        if(audio_queue_->Size() > 100 || video_queue_->Size() > 100) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        ret = av_read_frame(ifmt_ctx_, &packet);
        if(ret < 0) {
            av_strerror(ret, err2str, sizeof(err2str));
            printf("%s(%d) av_read_frame failed:%d, %s\n", __FUNCTION__, __LINE__, ret, err2str);
            break;
        }
        if(packet.stream_index == audio_stream_) {  // 音频包队列
            audio_queue_->Push(&packet);
            printf("audio pkt size:%d\n", audio_queue_->Size());
        } else if(packet.stream_index == video_stream_) {  // 视频包队列
            video_queue_->Push(&packet);
            printf("video pkt size:%d\n", video_queue_->Size());
        } else {
            av_packet_unref(&packet);
        }
    }
    avformat_close_input(&ifmt_ctx_);
    printf("DemuxThread::Run() leave\n");
}

AVCodecParameters *DemuxThread::AudioCodecParameters()
{
    if(audio_stream_ != -1) {
        return ifmt_ctx_->streams[audio_stream_]->codecpar;
    } else {
        return NULL;
    }
}

AVCodecParameters *DemuxThread::VideoCodecParameters()
{
    if(video_stream_ != -1) {
        return ifmt_ctx_->streams[video_stream_]->codecpar;
    } else {
        return NULL;
    }
}
