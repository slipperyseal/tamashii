
#ifndef ESP_H
#define ESP_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>

class Stream {
private:
    int handle;
    int len;
    int lastEol;

public:
    Stream(const char * path);
    ~Stream();

    char * filePath;
    char buffer[256];
    char readChar();
    void readLine();
    void writeString(char * string);
};

class AtSession {
public:
    AtSession(const char * path);

    Stream stream;
    bool isError(void);
    bool isOk(void);
    void writeLine(char * line);
    void writeString(char * line);
    void command(char * line);
    void commandResponse();
};

class EspSender {
private:
    AtSession atSession;
public:
    EspSender(const char * path);
    ~EspSender();

    void init();
    void connectAp(char * ssid, char * password);
    void sendPacket(char * string);
};

#endif