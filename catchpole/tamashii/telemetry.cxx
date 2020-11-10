
#include "telemetry.h"

// Location

TelemetryType Location::getType() {
    return TelemetryType::LocationType;
}

const char * Location::getName() {
    return "location";
}

size_t Location::getSize() {
    return sizeof(Location);
}

void Location::writeJson(char * buffer) {
    sprintf(buffer, "{ \"latitude\": %.6f, \"longitude\": %.6f }", this->latitude, this->longitude);
}

// target

TelemetryType Target::getType() {
    return TelemetryType::TargetType;
}

const char * Target::getName() {
    return "target";
}

size_t Target::getSize() {
    return sizeof(Target);
}

void Target::writeJson(char * buffer) {
    sprintf(buffer, "{ \"location\": { \"latitude\": %.6f, \"longitude\": %.6f }, \"radiusMeters\": %.3f } }",
            this->location.latitude, this->location.longitude, this->radiusMeters);
}

// Heading

TelemetryType Heading::getType() {
    return TelemetryType::HeadingType;
}

size_t Heading::getSize() {
    return sizeof(Heading);
}

const char * Heading::getName() {
    return "heading";
}

void Heading::writeJson(char * buffer) {
    sprintf(buffer, "{ \"degrees\": %2.4f }", this->degrees);
}

// Steer

TelemetryType Steer::getType() {
    return TelemetryType::SteerType;
}

size_t Steer::getSize() {
    return sizeof(Steer);
}

const char * Steer::getName() {
    return "steer";
}

void Steer::writeJson(char * buffer) {
    sprintf(buffer, "{ \"steer\": %.5f }", this->steer);
}

// Thrust

TelemetryType Thrust::getType() {
    return TelemetryType::ThrustType;
}

size_t Thrust::getSize() {
    return sizeof(Thrust);
}

const char * Thrust::getName() {
    return "thrust";
}

void Thrust::writeJson(char * buffer) {
    sprintf(buffer, "{ \"powerLeft\": %.5f, \"powerRight\": %.5f }", this->powerLeft, this->powerRight);
}

// Time

Time::Time() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    setTime(ts);
}

Time::Time(timespec ts) {
    setTime(ts);
}

void Time::setTime(timespec ts) {
    this->time = (((long long)ts.tv_sec)*1000)+(ts.tv_nsec/1000000);
}

void Time::printMillis(char * buffer) {
    sprintf(buffer, "%lld", time);
}

TelemetryType Time::getType() {
    return TelemetryType::TimeType;
}

size_t Time::getSize() {
    return sizeof(Time);
}

const char * Time::getName() {
    return "time";
}

void Time::writeJson(char * buffer) {
    sprintf(buffer, "{ \"time\": %lld }", this->time);
}
