/* * BasicSigmaDelta.cpp
 *
 *	Implementation of BasicSigmaDelta algorithm for motion detection. 
 * 
 *  Created on: Jan 18, 2016
 *  Authors: Ivan Pavić, Ivan Spasić 
 */
#include <highgui.hpp>
#include <opencv.hpp>
#include <imgproc.hpp>
#include <iostream>
#include <cmath>
#include <chrono>
#include <UnixIPCHelper.h>
#include <SigmaDeltaImpl.h>
#include <base64.h>
#include <functional>

#define CONTOUR_AREA_THRESH 200

using namespace std;
using namespace cv;
using namespace std::chrono;

/**
 * Profiling wrapper for easier function time profiling.
 */
typedef std::function<void(void)> ProfilingWrapper;
/**
 * Default variance parameter.
 */
static int N = 2;
/**
 * Flag for enabling morphology filtering. Default value => true.
 */
static bool enableMorph = true;
/**
 * Flag for enabling sigma delta mask for output. Default value => true.
 */
static bool enableMask = true;
/**
 * Flag for enabling contour drawing. Default value => false.
 */
static bool enableContours = false;

/**
 * Function measures time elapsed for execution of given wrapper.
 */ 
int measureTime(ProfilingWrapper wrapper, string description) {
	high_resolution_clock::time_point startTime = high_resolution_clock::now();
	wrapper();
	high_resolution_clock::time_point stopTime = high_resolution_clock::now();
//	cout << "Time elapsed for " << description << " is: ";
//	cout << duration_cast<milliseconds>(stopTime - startTime).count() << " ms" << endl;
	return duration_cast<milliseconds>(stopTime - startTime).count();
}

/**
 * Callback for stream socket.
 */
void streamCallback(char *data) {
	std::cout << data << std::endl;
}

/**
 * Callback for detection socket.
 */ 
void detectionCallback(char *data) {
	std::cout << data << std::endl;
}

/**
 * Callback for changing algorithm parameters.
 */ 
void algorithmCallback(char *data) {
	int recv = atoi(data);
	cout << "Received: "<< data << endl;
	switch (recv) {
		case 10: enableMorph = false; break;
		case 11: enableMorph = true; break;
		case 20: enableMask = false; break;
		case 21: enableMask = true; break;
		case 31: N = 1; break;
		case 32: N = 2; break;
		case 33: N = 3; break;
		case 34: N = 4; break;
		case 40: enableContours = false; break;
		case 41: enableContours = true; 
	}
}


/**
 * Main function.
 */
int main(int argc, char** argv) {
	cout << "Background estimation using Sigma-Delta algorithm..." << endl;

	if (argc != 2) {
		cout << "Provide camera device id as argument!" << endl;
		return -1;	
	}		
	
	/**
	 * Input, Mode, Variance, and Mask matrices.
	 */
	Mat I, M, V, E;
	
	vector<unsigned char> buff;
	
	/**
	 * Parameters for JPEG encoding.
	 */
	vector<int> params = vector<int>(2);
	params[0] = CV_IMWRITE_JPEG_QUALITY;
	params[1] = 50;
	
	UnixIPCHelper nodeStream(string("/tmp/node_stream"), streamCallback);
	nodeStream.bindNode();
	nodeStream.listenNode();
	
	UnixIPCHelper nodeDetection(string("/tmp/node_detection"), detectionCallback);
	nodeDetection.bindNode();
	nodeDetection.listenNode();
	
	UnixIPCHelper nodeAlgorithm(string("/tmp/node_algorithm"), algorithmCallback);
	nodeAlgorithm.bindNode();
	nodeAlgorithm.listenNode();
	
	VideoCapture cap(atoi(argv[1]));
	if (!cap.isOpened()) {
		cerr << "Err: Initalizing video capture" << endl;
		return -1;
	}
	
	cap.read(I);
	cvtColor(I, I, CV_RGB2GRAY);

	M = Mat_<uchar>(I.rows, I.cols);
	V = Mat_<uchar>(I.rows, I.cols);
	E = Mat_<uchar>(I.rows, I.cols);

	/**
	 * Element for morphological filtering.
	 */
	Mat morphElement = getStructuringElement( 0, Size(3, 3), Point (1, 1) );
	
	/**
	 * Stream frame reference.
	 */
	Mat streamFrame;
	
	/**
	 * Parameters and containers for edge detection.
	 */
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	/**
	 * Detection flag.
	 */
	bool detectionFlag = false;
	
	/**
	 * Sending dummy byte to enter setup state.
	 */
	nodeAlgorithm.sendNode("1");
	
	/**
	 * Area in contours.
	 */
	 double area;
	
	while (1) {
		bool captureSuccess = cap.read(I); /* read a new frame from camera feed */
		
		if (!captureSuccess) {
			cerr << "Err: reading from camera feed" << endl;
			break;
		}
		
		streamFrame = (enableMask)? E : I;
		
		measureTime([&I] () {cvtColor(I, I, CV_RGB2GRAY);}, "conversion to grayscale");
		
		measureTime([&I, &M, &V, &E] () {basicSigmaDeltaBS(I, M, V, E, N);}, "sigma delta background subtraction");

		if (enableMorph) {
			measureTime([&E, &morphElement] () {morphologyEx(E, E, 2, morphElement);}, "morphology processing"); 
		}
		
		//Canny(E, canny_out, thresh, thresh * 2, 3);
		findContours(E.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		for(unsigned int i = 0; i < contours.size(); i++) {
			Scalar color = Scalar(0, 255, 0);
			area = contourArea(contours[i]);
			if(area > CONTOUR_AREA_THRESH) {
				detectionFlag = true;
			}
			if (enableContours && !enableMask) {
				drawContours(streamFrame, contours, i, color, 10, 8, hierarchy, 0, Point());
			}
		}
		
		if (detectionFlag) {
			nodeDetection.sendNode("1");
		} else {
			nodeDetection.sendNode("0");
		}

		detectionFlag = false;
		
		measureTime([&streamFrame, &buff, &params] () {imencode(".jpg", streamFrame, buff, params);}, "JPEG encoding");
		
		string base64EncodedJPEG;
		
		measureTime([&base64EncodedJPEG, &buff] () {
			base64EncodedJPEG = base64_encode(&buff[0], buff.size());
		}, "base64 encoding");
		
		measureTime([&base64EncodedJPEG, &nodeStream] () {nodeStream.sendNode(base64EncodedJPEG.c_str());}, "sending to Unix socket");
	
	}
	return 0;
}


