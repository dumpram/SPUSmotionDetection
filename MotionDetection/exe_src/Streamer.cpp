#include <highgui.hpp>
#include <opencv.hpp>
#include <imgproc.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <UnixIPCHelper.h>
#include <base64.h>


using namespace std;
using namespace cv;


void callback(char *data) {
	std::cout << data << std::endl;
}
 
int main(int argc, char **argv) {
    cout << "Super fast streaming mode..." << endl;

	if (argc != 2) {
		cout << "Provide camera device id as argument!" << endl;
		return -1;	
	}		
	
	Mat frame;
	vector<uchar> frameBuffer;
	vector<int> params = vector<int>(2);
	params[0] = CV_IMWRITE_JPEG_QUALITY;
	params[1] = 50;
	
    UnixIPCHelper node(string("/tmp/node_img"), callback);
	node.bindNode();
	node.listenNode();
	
    VideoCapture cap(atoi(argv[1]));

	if (!cap.isOpened()) {
		cerr << "Err: Initalizing video capture" << endl;
		return -1;
	}

	while(1) {
	    bool captureSuccess = cap.read(frame);
	    if (!captureSuccess) {
			cerr << "Err: reading from camera feed" << endl;
			break;
		}
		imencode(".jpg", frame, frameBuffer, params);
		string encoded = base64_encode(&frameBuffer[0], frameBuffer.size());
		node.sendNode(encoded.c_str());
	}
	
}