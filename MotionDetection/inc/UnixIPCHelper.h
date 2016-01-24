/** @file UnixIPCHelper.h
 *
 * @brief header file for UnixHelperIPC class. Contains class definition
 * and method documentation.
 * @author Ivan Pavic
 * @version 0.1.0
 */
#ifndef __UNIXIPCHELPER_H__
#define __UNIXIPCHELPER_H__

#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <thread>
#include <cstdlib>
#include <vector>


/**
 * UnixIPCHelper class creates UNIX socket for interprocess communication.
 * API enables user to create object which represents socket. It enables 
 * user to interact with other process(etc. sending raw data). Additonally
 * it starts listening thread for incoming data. When creating object provide
 * callback function. Provided callback function will be called everytime
 * data is available on socket.
 * 
 */
class UnixIPCHelper {
    private:
        /**
         * Thread instance.
         */
        std::thread *ipcThread;
        /**
         * Local socket address structure.
         */
        struct sockaddr_un local;
        /**
         * Remote socket address structure.
         */
        struct sockaddr_un remote;
        /**
         * Local descriptor.
         */
        int localDesc;
        /**
         * Remote descriptor.
         */
        int remoteDesc;
        /**
         * Full path to socket file.
         */
        std::string sockPath;
        /**
         * Pointer to callback function.
         */
        void (*callback)(char *data);
        /**
         * Listening thread method.
         */
        void listenThread();
        /**
         * Method for parsing JSON data.
         */
        char* extractData(char *data);
        
    public:
        /**
         * Constructor accepts 2 arguments. Socket path and custom callback
         * function.
         */
        UnixIPCHelper(std::string sockPath, void (*callback)(char *));
        /**
         * Sends given data to socket. Returns true if successful, false otherwise.
         */
        bool sendNode(const char *data);
		
		/**
`		 * Sends given bytes to socket.
		 */
		bool sendNode(unsigned char *data, int size);
		/**
		 * Sends given vector of bytes to socket.
		 */
		bool sendNode(std::vector<unsigned char> &data);
		/**
         * Binds node. Returns true if successful, false otherwise.
         */
        bool bindNode();
        /**
         * Starts listening thread. Returns true if successful, false otherwise.
         */
        bool listenNode();
};

#endif
