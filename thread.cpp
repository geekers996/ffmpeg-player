#include "thread.h"
#include <stdio.h>
Thread::Thread()
{
}

Thread::~Thread()
{
}

int Thread::Start()
{
    return 0;
}

int Thread::Stop()
{
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    abort_ = 1;
    if(thread_) {
        thread_->join();
        delete thread_;
        thread_ = nullptr;
    }
    return 0;
}
