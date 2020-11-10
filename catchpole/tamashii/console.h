
#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <wchar.h>

#define CONSOLE_ENTRIES 32

class Entry {
public:
    int count;
    const char * source;
    const char * instance;
    const char * action;
};

class Console {
    int lastLine;
private:
    Entry entries[CONSOLE_ENTRIES];
    int findRow(const char * source, const char * instance, const char * action);

public:
    Console();

    void log(const char * source, const char * instance, const char * action, char * detail);
    void log(const char * source, const char * instance, const char * action, char * detail, int value);
};

extern Console console;

#endif