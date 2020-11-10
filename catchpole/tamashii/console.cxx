
#include <nuttx/config.h>
#include <cstdio>
#include <debug.h>
#include <nuttx/init.h>
#include "platform/cxxinitialize.h"

#include "console.h"

Console console = Console();

#define CONSOLE_START_LINE  5

Console::Console() {
    this->lastLine = CONSOLE_START_LINE;
    for (int x=0;x<CONSOLE_ENTRIES;x++) {
        entries[x].count = 0;
    }
}

void Console::log(const char * source, const char * instance, const char * action, char * detail) {
    this->log(source, instance, action, detail, -1);
}

void Console::log(const char * source, const char * instance, const char * action, char * detail, int value) {
    if (entries[0].count == 0) {
        printf("\033[2J" //clear
               "\r\n"
               "\033[0;32mCatchpole Robotics Platform\033[0m\r\n\r\n"
               "\033[7m%8s\033[0m" // 2+6
               "\033[0;32m\033[7m%10s\033[0m"
               "\033[0;36m\033[7m%12s\033[0m"
               "\033[0;33m\033[7m%10s\033[0m"
               "\033[7m%20s\033[0m",
               "count", "source","instance","action","detail");
    }
    char moreDetail[10];
    if (value != -1) {
        sprintf(moreDetail, "%d", value);
    } else {
        moreDetail[0] = 0;
    }

    int row = findRow(source, instance, action);
    int last = this->lastLine; // this could/should be atomic
    this->lastLine = row;

    printf("\u001b[%d;%df" // goto
           " " // clear last arrow
           "\u001b[%d;%df" // goto
           "> %6d"
           "\033[0;32m%10s\033[0m"
           "\033[0;36m%12s\033[0m"
           "\033[0;33m%10s\033[0m"
           " %s" // detail
           " \033[0;32m%s\033[0m" // more detail
           "\033[K", // clear to EOL
           CONSOLE_START_LINE + last, 1,
           CONSOLE_START_LINE + row, 1,
           entries[row].count,
           source,
           instance != NULL ? instance : "",
           action != NULL ? action : "",
           detail != NULL ? detail : "",
           moreDetail);
}

int Console::findRow(const char * source, const char * instance, const char * action) {
    for (int x=0;x<CONSOLE_ENTRIES;x++) {
        Entry * entry = &entries[x];
        if ((entry->count == 0) ||
            (entry->source == source && entry->instance == instance && entry->action == action)) {
            entry->source = source;
            entry->instance = instance;
            entry->action = action;
            entry->count++;
            return x;
        }
    }
    return CONSOLE_ENTRIES-1;
}