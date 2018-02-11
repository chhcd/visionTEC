#ifndef rutinasVision_h
#define rutinasVision_h

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace std;
using namespace cv;







void showMenu()
{
	cout << "\n\n+---------------------- USER INTERFACE ----------------------+\n\n";
	cout << "P. Cambiar Modo continuo/ Modo congelado\n";
	cout << "X. Terminar\n";
}
/*
 * This method flips horizontally the sourceImage into destinationImage. Because it uses 
 * "Mat::at" method, its performance is low (redundant memory access searching for pixels).
 */
void flipImageBasic(const Mat &sourceImage, Mat &destinationImage)
{
	if (destinationImage.empty())
		destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());

	for (int y = 0; y < sourceImage.rows; ++y)
		for (int x = 0; x < sourceImage.cols / 2; ++x)
			for (int i = 0; i < sourceImage.channels(); ++i)
			{
				destinationImage.at<Vec3b>(y, x)[i] = sourceImage.at<Vec3b>(y, sourceImage.cols - 1 - x)[i];
				destinationImage.at<Vec3b>(y, sourceImage.cols - 1 - x)[i] = sourceImage.at<Vec3b>(y, x)[i];
			}
}

void color2gray(const Mat &sourceImage, Mat &grayImage)
{
	int grayValue = 0;
	for (int y = 0; y < sourceImage.rows; ++y){
		for (int x = 0; x < sourceImage.cols ; ++x){
			grayValue = 0;
			for (int i = 0; i < sourceImage.channels(); ++i)
			{
				grayValue += sourceImage.at<Vec3b>(y, x)[i] ;
			}
			grayImage.at<uint8_t>(y,x) = grayValue/3;
		}
		
	}
}

void color2yiq(const Mat &sourceImage, Mat &destinationImage)
{
	if (destinationImage.empty())
		destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());

	for (int y = 0; y < sourceImage.rows; ++y){
		for (int x = 0; x < sourceImage.cols; ++x){
			int vR = sourceImage.at<Vec3b>(y, x)[0];
            int vG = sourceImage.at<Vec3b>(y, x)[1];
            int vB = sourceImage.at<Vec3b>(y, x)[2];

			destinationImage.at<Vec3b>(y, x)[0] = int(0.299*vR + 0.587*vG + 0.114*vB);
			destinationImage.at<Vec3b>(y, x)[1] = int(0.596*vR - 0.275*vG - 0.321*vB);
			destinationImage.at<Vec3b>(y, x)[2] = int(0.212*vR - 0.523*vG + 0.311*vB);
		}
		
	}
}

void gray2threshold(const Mat &sourceImage, Mat &binImage, uint8_t threshold_value){
  /* 
  	Last param is type
  	0: Binary
    1: Binary Inverted
    2: Threshold Truncated
    3: Threshold to Zero
    4: Threshold to Zero Inverted
   */

  threshold( sourceImage, binImage, threshold_value, 255,0 );

}

#endif