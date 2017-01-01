#ifndef SERIAL_HELPER_HPP
#define SERIAL_HELPER_HPP

#include <thread>
#include <termios.h>


#define MAX_BUFFER_SIZE 4096

class SerialHelper {
private:
    std::thread *serialRecieveThread;

    int serialFileDescriptor;

    char *buffer;

    struct termios oldConfiguration;

    struct termios newConfiguration;

    void (*callback)(char *data, int size);

    void listenThread();

    void configureSerialPort();

public:
    SerialHelper(std::string serialPath, void (*callback)(char *, int));

    bool sendSerial(char *data, int size);

    void closeSerial();

    bool isOpen();
};

#endif /* end of include guard: SERIAL_HELPER_H */
