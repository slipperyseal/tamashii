//***************************************************************************
// catchpole/tamashii/tamashii_main.cxx
//***************************************************************************

#include "tamashii.h"

#if !defined(CONFIG_HAVE_CXX) || !defined(CONFIG_HAVE_CXXINITIALIZE)
#  undef CONFIG_CATCHPOLE_TAMASHII_CXXINITIALIZE
#endif

// /dev/ttyS0 UART2  nsh         TX/RX PA2/PA3
// /dev/ttyS1 UART5  sat/wifi    TX/RX PC12/PD2
// /dev/ttyS2 USART6 GPS         TX/RX? PC6/PC7

#define ESP_TTY         "/dev/ttyS1"
#define GPS_TTY         "/dev/ttyS2"

void startMesage() {
    Time time;
    char buffer[60];
    sprintf(buffer, "time: %lld max headroom: %d", time.time, (int)sizeof(Snapshot));
    console.log("Tamashii","NYX","start", buffer);
}

void cpuCheck() {
    FileRead fileRead = FileRead((char *)"/proc/cpuload");
    fileRead.readLine();
    console.log("Tamashii", NULL, "cpu", fileRead.line);
    fileRead.closeFile();
}

void memCheck() {
    FileRead fileRead = FileRead((char *)"/proc/meminfo");
    fileRead.readLine();
    fileRead.readLine();
    console.log("Tamashii", NULL, "memory", fileRead.line);
    fileRead.closeFile();
}

FileRead::FileRead(char * deviceName) {
    this->fd = open(deviceName, O_RDONLY);
    if (this->fd < 0) {
        printf("Unable to open %s\n", deviceName);
    }
}

void FileRead::readLine() {
    int count = 0;
    char ch;
    do {
        read(fd, &ch, 1);
        if (ch != '\r' && ch != '\n' && count < (int)sizeof(line)-1) {
            line[count++] = ch;
        }
    }
    while (ch != '\r' && ch != '\n');
    line[count] = '\0';
}

void FileRead::closeFile() {
    close(this->fd);
}

SensorProcess::SensorProcess(Queue * telemetryQ) {
    this->telemetryQueue = telemetryQ;
}

void SensorProcess::run() {
    QueueSender queueSender = QueueSender(this->telemetryQueue);
    while (this->running) {
        queueSender.send((char*)&heading, sizeof(Heading));
        usleep(250000);
    }
}

EspProcess::EspProcess(Queue * broadcastQueue) {
    this->sourceQueue = broadcastQueue;
}

void EspProcess::run() {
    QueueReceiver queueReceiver = QueueReceiver(this->sourceQueue);

    EspSender espSender = EspSender(ESP_TTY);
    espSender.init();

    while (this->running) {
        char telemetryBuffer[sizeof(TelemetryUnion)];
        if ((queueReceiver.receive((char*)&telemetryBuffer, sizeof(TelemetryUnion))) != 0) {
            Telemetry * telemetry = (Telemetry *)&telemetryBuffer[0];

            // send Time whenever we send a Location
            if (telemetry->getType() == TelemetryType::LocationType) {
                Time time = Time();
                time.writeJson(this->packet);
                espSender.sendPacket((char *) this->packet);
            }

            telemetry->writeJson(this->packet);
            espSender.sendPacket((char *) this->packet);
        }
    }
}

RoutingProcess::RoutingProcess(Queue * telemetryQ, Queue *broadcastQ, Queue * snapshotQ) {
    this->telemetryQueue = telemetryQ;
    this->broadcastQueue = broadcastQ;
    this->snapshotQueue= snapshotQ;
}

void RoutingProcess::run() {
    QueueReceiver telemetryQueueReceiver = QueueReceiver(this->telemetryQueue);
    QueueSender broadcastQueueSender = QueueSender(this->broadcastQueue);
    QueueSender snapshotQueueSender = QueueSender(this->snapshotQueue);

    while (this->running) {
        char telemetryBuffer[sizeof(TelemetryUnion)];
        int size;
        if ((size = telemetryQueueReceiver.receive((char*)&telemetryBuffer, sizeof(TelemetryUnion))) != 0) {
            Telemetry * telemetry = (Telemetry *)&telemetryBuffer[0];

            // capture last known good state of Telemetry in Snapshot
            switch (telemetry->getType()) {
                case LocationType:
                    snapshot.location = Location((Location&)telemetryBuffer);
                    break;
                case HeadingType:
                    snapshot.heading = Heading((Heading&)telemetryBuffer);
                    break;
                default:
                    break;
            }

            char detail[128];
            telemetry->writeJson(detail);
            console.log("Routing", this->snapshotQueue->getQueueName(), telemetry->getName(), detail);

            // snapshot triggered by Location Telemetry
            if (telemetry->getType() == LocationType) {
                snapshotQueueSender.send((char *) &this->snapshot, sizeof(Snapshot));
            }

            // send everything to the broadcast queue
            broadcastQueueSender.send((char*)&telemetryBuffer, size);
        }
    }
}

