
#ifndef TAMASHI_H
#define TAMASHI_H

#include <nuttx/config.h>
#include <cstdio>
#include <debug.h>
#include <nuttx/init.h>
#include "platform/cxxinitialize.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <syslog.h>

#include "gps.h"
#include "queue.h"
#include "esp.h"
#include "thread.h"
#include "console.h"
#include "geo.h"

// used to find the max size of any telemetry object
union TelemetryUnion {
    Location location;
    Target target;
    Heading heading;
    Steer steer;
    Thrust thrust;
};

class Snapshot {
public:
    Location location;
    Heading heading;
};

class SensorProcess : public Process {
private:
    Queue * telemetryQueue;
public:
    Heading heading;
    SensorProcess(Queue * telemetryQ);
    virtual void run();
};

class EspProcess : public Process {
private:
    Queue * sourceQueue;
    char packet[128];
public:
    EspProcess(Queue * broadcastQueue);
    virtual void run();
};

class RoutingProcess : public Process {
private:
    Queue * telemetryQueue;
    Queue * broadcastQueue;
    Queue * snapshotQueue;
    Snapshot snapshot;
public:
    RoutingProcess(Queue * telemetryQ, Queue *broadcastQ, Queue * snapshotQ);
    virtual void run();
};

class AutoPilotProcess : public Process {
private:
    Queue * snapshotQueue;
    Queue * broadcastQueue;
    Gps * gps;
    Heading * compass;

    Target target[3];
    int targetIndex;
    bool locationLock = false;
public:
    AutoPilotProcess(Queue * snapshot, Queue * broadcast, Gps * g, Heading * heading);
    virtual void run();
};

class Tamashii : public Process {
private:
    Queue * telemetryQ;
    Gps * gps;

public:
    Tamashii();
    virtual void run();
};

class FileRead {
private:
    char * device;
    int fd;
public:
    char line[128];

    FileRead(char * deviceName);
    void readLine();
    void closeFile();
};

#endif