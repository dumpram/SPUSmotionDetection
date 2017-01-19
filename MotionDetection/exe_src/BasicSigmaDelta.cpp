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
#include <SerialHelper.hpp>
#include <SigmaDeltaImpl.h>
#include <base64.h>
#include <functional>

#define CONTOUR_AREA_THRESH 30

#define MAX_ANGLE_X 15
#define MAX_ANGLE_Y 10

#define CMD_MESSAGE "{\"x_offset\":%d, \"y_offset\":%d}\n"


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

static char commandBuffer[100];

static string motionDetected = "Motion detected! :D\r\n";

static bool sendEnable = true;

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


void detect_movement(Mat& gray1, Mat& gray2, Mat& img, Mat& element,
        vector<vector<Point>>& contours){
    Mat differenceImage, thresholdImage;
    ///////////////
    vector<Vec4i> hierarchy;
    ///////////////

    absdiff(gray1, gray2, differenceImage);
    threshold(differenceImage, thresholdImage, 5, 255, CV_THRESH_BINARY);
    blur(thresholdImage, thresholdImage, Size(30, 30));
    threshold(differenceImage, thresholdImage, 5, 255, CV_THRESH_BINARY);
    erode(thresholdImage, thresholdImage, element, Point(0, 0));
    findContours(thresholdImage, contours, CV_RETR_TREE,
        CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    /////////////
    //vector<Rect> boundRect(contours.size());
    /////////////

    //The motion has been detected
    // for(unsigned int i = 0; i < contours.size(); i++){
    //     //second parameter tells if we shoul consider contour orientation
    //     // (clockwise/counter-clockwise) to have signed area number
    //     if (contourArea(contours[i], false) > CONTOUR_AREA_THRESH){
    //         continue;
    //     }
    //     boundRect[i] = boundingRect(Mat(contours[i]));
    //     rectangle(img, Point(boundRect[i].x, boundRect[i].y),
    //         Point(boundRect[i].x + boundRect[i].width,
    //             boundRect[i].y + boundRect[i].height),
    //             CV_RGB(0, 255, 0), 2, 8, 0);
    // }
}


void uartRxCallback(char *data, int size) {
    string message(data, size);
    cout << "RX got: " << message;
	usleep(100 * 1000);
	cout << "Slept enough" << endl;
	sendEnable = true;
}

#define PI 3.14159265359

int getAngle(double x, double xref, double xmax, double max_angle) {
	cout << xmax << " " << x << endl;
	if (x > xmax) {
 		cout << "ERROR";
	}
	max_angle *= PI / 180.0;
    double alfa = atan((((x - xref) / (xmax - xref)) * tan(max_angle)));
    return (int) (alfa * 180.0 / PI);
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
    Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2();
	/**
	 * Input, Mode, Variance, and Mask matrices.
	 */
	Mat I, M, V, E;
	int erosion_size = 5;
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

    SerialHelper uart(string("/dev/ttyUSB0"), uartRxCallback);

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

	Mat element = getStructuringElement(0,
            Size(2 * erosion_size + 1, 2 * erosion_size +1),
            Point(erosion_size, erosion_size));

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
		        M = I;

        bool captureSuccess = cap.read(I); /* read a new frame from camera feed */

		if (!captureSuccess) {
			cerr << "Err: reading from camera feed" << endl;
			break;
		}

		streamFrame = (enableMask)? E : I;

		cvtColor(I, I, CV_RGB2GRAY);

		if (sendEnable) {

        
		

        detect_movement(I, M, E, element, contours);

        vector<Moments> mu(contours.size());

        for(unsigned int i = 0; i < contours.size(); i++) {
            mu[i] = moments(contours[i], false);
        }

        Point2f avgPoint = Point2f();

        //  Get maximum contour area
        double maxArea = CONTOUR_AREA_THRESH;
        unsigned int maxAreaIndex;
        for (unsigned int i = 0; i < contours.size(); i++) {
            area = contourArea(contours[i]);
            if (area > maxArea) {
                maxArea = area;
                maxAreaIndex = i;
            }
        }



        //  Get the mass centers:
        vector<Point2f> mc(contours.size());
        
		int mcCount = 0;
        
		for (unsigned int i = 0; i < contours.size(); i++) {
            mc[i] = Point2f(mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00);
            area = contours[i].size();
            if (!(mc[i].x != mc[i].x || mc[i].y != mc[i].y)) {
                avgPoint.x = (avgPoint.x + mc[i].x);
                avgPoint.y = (avgPoint.y + mc[i].y);
                mcCount++;
            }
        }

        avgPoint.x /= mcCount;
        avgPoint.y /= mcCount;

		Scalar color = Scalar(0, 255, 0);
        Scalar dotColor = Scalar(0, 0, 255);
        Point2f center(streamFrame.cols/2, streamFrame.rows/2);

        for (unsigned int i = 0; i < contours.size(); i++) {
            drawContours(streamFrame, contours, i, color, 10, 8,
                hierarchy, 0, Point());
        }

        if (maxArea > CONTOUR_AREA_THRESH && enableContours && !enableMask) {
            	drawContours(streamFrame, contours, maxAreaIndex, color, 10, 8,
                    hierarchy, 0, Point());
                circle(streamFrame, avgPoint, 4, dotColor, -1, 8, 0 );
                line(streamFrame, center, avgPoint, dotColor);
        }

        circle(streamFrame, center, 4, dotColor, - 1, 8, 0);

		if (maxArea > CONTOUR_AREA_THRESH && sendEnable && enableContours) {
			sendEnable = false;
            double x = avgPoint.x;
            double y = avgPoint.y;
            double xref = streamFrame.cols / 2;
            double yref = streamFrame.rows / 2;
            int xalfa = getAngle(x, xref, xref * 2, MAX_ANGLE_X);
            int yalfa = -getAngle(y, yref, yref * 2, MAX_ANGLE_Y);
			snprintf(commandBuffer, 100, CMD_MESSAGE, xalfa, yalfa);
            printf("%s", commandBuffer);
			uart.sendSerial(commandBuffer, strlen(commandBuffer));
			
		} else {
			
		}
		}

		measureTime([&streamFrame, &buff, &params] () {imencode(".jpg", streamFrame, buff, params);}, "JPEG encoding");

		string base64EncodedJPEG;

		measureTime([&base64EncodedJPEG, &buff] () {
			base64EncodedJPEG = base64_encode(&buff[0], buff.size());
		}, "base64 encoding");

		measureTime([&base64EncodedJPEG, &nodeStream] () {nodeStream.sendNode(base64EncodedJPEG.c_str());}, "sending to Unix socket");

	}
	return 0;
}
