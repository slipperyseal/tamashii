
#ifndef GPS_H
#define GPS_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include "gpsutils/minmea.h"
#include <time.h>
#include "telemetry.h"

class Gps {
private:
    char * device;
    int fd;
    char line[MINMEA_MAX_LENGTH+1];
    bool clockSet = false;

public:
    Location offsetLocation;

    Gps(char * deviceName);
    void findLocation(Location * location);
private:
    void readLine();
};

#endif