
#include "console.h"
#include "thread.h"

static void * process_thread(void *arg) {
    ((Process*)arg)->run();
    return NULL;
}

Thread::Thread(size_t stackSize, Process * process) {
    pthread_attr_t attr;
    int status = pthread_attr_init(&attr);
    if (status != 0) {
        printf("pthread_attr_init failed, status=%d\n", status);
    }

    status = pthread_attr_setstacksize(&attr, stackSize);
    if (status != 0) {
        printf("pthread_attr_setstacksize failed, status=%d\n", status);
    }

    int prio_min = sched_get_priority_min(SCHED_FIFO);
    int prio_max = sched_get_priority_max(SCHED_FIFO);
    int prio_mid = (prio_min + prio_max) / 2;

    struct sched_param sparam;
    sparam.sched_priority = prio_mid;
    status = pthread_attr_setschedparam(&attr, &sparam);
    if (status != OK) {
        printf("pthread_attr_setschedparam failed, status=%d\n", status);
    }
    status = pthread_create(&this->pthread, &attr, process_thread, process);
    if (status != 0) {
        printf("pthread_create failed, status=%d\n", status);
    } else {
        console.log("Thread",NULL,"create",(char*)"priority", sparam.sched_priority);
    }
}

void Thread::join() {
    pthread_join(this->pthread, NULL);
}