
#include "console.h"
#include "esp.h"

Stream::Stream(const char * path) {
    this->filePath = (char*)path;
    this->handle = open(path, O_RDWR);
    if (this->handle == -1) {
        printf("Unable to open %s\r\n", path);
    }
    this->len = 0;
    this->lastEol = -1;
}

Stream::~Stream() {
    close(this->handle);
}

char Stream::readChar(void) {
    char ch[1];
    int result = read(this->handle, ch, 1);
    if (result == -1) {
        return 0;
    }
    return ch[0];
}

void Stream::readLine(void) {
    for (int x=0;x<(int)sizeof(buffer);x++) {
        int result = read(this->handle, &this->buffer[x], 1);
        if (result == -1) {
            return;
        }
        if (this->buffer[x] == '\r') {
            this->buffer[x] = 0;
        } else if (this->buffer[x] == '\n') {
            this->buffer[x] = 0;
            if (buffer[0] == 'W') {
                console.log("AtSession",this->filePath,"command",(char*)this->buffer);
            }
            return;
        }
    }
}

void Stream::writeString(char * line) {
    write(handle,line,strlen(line));
}

AtSession::AtSession(const char * path) : stream(path) {
}

bool AtSession::isError(void) {
    return stream.buffer[0] == 'E' && stream.buffer[1] == 'R' && stream.buffer[2] == 'R' &&
           stream.buffer[3] == 'O' && stream.buffer[4] == 'R' && stream.buffer[5] == 0;
}

bool AtSession::isOk(void) {
    return stream.buffer[0] == 'O' && stream.buffer[1] == 'K' && stream.buffer[3] == 0;
}

void AtSession::writeLine(char * line) {
    stream.writeString(line);
    stream.writeString((char*)"\r\n");
}

void AtSession::command(char * line) {
    console.log("AtSession",this->stream.filePath,"command",line);
    this->writeLine(line);
    this->commandResponse();
}

void AtSession::commandResponse() {
    stream.readLine(); // first CRLF
    while (!isError() && !isOk()) {
        stream.readLine();
    }
}

EspSender::EspSender(const char * path) : atSession(path) {
}

EspSender::~EspSender() {
}

void EspSender::sendPacket(char * string) {
    char cipsend[32];
    sprintf(cipsend, "AT+CIPSEND=%d", (int)strlen(string));
    atSession.command(cipsend);
    atSession.stream.readChar();
    atSession.stream.readChar();
    atSession.stream.writeString(string);
    atSession.stream.readLine();
    atSession.stream.readLine();
    atSession.stream.readLine();
    atSession.stream.readLine();
    console.log("EspSender",this->atSession.stream.filePath,"send",(char*)string);
}

void EspSender::connectAp(char * ssid, char * password) {
    atSession.stream.writeString((char*)"AT+CWJAP=\"");
    atSession.stream.writeString(ssid);
    atSession.stream.writeString((char*)"\",\"");
    atSession.stream.writeString(password);
    atSession.stream.writeString((char*)"\"\r\n");
    atSession.commandResponse();
}

void EspSender::init() {
    atSession.command((char*)"ATE0");
    //atSession.command((char*)"AT+GMR");    // view version info
    atSession.command((char*)"AT+CWMODE=1"); // set station mode
    //atSession.command((char*)"AT+CWLAP");  // list access points
    //atSession.command((char*)"AT+CWJAP?"); // show connected AP

    atSession.command((char*)"AT+CWQAP");    // disconnect
    this->connectAp((char*)"ssid!", (char*)"password!"); // once connected, disconnect/connect can be omitted

    atSession.command((char*)"AT+CIPCLOSE");
    atSession.command((char*)"AT+CIPSTART=\"UDP\",\"10.0.0.29\",1185,1185,2");
}
