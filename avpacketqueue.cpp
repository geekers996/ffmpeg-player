#include "avpacketqueue.h"

AVPacketQueue::AVPacketQueue()
{
}

AVPacketQueue::~AVPacketQueue()
{
}

void AVPacketQueue::Abort()
{
    // 先释放队列里的资源
    release();
    queue_.Abort();
}

int AVPacketQueue::Size()
{
    return queue_.Size();
}

int AVPacketQueue::Push(AVPacket *val)
{
    AVPacket *tmp_pkt = av_packet_alloc();
    av_packet_move_ref(tmp_pkt, val);
    return queue_.Push(tmp_pkt);
}

AVPacket *AVPacketQueue::Pop(const int timeout)
{
    AVPacket *tmp_pkt = NULL;
    int ret = queue_.Pop(tmp_pkt, timeout);
    if(ret < 0) {
        if(ret == -1) {
            printf("queue_ abort\n ");
        }
    }
    return tmp_pkt;
}

void AVPacketQueue::release()
{
    while(true) {
        AVPacket *tmp_pkt = NULL;
        int ret = queue_.Pop(tmp_pkt, 1);
        if(ret < 0) {
            break;
        } else {
            av_packet_free(&tmp_pkt);
            continue;
        }
    }
}
