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
			int vB = sourceImage.at<Vec3b>(y, x)[0];
            int vG = sourceImage.at<Vec3b>(y, x)[1];
            int vR = sourceImage.at<Vec3b>(y, x)[2];

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

void imageHistogram(const Mat &src)
{
  /// Separate the image in 3 places ( B, G and R )
  vector<Mat> bgr_planes;
  split( src, bgr_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for B,G,R) )
  float range[] = { 0, 255 } ;
  const float* histRange = { range };

  bool uniform = true; bool accumulate = false;

  Mat b_hist, g_hist, r_hist;

  /// Compute the histograms:
  calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
  calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

  // Draw the histograms for B, G and R
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

  /// Draw for each channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
                       Scalar( 255, 0, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
                       Scalar( 0, 255, 0), 2, 8, 0  );
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
                       Scalar( 0, 0, 255), 2, 8, 0  );
  }

  /// Display
  namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE );
  imshow("calcHist Demo", histImage );
}

#endif