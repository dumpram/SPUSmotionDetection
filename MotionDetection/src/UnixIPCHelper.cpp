#include "UnixIPCHelper.h"


UnixIPCHelper::UnixIPCHelper(std::string socketPath, void (*givenCallback)(char *)) {
    if ((localDesc = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Error while initializing socket!");
    }
    callback = givenCallback;
    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, socketPath.c_str());
    unlink(local.sun_path);
}

char* UnixIPCHelper::extractData(char *data) {
    char *pch;
    pch = strstr(data, "\"data\":\"") + strlen("\"data\":\"");
    return pch;
}

bool UnixIPCHelper::sendNode(const char *data) {
    if (send(remoteDesc, data, strlen(data), 0) < 0) {
        return false;
    }
    return true;
}

bool UnixIPCHelper::sendNode(unsigned char *data, int size) {
	if (send(remoteDesc, data, size, 0) < 0) {
        return false;
    }
    return true;
}

bool UnixIPCHelper::sendNode(std::vector<unsigned char> &data) {
	return sendNode(&data[0], data.size());
}

bool UnixIPCHelper::bindNode() {
    int len =  strlen(local.sun_path) + sizeof(local.sun_family);  
    if (bind(localDesc, (struct sockaddr *)&local, len) == -1) {
        return false;
    }
    return true;
}

bool UnixIPCHelper::listenNode() {
    if (listen(localDesc, 5) == -1) {
        perror("Error while trying to listen!");
        return false;
    }
    //std::cout << "Started listening!" << std::endl;
    std::thread newOne(&UnixIPCHelper::listenThread, this);
    newOne.detach();
    //std::cout << "Started new thread!" << std::endl;
    return true;
}

void UnixIPCHelper::listenThread() {
    while(1367) {
        int done, n;
        socklen_t t;
        char str[100];
        printf("Waiting for a connection...\n");
        t = sizeof(remote);
        if ((remoteDesc = accept(localDesc, (struct sockaddr *)&remote, &t)) == -1) {
            perror("Error while accepting connection!");
            exit(1);
        }
        printf("Connected.\n");
        done = 0;
        do {
            n = recv(remoteDesc, str, 100, 0);
            if (n <= 0) {
                if (n < 0) perror("Error while receiving");
                done = 1;
            }
            if (!done) {
                callback(str);
            }
        } while(!done);
    }
}

/**
 *  Dummy test code...
 */

// void callback(char *data) {
//     std::cout << data << std::endl;
// }

// int main() {
//     std::cout << "Trying to create object!" << std::endl;
//     UnixIPCHelper helper(std::string(SOCK_PATH), callback);
//     std::cout << "Success while creating object!" << std::endl;
//     helper.bindNode();
//     helper.listenNode();
//     sleep(1);
//     while(1) {
//         helper.sendNode("{\"type\":\"message\",\"data\":\"test\"}\f");
//         sleep(1);
//     }
//}
