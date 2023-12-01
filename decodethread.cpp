#include "decodethread.h"

DecodeThread::DecodeThread(AVPacketQueue *packet_queue, AVFrameQueue  *frame_queue):
    packet_queue_(packet_queue), frame_queue_(frame_queue)
{
}

DecodeThread::~DecodeThread()
{
}

int DecodeThread::Init(AVCodecParameters *par)
{
    if(!par) {
        printf("DecodeThread::Init par is NULL\n");
        return -1;
    }
    codec_ctx_ = avcodec_alloc_context3(NULL);
    int ret = avcodec_parameters_to_context(codec_ctx_, par);
    //    avcodec_parameters_from_context // 合成复用的时候用
    if(ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        printf("avcodec_parameters_to_context failed, ret:%d, err2str:%s", ret, err2str);
        return -1;
    }
    const AVCodec *codec = avcodec_find_decoder(codec_ctx_->codec_id);
    if(!codec) {
        printf("avcodec_find_decoder failed\n");
        return -1;
    }
    ret = avcodec_open2(codec_ctx_, codec, NULL);
    if(ret < 0) {
        av_strerror(ret, err2str, sizeof(err2str));
        printf("avcodec_open2 failed, ret:%d, err2str:%s", ret, err2str);
        return -1;
    }
    printf("Init decode finish\n");
    return 0;
}

int DecodeThread::Start()
{
    thread_ = new std::thread(&DecodeThread::Run, this);
    if(!thread_) {
        printf("new DecodeThread failed\n");
        return -1;
    }
    return 0;
}

int DecodeThread::Stop()
{
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    Thread::Stop();
    return 0;
}

void DecodeThread::Run()
{
    int ret = 0;
    AVFrame *frame = av_frame_alloc();
    while(1) {
        if(abort_ == 1) {
            break;
        }
        // 限制帧队列长度
        if(frame_queue_->Size() > 10) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        // 从packet_queue读取packet
        AVPacket *packet = packet_queue_->Pop(10);
        if(packet) {
            // 送给解码器
            ret = avcodec_send_packet(codec_ctx_, packet);
            av_packet_free(&packet);
            if(ret < 0) {
                av_strerror(ret, err2str, sizeof(err2str));
                printf("avcodec_send_packet failed, ret:%d, err2str:%s", ret, err2str);
                break;
            }
            // 从解码器读取frame
            while (true) {
                ret = avcodec_receive_frame(codec_ctx_, frame);  // 存在B帧的场景  B3-2  P2-3   I1-1 --> P3  B2  I1
                if(ret == 0) {
                    frame_queue_->Push(frame);
                    printf("%s frame_queue size:%d\n ", codec_ctx_->codec->name, frame_queue_->Size());
                    continue;
                } else if(ret == AVERROR(EAGAIN)) {
                    break;
                } else {
                    abort_  = 1;
                    av_strerror(ret, err2str, sizeof(err2str));
                    printf("avcodec_receive_frame failed, ret:%d, err2str:%s", ret, err2str);
                    break;
                }
            }
            // 把frame发送给framequeue
        } else {
            printf("no packet\n");
        }
    }
}
