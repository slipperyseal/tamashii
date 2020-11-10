
#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <syslog.h>
#include <pthread.h>

class Process {
protected:
    volatile bool running=true;
public:
    virtual void run()=0;
};

class Thread {
private:
    pthread_t pthread;

public:
    Thread(size_t stackSize, Process * process);
    void join();
};

#endif
