
#include "geo.h"

#define DEGREES_TO_RADIANS 0.017453292519943295f
#define RADIANS_TO_DEGREES 57.29577951308232f
#define PI 3.14159265358979323846f

#define EARTH_RADIUS   6371.0f
#define MARS_RADIUS    3390.0f
#define JUPITER_RADIUS 69911.0f

float toDegrees(float radians) {
    return radians * RADIANS_TO_DEGREES;
}

float toRadians(float degrees) {
    return degrees * DEGREES_TO_RADIANS;
}

float toDegreesFloat(float radians) {
    return radians * RADIANS_TO_DEGREES;
}

float getExpectedHeading(Location * location, Location * targetLocation) {
    float startLatitude = toRadians(location->latitude);
    float startLongitude = toRadians(location->longitude);
    float endLatitude = toRadians(targetLocation->latitude);
    float endLongitude = toRadians(targetLocation->longitude);
    float diffLongitude = endLongitude - startLongitude;
    float phi = log(tan(endLatitude/2.0f+PI/4.0f)/tan(startLatitude/2.0f+PI/4.0f));
    if (fabs(diffLongitude) > PI) {
        if (diffLongitude > 0.0f) {
            diffLongitude = -(2.0f * PI - diffLongitude);
        } else {
            diffLongitude = (2.0f * PI + diffLongitude);
        }
    }
    return fmod(toDegrees(atan2(diffLongitude, phi)) + 360.0, 360.0);
}

float getSteer(float heading, float expectedHeading) {
    float steer = ((expectedHeading - heading)/360.0f);

    if (steer < -0.5f) steer += 1.0f;
    if (steer > 0.5f) steer -= 1.0f;

    steer *= 2.0f; // angle bias - higher number for sharper turns
    float maxSteer = 0.7f;
    if (steer > maxSteer) steer = maxSteer;
    if (steer < 0.0f-maxSteer) steer = 0.0f-maxSteer;
    return steer;
}

float simulateCompass(float degrees, float steer) {
    degrees += (steer * 30.0);
    while (degrees >= 360) { degrees -= 360.0; }
    while (degrees < 0) { degrees += 360.0; }
    return degrees;
}

void getLocation(Location * location, float heading, float meters, Location * result) {
    float dr = meters / (EARTH_RADIUS*1000.0);
    float bearing = toRadians(heading);

    float locationLatitude = toRadians(location->latitude);
    float locationLongitude = toRadians(location->longitude);
    float sinDr = sin(dr);
    float cosDr = cos(dr);
    float sinLocLat = sin(locationLatitude);
    float cosLocLat = cos(locationLatitude);
    float latitude = asin(sinLocLat * cosDr + cosLocLat * sinDr * cos(bearing) );
    float longitude = locationLongitude + atan2(
            sin(bearing) * sinDr * cosLocLat,
            cosDr - sinLocLat * sin(latitude));
    result->latitude = toDegrees(latitude);
    result->longitude = toDegrees(longitude);
}

float distanceMeters(Location * location1, Location * location2) {
    float latDistance = toRadians(location2->latitude - location1->latitude);
    float lonDistance = toRadians(location2->longitude - location1->longitude);
    float sinLatD2 = sin(latDistance / 2.0f);
    float sinLonD2 = sin(lonDistance / 2.0f);
    float a = sinLatD2 * sinLatD2 +
               cos(toRadians(location1->latitude)) * cos(toRadians(location2->latitude))
               * sinLonD2 * sinLonD2;
    float c = 2.0f * atan2(sqrt(a), sqrt(1.0f - a));
    float distanceMeters =  EARTH_RADIUS * c * 1000.0f;
    return fabs(distanceMeters);
}

float distanceMetersHack(Location * location1, Location * location2) {
    float lon = location1->longitude - location2->longitude;
    float lat = location1->latitude - location2->latitude;
    if (lon < 0) lon = 0-lon;
    if (lat < 0) lat = 0-lat;
    return (lon > lat ? lon : lat) * 110000.0;
}
