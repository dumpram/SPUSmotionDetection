/* * simple_sigma_delta.cpp
 *
 *  Created on: Jan 3, 2016
 *      Author: ivan
 */
#include <highgui.hpp>
#include <opencv.hpp>
#include <imgproc.hpp>
#include <iostream>
#include <cmath>

using namespace std;
using namespace cv;

 

int main(int argc, char** argv) {
	cout << "Background estimation using Sigma-Delta algorithm..." << endl;

	if (argc != 2) {
		cout << "Provide camera device id as argument!" << endl;
		return -1;	
	}		

	Mat I, M_1, V_1, E, O;
	VideoCapture cap(atoi(argv[1]));

	if (!cap.isOpened()) {
		cout << "ERROR INITIALIZING VIDEO CAPTURE!" << endl;
		return -1;
	}

	namedWindow("Raw",CV_WINDOW_AUTOSIZE); //raw feed
	namedWindow("M",CV_WINDOW_AUTOSIZE); //background estimator feed
	namedWindow("V",CV_WINDOW_AUTOSIZE); //variance feed
	namedWindow("E",CV_WINDOW_AUTOSIZE); //resulting binary feed


	cap.read(I);
	cvtColor(I, I, CV_RGB2GRAY);

	//M_1 = I;
	//V_1 = I;

	M_1 = Mat_<uchar>(I.rows, I.cols);
	V_1 = Mat_<uchar>(I.rows, I.cols);
	E = Mat_<uchar>(I.rows, I.cols);
//	for(int j = 0; j < I.rows; j++){
	//	for(int i = 0; i < I.cols; i++){
		//	V_1.at<uchar>(j, i) = 0;
					
		//}
	//}



	while (1) {
		//Mat E = I;
		bool bSuccess = cap.read(I); // read a new frame from camera feed
		cvtColor(I, I, CV_RGB2GRAY);

		if (!bSuccess) {
			cout << "ERROR READING FRAME FROM CAMERA FEED" << endl;
			break;
		}

		for(int j = 0; j < I.rows; j++) {
			for(int i = 0; i < I.cols; i++) {
				if (E.at<uchar>(j, i) == 0) {
					if (M_1.at<uchar>(j, i) < I.at<uchar>(j, i)) {
						M_1.at<uchar>(j, i)++;
					}
					if (M_1.at<uchar>(j, i) > I.at<uchar>(j, i)) {
						M_1.at<uchar>(j, i)--;
					}
					uchar O = abs(M_1.at<uchar>(j, i) - I.at<uchar>(j, i));
					if (O != 0) {
						if (V_1.at<uchar>(j, i) < 1*O) {
							V_1.at<uchar>(j, i)++;
						}	
						if (V_1.at<uchar>(j, i) > 1*O) {
							V_1.at<uchar>(j, i)--;
						}
					}
					if (O < V_1.at<uchar>(j, i)) {
						E.at<uchar>(j, i) = (uchar)255;
					}
					if (O >= V_1.at<uchar>(j, i)) {
						E.at<uchar>(j, i) = (uchar)0;
					}
				}
			}
		}
		imshow("Raw", I); //show raw frame
		imshow("M", M_1); //show background estimation frame
		imshow("V", V_1); //show variance estimation frame
		imshow("E", E); //show motion detection frame
		waitKey(10); //wait 10ms
	}
	return 0;
}


