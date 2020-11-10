
#include "console.h"
#include "gps.h"

Gps::Gps(char * deviceName) {
    this->device = deviceName;
    this->fd = open(deviceName, O_RDONLY);
    if (this->fd < 0) {
        printf("Unable to open GPS\n");
    }

    this->offsetLocation.latitude = -37.868384;
    this->offsetLocation.longitude = 144.968944;
}

void Gps::readLine() {
    int count = 0;
    char ch;
    do {
        read(fd, &ch, 1);
        if (ch != '\r' && ch != '\n' && count < MINMEA_MAX_LENGTH) {
            line[count++] = ch;
        }
    }
    while (ch != '\r' && ch != '\n');
    line[count] = '\0';
}

void Gps::findLocation(Location *location) {
    for (;;) {
        readLine();
        switch (minmea_sentence_id(this->line, false)) {
            case MINMEA_SENTENCE_RMC: {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, this->line)) {
                    float latitude = minmea_tocoord(&frame.latitude);
                    float longitude = minmea_tocoord(&frame.longitude);
                    if (!clockSet) {
                        struct timespec ts;
                        minmea_gettime(&ts, &frame.date, &frame.time);
                        if (ts.tv_sec > 0) {
                            clock_settime(CLOCK_REALTIME, &ts);

                            Time time = Time(ts);
                            char timeString[20];
                            time.printMillis(timeString);
                            console.log("Gps", device, "clockset", timeString);

                            // convert absolute position to offset by subtracting first real position
                            this->offsetLocation.latitude -= latitude;
                            this->offsetLocation.longitude -= longitude;
                            this->clockSet = true;
                        }
                    }
                    location->latitude = latitude + this->offsetLocation.latitude;
                    location->longitude = longitude + this->offsetLocation.longitude;

                    char buffer[50];
                    sprintf(buffer, "latitude %2.6f longitude %2.6f", location->latitude, location->longitude);
                    console.log("Gps",device,"RMC",(char*)buffer);
                    return;
                }
            }
                break;
            default:
                break;
        }
    }
}
