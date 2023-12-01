#ifndef AVSYNC_H
#define AVSYNC_H
#include <chrono>
#include <ctime>
#include <math.h>
using namespace std::chrono;

class AVSync
{
public:
    AVSync()
    {
    }
    ~AVSync()
    {
    }

    void InitClock()
    {
        SetClock(NAN);
    }
    // 由audio pts获得
    void SetClock(double pts)
    {
        double time = GetMicroseconds() / 1000000.0; //秒
        pts_drift_ = pts - time;
    }
    // 由video 刷新读取
    double GetClock()
    {
        double time = GetMicroseconds() / 1000000.0;
        return pts_drift_ + time;
    }

    // 微秒的单位
    time_t GetMicroseconds()
    {
        system_clock::time_point time_point_new = system_clock::now();
        system_clock::duration duration = time_point_new.time_since_epoch();
        time_t us = duration_cast<microseconds>(duration).count();
        return us;
    }
    double pts_drift_ = 0;
};

#endif // AVSYNC_H