AutoPilotProcess::AutoPilotProcess(Queue * snapshot, Queue * broadcast, Gps * g, Heading * heading) {
    this->snapshotQueue = snapshot;
    this->broadcastQueue = broadcast;
    this->gps = g;
    this->compass = heading;

    this->target[0].location.latitude = -37.869671;
    this->target[0].location.longitude = 144.970274;
    this->target[0].radiusMeters = 15;
    this->target[1].location.latitude = -37.867156;
    this->target[1].location.longitude = 144.970757;
    this->target[1].radiusMeters = 10;
    this->target[2].location.latitude = -37.867609;
    this->target[2].location.longitude = 144.967614;
    this->target[2].radiusMeters = 25;
    this->targetIndex = 0;
}

void AutoPilotProcess::run() {
    QueueReceiver snapshotQueueReceiver = QueueReceiver(this->snapshotQueue);
    QueueSender broadcastQueueSender = QueueSender(this->broadcastQueue);

    while (this->running) {
        Snapshot snapshot;
        if ((snapshotQueueReceiver.receive((char*)&snapshot, sizeof(Snapshot))) != 0) {
            Target * currentTarget = &this->target[this->targetIndex];

            float expectedHeading = getExpectedHeading(&snapshot.location, &currentTarget->location);

            float impulse = 0.7;

            Steer steer = Steer();
            steer.steer = getSteer(snapshot.heading.degrees, expectedHeading);

            Thrust thrust = Thrust();
            thrust.powerLeft = impulse + (((steer.steer) * impulse)/2);
            thrust.powerRight = impulse + (((0-steer.steer) * impulse)/2);

            // move compass for simulation porpoises
            this->compass->degrees = simulateCompass(this->compass->degrees, steer.steer);

            // move GPS offset for simulation porpoises
            getLocation(&this->gps->offsetLocation, snapshot.heading.degrees,
                        6 * impulse, &this->gps->offsetLocation);

            char buffer[60];
            sprintf(buffer, "hed %2.2f > %2.2f str %2.2f L %2.2f R %2.2f",
                    snapshot.heading.degrees, expectedHeading, steer.steer, thrust.powerLeft, thrust.powerRight);
            console.log("AutoPilot", this->snapshotQueue->getQueueName(), "output", buffer);

            broadcastQueueSender.send((char*)&steer, sizeof(Steer));
            broadcastQueueSender.send((char*)&thrust, sizeof(Thrust));

            bool sendTarget = false;
            if (!this->locationLock) {
                this->locationLock = true;
                sendTarget = true;
            }
            float distance = distanceMeters(&snapshot.location, &currentTarget->location);
            if (distance < currentTarget->radiusMeters) {
                if (++this->targetIndex > 2) { this->targetIndex = 0; };
                currentTarget = &this->target[this->targetIndex];
                sendTarget = true;
            }
            if (sendTarget) {
                console.log("AutoPilot", this->snapshotQueue->getQueueName(),
                            "target",(char*)"distance", (int)distance);
                broadcastQueueSender.send((char*)currentTarget, sizeof(Target));
            }

            cpuCheck();
            memCheck();
        }
    }
}

Tamashii::Tamashii() {
    Gps g = Gps((char *)GPS_TTY);
    this->gps = &g;

    Queue telemetryQueue = Queue((char *) "/telemetry", sizeof(TelemetryUnion), 10);
    Queue broadcastQueue = Queue((char *) "/broadcast", sizeof(TelemetryUnion), 10);
    Queue snapshotQueue = Queue((char *) "/snapshot", sizeof(Snapshot), 4);

    RoutingProcess routingProcess = RoutingProcess(&telemetryQueue, &broadcastQueue, &snapshotQueue);
    Thread routingThread = Thread(4096, &routingProcess);

    EspProcess espProcess = EspProcess(&broadcastQueue);
    Thread espThread = Thread(2048, &espProcess);

    SensorProcess sensorProcess = SensorProcess(&telemetryQueue);
    Thread sensorThread = Thread(2048, &sensorProcess);

    AutoPilotProcess autoPilotProcess = AutoPilotProcess(&snapshotQueue,
                                     &broadcastQueue, this->gps, &sensorProcess.heading);
    Thread autopilotThread = Thread(8192, &autoPilotProcess);

    this->telemetryQ = &telemetryQueue;

    run();

    routingThread.join();
    espThread.join();
    sensorThread.join();
    autopilotThread.join();
}

void Tamashii::run() {
    QueueSender queueSender = QueueSender(this->telemetryQ);
    while (this->running) {
        Location location;
        this->gps->findLocation(&location);
        queueSender.send((char*)&location, sizeof(Location));
    }
}

extern "C" {
    int main(int argc, FAR char *argv[]) {
#ifdef CONFIG_CATCHPOLE_TAMASHII_CXXINITIALIZE
        up_cxxinitialize();
#endif
        startMesage();
        cpuCheck();
        memCheck();
        Tamashii();
        return 0;
    }
}
