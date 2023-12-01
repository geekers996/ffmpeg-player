#ifndef VIDEOOUTPUT_H
#define VIDEOOUTPUT_H

#include "avframequeue.h"
#include "AVSync.h"
#ifdef __cplusplus  ///
extern "C"
{
#include "SDL.h"
}
#endif
class VideoOutput
{
public:
    VideoOutput(AVSync *avsync, AVFrameQueue *frame_queue, int video_width, int video_height,  AVRational time_base);
    ~VideoOutput();
    int Init();
    int MainLoop();
    void RefreshLoopWaitEvent(SDL_Event *event);
private:
    void videoRefresh(double &remain_time);
    AVFrameQueue *frame_queue_ = NULL;
    SDL_Window *win_  = NULL;
    SDL_Renderer *renderer_  = NULL;
    SDL_Texture *texture_  = NULL;

    int video_width_ = 0;
    int video_height_ = 0;
    AVRational time_base_ ;
    AVSync *avsync_ = NULL;
};

#endif // VIDEOOUTPUT_H
