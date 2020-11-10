
#include <nuttx/config.h>
#include <cstdio>
#include <debug.h>
#include <nuttx/init.h>
#include "platform/cxxinitialize.h"

#include <nuttx/mqueue.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <syslog.h>

#include "console.h"
#include "queue.h"

Queue::Queue(char * name, int messageSize, int maxMessages) {
    this->queueName = name;
    this->attr.mq_maxmsg  = maxMessages;
    this->attr.mq_msgsize = messageSize;
    this->attr.mq_flags   = 0;
}

char * Queue::getQueueName() {
    return this->queueName;
}

mq_attr * Queue::getAttributes() {
    return &attr;
}

QueueSender::QueueSender(Queue * q) {
    this->queue = q;
    this->fd = mq_open(this->queue->getQueueName(), O_WRONLY|O_CREAT, 0666, this->queue->getAttributes());
    if (this->fd == (mqd_t)-1) {
        printf("ERROR mq_open failed\n");
    }
}

bool QueueSender::send(char * message, int size) {
    int status = mq_send(this->fd, message, size, 42);
    if (status < 0) {
        console.log("Queue",this->queue->getQueueName(),"send",(char*)"fail:", status);
        return false;
    }
    console.log("Queue",this->queue->getQueueName(),"send",(char*)"bytes", size);
    return true;
}

void QueueSender::close() {
    if (mq_close(fd) < 0) {
        printf("ERROR mq_close failed\n");
    }
    fd = NULL;
}

QueueReceiver::QueueReceiver(Queue *q) {
    this->queue = q;
    this->fd = mq_open(this->queue->getQueueName(), O_RDONLY|O_CREAT, 0666, queue->getAttributes());
    if (this->fd < 0) {
        printf("receiver_thread: ERROR mq_open failed\n");
    }
}

int QueueReceiver::receive(char * message, int len) {
    int size = mq_receive(this->fd, message, len, 0);
    if (size < 0) {
        // mq_receive failed.  If the error is because of EINTR then it is not a failure.
        if (errno != EINTR) {
            console.log("Queue", this->queue->getQueueName(), "receive",(char*)"fail:", errno);
        }
        return 0;
    }
    console.log("Queue", this->queue->getQueueName(),"receive",(char*)"bytes", size);
    return size;
}

void QueueReceiver::close() {
    if (mq_close(this->fd) < 0) {
        printf("ERROR mq_close failed\n");
    }
    fd = NULL;
}

