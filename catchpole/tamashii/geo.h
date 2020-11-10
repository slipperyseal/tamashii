
#ifndef GEO_H
#define GEO_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>
#include <math.h>

#include "telemetry.h"

float getExpectedHeading(Location * location, Location * targetLocation);
float getSteer(float heading, float newHeading);
void getLocation(Location * location, float heading, float meters, Location * result);
float distanceMeters(Location * location1, Location * location2);
float simulateCompass(float degrees, float steer);

#endif