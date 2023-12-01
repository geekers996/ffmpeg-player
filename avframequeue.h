#ifndef AVFRAMEQUEUE_H
#define AVFRAMEQUEUE_H
#include "queue.h"
#ifdef __cplusplus
extern "C" {
#include "libavcodec/avcodec.h"

}
#endif

class AVFrameQueue
{
public:
    AVFrameQueue();
    ~AVFrameQueue();
    void Abort();
    int Size();
    int Push(AVFrame *val);
    AVFrame *Pop(const int timeout);
    AVFrame *Front();
private:
    void release();
    Queue<AVFrame *> queue_;
};

#endif // AVFRAMEQUEUE_H
