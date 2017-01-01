#ifndef SERIAL_HELPER_CPP
#define SERIAL_HELPER_CPP

#include "SerialHelper.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

#include <iostream>


#define DEFAULT_BAUDRATE B115200

SerialHelper::SerialHelper(std::string serialPath,
    void (*callback)(char *, int)) {
    serialFileDescriptor = open(serialPath.c_str(), O_RDWR | O_NONBLOCK);
    buffer = (char *) malloc(sizeof(char) * MAX_BUFFER_SIZE);
    configureSerialPort();
    this->callback = callback;
    std::thread newOne(&SerialHelper::listenThread, this);
    newOne.detach();
}

void SerialHelper::listenThread() {
    while (this->isOpen()) {
        int ncount = read(serialFileDescriptor, buffer, MAX_BUFFER_SIZE);
        if (ncount > 0) {
            callback(buffer, ncount);
        }
    }
}

void SerialHelper::configureSerialPort() {
    if (!this->isOpen()) {
        return;
    }
    tcgetattr(serialFileDescriptor, &oldConfiguration);
    tcgetattr(serialFileDescriptor, &newConfiguration);
    cfsetispeed(&newConfiguration, B115200);
    cfsetospeed(&newConfiguration, B115200);
    newConfiguration.c_cflag |= (CLOCAL | CREAD);
    newConfiguration.c_lflag |= ICANON;
    tcsetattr(serialFileDescriptor, TCSANOW, &newConfiguration);
}

bool SerialHelper::sendSerial(char *data, int size) {
    int ncount = -1;
    if (this->isOpen()) {
        ncount = write(serialFileDescriptor, data, size);
    }
    return ncount == size;
}

void SerialHelper::closeSerial() {
    tcflush(serialFileDescriptor, TCIFLUSH);
    tcsetattr(serialFileDescriptor, TCSANOW, &oldConfiguration);
    serialFileDescriptor = -1;
    close(serialFileDescriptor);
    free(buffer);
}

bool SerialHelper::isOpen() {
    return serialFileDescriptor != -1;
}

#endif /* end of include guard: SERIAL_HELPER_CPP */
