#ifndef rutinasVision_h
#define rutinasVision_h

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
using namespace std;
using namespace cv;

void color2gray(const Mat &sourceImage, Mat &destinationImage)
{
	if (destinationImage.empty())
		destinationImage = Mat(sourceImage.rows, sourceImage.cols, CV_8UC1);

	int grayValue = 0;
	for (int y = 0; y < sourceImage.rows; ++y){
		for (int x = 0; x < sourceImage.cols ; ++x){
			grayValue = 0;
			for (int i = 0; i < sourceImage.channels(); ++i)
			{
				grayValue += sourceImage.at<Vec3b>(y, x)[i] ;
			}
			destinationImage.at<uint8_t>(y,x) = grayValue/3;
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

void yiq2color(const Mat &sourceImage, Mat &destinationImage)
{
  if (destinationImage.empty())
    destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());

  for (int y = 0; y < sourceImage.rows; ++y){
    for (int x = 0; x < sourceImage.cols; ++x){
        int vY = sourceImage.at<Vec3b>(y, x)[0];
        int vI = sourceImage.at<Vec3b>(y, x)[1];
        int vQ = sourceImage.at<Vec3b>(y, x)[2];

        destinationImage.at<Vec3b>(y, x)[2] = int(1*vY + 0.956*vI + 0.621*vQ);
        destinationImage.at<Vec3b>(y, x)[1] = int(1*vY - 0.272*vI - 0.647*vQ);
        destinationImage.at<Vec3b>(y, x)[0] = int(1*vY - 1.107*vI + 0.705*vQ);
    }
  }
}

void colorFilter(const Mat &sourceImage, Mat &destinationImage, int range[6])
{
	if (destinationImage.empty())
		destinationImage = Mat(sourceImage.rows, sourceImage.cols, sourceImage.type());

	for (int y = 0; y < sourceImage.rows; ++y){
		for (int x = 0; x < sourceImage.cols; ++x){
			int v0 = sourceImage.at<Vec3b>(y, x)[0];
            int v1 = sourceImage.at<Vec3b>(y, x)[1];
            int v2 = sourceImage.at<Vec3b>(y, x)[2];

            if(v0 >= range[0] && v0 <= range[1] && v1 >= range[2] && v1 <= range[3] && v2 >= range[4] && v2 <= range[5])
            	destinationImage.at<Vec3b>(y, x) = sourceImage.at<Vec3b>(y, x);
            else
            	destinationImage.at<Vec3b>(y, x) = Vec3b(0,0,0);
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


void regionAvg(const Mat &sourceImage, int x1,int y1, int x2, int y2, int &avrg0, int &avrg1, int &avrg2)
{
   avrg0 = 0;
   avrg1 = 0;
   avrg2 = 0;
  for (int y = y1; y < y2; ++y)
  {
    for (int x = x1; x < x2; ++x)
    {
      avrg0 += sourceImage.at<Vec3b>(y,x)[0];
      avrg1 += sourceImage.at<Vec3b>(y,x)[1];
      avrg2 += sourceImage.at<Vec3b>(y,x)[2];
    }
  }
  avrg0 = avrg0/(abs(x2-x1)*abs(y2-y1));
  avrg1 = avrg1/(abs(x2-x1)*abs(y2-y1));
  avrg2 = avrg2/(abs(x2-x1)*abs(y2-y1));


}

void imageHistogram(const Mat &sourceImage, Mat &destinationImage, string histogramChannel)
{
  /// Separate the image in 3 places ( B, G and R )
  vector<Mat> bgr_planes;
  split( sourceImage, bgr_planes );

  /// Establish the number of bins
  int histSize = 256;

  /// Set the ranges ( for B,G,R) )
  float range[] = { 0, 256 } ;
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

  Mat histImage( hist_h + 210, hist_w, CV_8UC3, Scalar( 0,0,0) );

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

  /// Gray line divisor
  for(int y = hist_h; y < hist_h + 20; y++)
  {
  	  for(int x = 0; x < hist_w; x++)
  	  {
	  	  histImage.at<Vec3b>(y, x) = Vec3b(200,200,200);
	  	  histImage.at<Vec3b>(y + 70, x) = Vec3b(200,200,200);
	  	  histImage.at<Vec3b>(y + 140, x) = Vec3b(200,200,200);
  	  }
  }

  Mat gradients[] = { Mat(50, hist_w, CV_8UC3), Mat(50, hist_w, CV_8UC3), Mat(50, hist_w, CV_8UC3) };

  /// HSV Histogram
  if(histogramChannel == "HSV")
  {
    /// HSV color gradients
    for(int y = 0; y < 50; y++)
    {
        for(int x = 0; x < hist_w; x++)
        {
          gradients[0].at<Vec3b>(y, x) = Vec3b(x/2,255,255);
          gradients[1].at<Vec3b>(y, x) = Vec3b(255,x/2,255);
          gradients[2].at<Vec3b>(y, x) = Vec3b(255,255,x/2);
        }
    }

    cvtColor(gradients[0], gradients[0], CV_HSV2BGR);
    cvtColor(gradients[1], gradients[1], CV_HSV2BGR);
    cvtColor(gradients[2], gradients[2], CV_HSV2BGR);
  }
  else if(histogramChannel == "YIQ")
  {
    /// YIQ color gradients
    for(int y = 0; y < 50; y++)
    {
        for(int x = 0; x < hist_w; x++)
        {
          gradients[0].at<Vec3b>(y, x) = Vec3b(x/2,0,0);
          gradients[1].at<Vec3b>(y, x) = Vec3b(0,x/2,0);
          gradients[2].at<Vec3b>(y, x) = Vec3b(0,0,x/2);
        }
    }

    yiq2color(gradients[0], gradients[0]);
    yiq2color(gradients[1], gradients[1]);
    yiq2color(gradients[2], gradients[2]);
  }
  else
  {
    /// RGB color gradients
    for(int y = 0; y < 50; y++)
    {
        for(int x = 0; x < hist_w; x++)
        {
          gradients[0].at<Vec3b>(y, x) = Vec3b(x/2,0,0);
          gradients[1].at<Vec3b>(y, x) = Vec3b(0,x/2,0);
          gradients[2].at<Vec3b>(y, x) = Vec3b(0,0,x/2);
        }
    }
  }

  /// Channel gradients
  for(int y = hist_h + 20; y < hist_h + 70; y++)
  {
  	  for(int x = 0; x < hist_w; x++)
  	  {
	  	  histImage.at<Vec3b>(y, x) = gradients[0].at<Vec3b>(y - hist_h - 20, x);
	  	  histImage.at<Vec3b>(y + 70, x) = gradients[1].at<Vec3b>(y - hist_h - 20, x);
	  	  histImage.at<Vec3b>(y + 140, x) = gradients[2].at<Vec3b>(y - hist_h - 20, x);
  	  }
  }

  destinationImage = histImage;
}

#endif