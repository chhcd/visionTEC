#define X_MAX 320
#define Y_MAX 240

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <time.h>

typedef long double ldd;
using namespace std;
using namespace cv;

struct rMoments{
	// ordinary
	ldd m00;
	ldd m10, m01, m11;
	ldd m20, m02;

	// centralized
	ldd u00;
	ldd u10, u01, u11;
	ldd u20, u02;

	// normalized
	ldd n20, n02;
	ldd n11;

	// invariant Hu
	ldd phi1, phi2;

	// Thetha
	ldd theta;
};



/*

*/
void generateSeeds(vector<Point> &vStrike, unsigned int N_SEEDS){
	// generate random seed
	srand(time(NULL));
	
	for(int i = 0; i < N_SEEDS; i++ ){
		int temp_x = rand()%(X_MAX + 1) + 1;
		int temp_y = rand()%(Y_MAX + 1) + 1; 
		vStrike.push_back(Point(temp_x, temp_y));
	}
}



bool isPosible(int y, int x){
	return (x<X_MAX && x>=0 && y<Y_MAX && y>=0);
}
/*

*/
vector<vector<Point> > mycontours(const Mat binImage, unsigned int n_seeds, Mat &colormat){

	// Vector of Objects
	vector<vector<Point> > vObjects;

	// building the object vector
	vector<Point> tempObject;


	// Color image as helper
	Mat colorImage(binImage.size().height,binImage.size().width,CV_8UC3, Scalar(0,0,0));
	
	// queue to explore
	queue<Point> qPoints;
	
	// vector to store N random seed
	vector<Point> vStrike;
	
	generateSeeds(vStrike,n_seeds);
	//printSeeds(vStrike);

	// strike the image and look for obj
	for (int i = 0; i < n_seeds; i++){
		tempObject.clear();
		qPoints.push(vStrike[i]);

		int colorR = rand()%(255 + 1) + 1;
		int colorG = rand()%(255 + 1) + 1;
		int colorB = rand()%(255 + 1) + 1;

		while(!qPoints.empty()){
			int x = qPoints.front().x;
			int y = qPoints.front().y;
			int val = binImage.at<uint8_t>(y,x);

			// IS REGION
			if(val > 0 && colorImage.at<Vec3b>(y,x)[0] == 0 ){
				// insert points in vector of regions
				tempObject.push_back(Point(x,y));

				// color matrix
				colorImage.at<Vec3b>(y,x)[2] = colorR;
				colorImage.at<Vec3b>(y,x)[1] = colorG;
				colorImage.at<Vec3b>(y,x)[0] = colorB;
				
				if( isPosible(y+1,x))
					if(binImage.at<uint8_t>(y+1,x) && colorImage.at<Vec3b>(y+1,x)[0] == 0 )
						qPoints.push(Point(x,y+1));
				
				if(isPosible(y-1,x))
					if(binImage.at<uint8_t>(y-1,x) && colorImage.at<Vec3b>(y-1,x)[0] == 0 )
						qPoints.push(Point(x,y-1));
							
				if(isPosible(y,x+1))
					if(binImage.at<uint8_t>(y,x+1)   && colorImage.at<Vec3b>(y,x+1)[0] == 0)
					qPoints.push(Point(x+1,y));
				
				if(isPosible(y,x-1))
					if(binImage.at<uint8_t>(y,x-1) && colorImage.at<Vec3b>(y,x-1)[0] == 0 )
						qPoints.push(Point(x-1,y));
				
			}

			
			qPoints.pop();	
		}

		// if found a region list it
		if(tempObject.size() > 50)
			vObjects.push_back(tempObject);

		

	}
	colormat = colorImage;
	// imshow("color mat", colorImage);
	return vObjects;
}



void printRegions(vector<vector<Point> > vRegions){

	for(int i = 0; i < vRegions.size(); i++){
		printf("Objecto # %u \t\t Numero de puntos: %lu\n", i,vRegions[i].size());
		// for(int j = 0; j < vRegions[i].size(); j++ ){
		// 	printf("%u \t %u \n",vRegions[i][j].x,vRegions[i][j].y);
		// }
		printf("\n");
	}
}

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

void imageHistogram(const Mat &sourceImage, Mat &destinationImage, string histogramChannel, int channelNum)
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

  Mat hist;

  /// Compute the histograms:
  calcHist( &bgr_planes[channelNum], 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate );

  // Draw the histograms for B, G and R
  int hist_w = 512; int hist_h = 400;
  int bin_w = cvRound( (double) hist_w/histSize );

  Mat histImage( hist_h + 70, hist_w, CV_8UC3, Scalar( 0,0,0) );

  /// Normalize the result to [ 0, histImage.rows ]
  normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

  Scalar lineColor = (channelNum == 0) ? Scalar(255,0,0) : ( (channelNum == 1) ? Scalar(0,255,0) : Scalar(0,0,255) );

  /// Draw for each channel
  for( int i = 1; i < histSize; i++ )
  {
      line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
                       Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
                       lineColor, 2, 8, 0  );
  }

  /// Gray line divisor
  for(int y = hist_h; y < hist_h + 20; y++)
  {
  	  for(int x = 0; x < hist_w; x++)
  	  {
	  	  histImage.at<Vec3b>(y, x) = Vec3b(200,200,200);
  	  }
  }

  Mat gradient = Mat(50, hist_w, CV_8UC3);

  /// HSV Histogram
  if(histogramChannel == "HSV")
  {
    /// HSV color gradients
    for(int y = 0; y < 50; y++)
    {
        for(int x = 0; x < hist_w; x++)
        {
          gradient.at<Vec3b>(y, x) = (channelNum == 0) ? Vec3b(x/2,255,255) : ( (channelNum == 1) ? Vec3b(255,x/2,255) : Vec3b(255,255,x/2) );
        }
    }

    cvtColor(gradient, gradient, CV_HSV2BGR);
  }
  else if(histogramChannel == "YIQ")
  {
    /// YIQ color gradients
    for(int y = 0; y < 50; y++)
    {
        for(int x = 0; x < hist_w; x++)
        {
          gradient.at<Vec3b>(y, x) = (channelNum == 0) ? Vec3b(x/2,0,0) : ( (channelNum == 1) ? Vec3b(0,x/2,0) : Vec3b(0,0,x/2) );
        }
    }

    yiq2color(gradient, gradient);
  }
  else
  {
    /// RGB color gradients
    for(int y = 0; y < 50; y++)
    {
        for(int x = 0; x < hist_w; x++)
        {
          gradient.at<Vec3b>(y, x) = (channelNum == 0) ? Vec3b(x/2,0,0) : ( (channelNum == 1) ? Vec3b(0,x/2,0) : Vec3b(0,0,x/2) );
        }
    }
  }

  /// Channel gradients
  for(int y = hist_h + 20; y < hist_h + 70; y++)
  {
  	  for(int x = 0; x < hist_w; x++)
  	  {
	  	  histImage.at<Vec3b>(y, x) = gradient.at<Vec3b>(y - hist_h - 20, x);
  	  }
  }

  destinationImage = histImage;
}