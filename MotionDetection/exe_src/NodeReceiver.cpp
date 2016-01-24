#include <iostream>
#include <UnixIPCHelper.h>


using namespace std;

void static callback(char *data) {
	cout << data << endl;
}

int main() {
	UnixIPCHelper recv(string("/tmp/node_img"), callback);
	recv.bindNode();
	recv.listenNode();
	sleep(-1);
	return 0;
}
