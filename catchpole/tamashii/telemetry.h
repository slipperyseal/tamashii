
#ifndef TELEMERTY_H
#define TELEMERTY_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <time.h>

enum TelemetryType {
    HeadingType, LocationType, TargetType, SteerType, ThrustType, TimeType
};

class Telemetry {
public:
    void setTime();
    virtual TelemetryType getType() = 0;
    virtual const char * getName() = 0;
    virtual size_t getSize() = 0;
    virtual void writeJson(char * buffer) = 0;
};

class Heading : public Telemetry {
public:
    float degrees;

    virtual TelemetryType getType();
    virtual const char * getName();
    virtual size_t getSize();
    virtual void writeJson(char * buffer);
};

class Location : public Telemetry {
public:
    float latitude;
    float longitude;

    virtual TelemetryType getType();
    virtual const char * getName();
    virtual size_t getSize();
    virtual void writeJson(char * buffer);
};

class Target : public Telemetry {
public:
    Location location;
    float radiusMeters;

    virtual TelemetryType getType();
    virtual const char * getName();
    virtual size_t getSize();
    virtual void writeJson(char * buffer);
};

class Steer : public Telemetry {
public:
    float steer;

    virtual TelemetryType getType();
    virtual const char * getName();
    virtual size_t getSize();
    virtual void writeJson(char * buffer);
};

class Thrust : public Telemetry {
public:
    float powerLeft;
    float powerRight;

    virtual TelemetryType getType();
    virtual const char * getName();
    virtual size_t getSize();
    virtual void writeJson(char * buffer);
};

class Time : public Telemetry {
public:
    long long time;
protected:
    void setTime(timespec ts);

public:
    Time();
    Time(timespec ts);

    void printMillis(char * buffer);

    virtual TelemetryType getType();
    virtual const char * getName();
    virtual size_t getSize();
    virtual void writeJson(char * buffer);
};

#endif