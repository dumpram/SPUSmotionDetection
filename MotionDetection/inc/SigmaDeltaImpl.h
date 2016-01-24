/** @file SigmaDeltaImpl.h
 *
 * @brief header file for Sigma Delta algorithm implementation. 
 * Contains method definitions and documentation.
 * @author Ivan Pavic
 * @version 0.1.0
 */

#ifndef __SIGMADELTAIMPL_H__
#define __SIGMADELTAIMPL_H__


#include <highgui.hpp>
#include <opencv.hpp>
#include <imgproc.hpp>

using namespace cv;

/**
 * Basic sigma delta algorithm.
 */
void basicSigmaDeltaBS(Mat &I, Mat &M, Mat &V, Mat &E, int N);

/**
 * Sigma delta algorithm combined with zipfian distribution heuristic.
 */ 
void zipfSigmaDeltaBS(Mat &I, Mat &M, Mat &V, Mat &E, int N);



#endif