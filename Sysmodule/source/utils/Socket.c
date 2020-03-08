#include <arpa/inet.h>
#include <Commands.h>
#include <errno.h>
#include "Log.h"
#include "Socket.h"
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/socket.h>
#include <unistd.h>

// Buffer size (in bytes)
#define BUFFER_SIZE 255
// Queue size
#define CONN_QUEUE 0
// Port to listen on
#define LISTEN_PORT 3333
// Timeout (sec) for accepts and reads
#define TIMEOUT 3
#define TIMEOUTE 1E+9

static struct sockaddr_in addr;
// Listening socket
static int lSocket = -1;
// Current transfer socket
static int tSocket = -1;

// File to log to (thread-safe/won't be overwritten as these functions are only called in one thread)
FILE * logFile;

int createListeningSocket() {
    logFile = logOpenFile();

    // Get socket
    lSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (lSocket < 0) {
        logError(logFile, "[SOCKET] Unable to create listening socket", errno);
        return -1;
    }

    // Bind and listen
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(LISTEN_PORT);
    const int optVal = 1;
    setsockopt(lSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&optVal, sizeof(optVal));
    if (bind(lSocket, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
        logError(logFile, "[SOCKET] Error binding to address/port", errno);
        return -2;
    }
    if (listen(lSocket, CONN_QUEUE) != 0) {
        logError(logFile, "[SOCKET] Error listening", errno);
        return -3;
    }

    // Succeeded
    logSuccess(logFile, "[SOCKET] Listening socket created successfully!");
    return 0;
}

void closeListeningSocket() {
    if (lSocket >= 0) {
        close(lSocket);
        lSocket = -1;
    }

    logCloseFile(logFile);
}

void acceptConnection() {
    // Set timeout
    struct timeval time;
    time.tv_sec = TIMEOUT;
    time.tv_usec = 0;

    // Check for a ready connection
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(lSocket, &readfds);
    switch (select(lSocket + 1, &readfds, NULL, NULL, &time)) {
        case -1:
            logError(logFile, "[SOCKET] Error occurred calling select()", errno);
            // Sleep thread if select didn't pause
            svcSleepThread(TIMEOUTE);
            break;

        case 0:
            // Timed out
            break;

        default: {
            socklen_t sz = sizeof(addr);
            tSocket = accept(lSocket, (struct sockaddr *) &addr, &sz);
            if (tSocket >= 0) {
                // Set read timeout
                setsockopt(tSocket, SOL_SOCKET, SO_RCVTIMEO, (const char *)& time, sizeof(time));
            }
            logError(logFile, "[SOCKET] ERRNO:", errno);
            logSuccess(logFile, "[SOCKET] Transfer socket connected!");
            break;
        }
    }
}

int haveConnection() {
    if (tSocket >= 0) {
        return 0;
    }

    return -1;
}

void closeConnection() {
    if (tSocket >= 0) {
        close(tSocket);
        tSocket = -1;
    }
}

char * readData() {
    char * buf = calloc(BUFFER_SIZE + 1, sizeof(char));
    int rd;
    int pos = 0;
    // Read until end of 'message'
    // do {
        rd = read(tSocket, buf + pos, BUFFER_SIZE - pos);
    //     logSuccess("Received some data!");
    //     char * p = buf + pos;
    //     while (*p) {
    //         logSuccess(p);
    //         p = strchr(p, '\0');
    //         p++;
    //     }
    //     pos += rd;
    // } while (rd > 0 && buf[rd] != '\x1c');

    // Handle errors
    if (rd == 0) {
        closeConnection();
        logError(logFile, "[SOCKET] Lost connection on read - closed tSocket", errno);
        free(buf);
        return NULL;
    } else if (rd < 0) {
        // Timed out
        free(buf);
        return NULL;
    }

    return buf;
}

void writeData(const char * data) {
    // Append end of message character to data
    int len = strlen(data);
    len += 2;
    char * str = (char *) malloc(len * sizeof(char));
    memcpy(str, data, len - 1);
    memset(str + (len - 1), SM_ENDMSG, 1);

    // Write
    if (write(tSocket, str, len) != len) {
        logError(logFile, "[SOCKET] Error writing data", -1);
    } else {
        logSuccess(logFile, "[SOCKET] Wrote data");
    }

    free((void *) str);
}