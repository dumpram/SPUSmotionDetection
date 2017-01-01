#include "SerialHelper.hpp"

#include <iostream>
#include <string>
#include <signal.h>

using namespace std;

bool interrupted = false;

void sigIntHandler(int signalId) {
    interrupted = true;
    exit(signalId);
}

void callback(char *data, int size) {
    string message(data, size);
    cout << "RX got: " << message;
}

int main() {
    string message = "This is message from hell!\r\n";
    SerialHelper tty("/dev/ttyUSB0", callback);
    tty.sendSerial((char *)message.c_str(), message.length());
    cout << message.length() << endl;

    signal (SIGINT, sigIntHandler);

    while(!interrupted);

    tty.closeSerial();

    return 0;
}
