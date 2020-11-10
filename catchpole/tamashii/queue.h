
#ifndef QUEUE_H
#define QUEUE_H

#include <nuttx/mqueue.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <syslog.h>

class Queue {
private:
    char * queueName;
    struct mq_attr attr;
public:

    Queue(char * name, int messageSize, int maxMessages);

    char * getQueueName();
    mq_attr * getAttributes();
};

class QueueSender {
private:
    Queue * queue;
    mqd_t fd;
public:
    QueueSender(Queue * q);

    bool send(char * data, int len);
    void close();
};

class QueueReceiver {
private:
    Queue * queue;
    mqd_t fd;
public:
    QueueReceiver(Queue * q);

    int receive(char * data, int len);
    void close();
};

#endif