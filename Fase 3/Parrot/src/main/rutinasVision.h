#ifndef rutinasVision_h
#define rutinasVision_h

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <vector>
#include <queue>
#include <time.h>
#include <map>
#include <set>
#include <climits>
#include <utility>
#include <algorithm>

using namespace std;
using namespace cv;

#define X_MAX 320
#define Y_MAX 240
#define MAX_ARRAY 300
#define INF 9999
#define GOING_LEFT 0
#define GOING_RIGHT 1
#define GOING_NORMAL 2

#define PI 3.14159265
#define START_POINT 360,700
#define END_POINT_1 360,70
#define END_POINT_2 360,360
#define N_PIVOTS 150
#define N_NEIGHBORS 5

typedef long double ldd;

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

  threshold( sourceImage, binImage, threshold_value, 255, 0);
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

void generateSeeds(vector<Point> &vStrike, unsigned int N_SEEDS){
  // generate random seed
  srand(time(NULL));
  
  for(int i = 0; i < N_SEEDS; i++ ){
    int temp_x = rand()%X_MAX;
    int temp_y = rand()%Y_MAX; 
    vStrike.push_back(Point(temp_x, temp_y));
  }
}

bool isPosible(int y, int x){
  return (x<X_MAX && x>=0 && y<Y_MAX && y>=0);
}

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

    int colorR = rand()%255 + 1;
    int colorG = rand()%255 + 1;
    int colorB = rand()%255 + 1;

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
    //  printf("%u \t %u \n",vRegions[i][j].x,vRegions[i][j].y);
    // }
    printf("\n");
  }
}

ldd getOrdinaryMoment(vector<Point> vecP, int xOrder, int yOrder){
  ldd tempMomment = 0;

  for (int i = 0; i < vecP.size(); i++){
    tempMomment += pow(vecP[i].x,xOrder) * pow(vecP[i].y,yOrder); 
    
  }
  return tempMomment;
}


ldd getCentralizedMoment(vector<Point> vecP, int xOrder,  ldd x_m, int yOrder, ldd y_m){
  ldd tempCMommentX = 0.0;
  ldd tempCMommentY = 0.0;

  for(int i = 0; i < vecP.size(); i++){
    tempCMommentX += pow((vecP[i].x - x_m),xOrder);
    tempCMommentY += pow((vecP[i].y - y_m),yOrder);

  }

  return tempCMommentX*tempCMommentY ;
}

ldd getNormalizedMoment(ldd uPQ,ldd m00, int xOrder, int yOrder){

  ldd rho = (xOrder + yOrder)/2 +1;
  return uPQ/(pow(m00,rho));
}

rMoments computeMoments(vector<Point> vp){
  rMoments tempMt;

  // ORDINARUY
  tempMt.m00 = getOrdinaryMoment(vp,0,0);
  tempMt.m10 = getOrdinaryMoment(vp,1,0);
  tempMt.m01 = getOrdinaryMoment(vp,0,1);
  tempMt.m11 = getOrdinaryMoment(vp,1,1);
  tempMt.m02 = getOrdinaryMoment(vp,0,2);
  tempMt.m20 = getOrdinaryMoment(vp,2,0);

  ldd x_m = tempMt.m10/tempMt.m00;
  ldd y_m = tempMt.m01/tempMt.m00;
  
  // CENTRALIZED
  tempMt.u10 = getCentralizedMoment(vp,1,x_m,0,y_m);
  tempMt.u01 = getCentralizedMoment(vp,0,x_m,1,y_m);  
  tempMt.u11 = tempMt.m11 - (y_m)*tempMt.m10; //  tempMt.u11 = getCentralizedMoment(vp,1,x_m,1,y_m);  
  tempMt.u20 = tempMt.m20 - (x_m)*tempMt.m10; //  tempMt.u20 = getCentralizedMoment(vp,2,x_m,0,y_m);  
  tempMt.u02 = tempMt.m02 - (y_m)*tempMt.m01; //  tempMt.u02 = getCentralizedMoment(vp,0,x_m,2,y_m);  

  // NORMALIZED
  tempMt.n20 =  getNormalizedMoment(tempMt.u20, tempMt.m00, 2,0);
  tempMt.n02 =  getNormalizedMoment(tempMt.u02, tempMt.m00, 0,2);
  tempMt.n11 =  getNormalizedMoment(tempMt.u11, tempMt.m00, 1,1);

  // HU INVARIANT
  tempMt.phi1 = tempMt.n20 + tempMt.n02;
  tempMt.phi2 = pow(tempMt.n20 - tempMt.n02,2) + 4*pow(tempMt.n11,2);


  // angle in radians
  tempMt.theta = 0.5*atan2(2*tempMt.u11, tempMt.u20-tempMt.u02);//*180/PI;
  return tempMt;
}

int mapC(double x, double in_min, double in_max, double out_min, double out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void showRegionGraph(double phi1, double phi2, Mat graph){
    
    int delta = 10;

    // create axis lines
    // Y
    line(graph, Point(20,20), Point(20,685), CV_RGB(0,255,255),1,8,0);
    // X
    line(graph, Point(20,685), Point(985,685), CV_RGB(0,255,255),1,8,0);

    // Phi 1
    putText(graph,"PHI 2", Point(25,25) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    putText(graph,"PHI 1", Point(925,675) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );


    // X MAX  = 1.5 = 1000
    // Y MAX  = 2.0 = 700
    double x_max = 1.5;
    double y_max = 2.0;
    double x_min = 0.0;
    double y_min = 0.0;
    double mat_x_max = 984;
    double mat_y_max = 30;

    double mat_x_min = 30;
    double mat_y_min = 675;


    double fig1X = 0.38;
    double fig1Y = 0.07;

    int fig1X_coord = mapC(fig1X,x_min,x_max,mat_x_min,mat_x_max);
    int fig1Y_coord = mapC(fig1Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Golfista", Point(fig1X_coord - delta,fig1Y_coord - delta - 10) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig1X_coord - delta, fig1Y_coord - delta),Point(fig1X_coord + delta, fig1Y_coord + delta),Scalar(255,0,0),2);

    double fig2X = 0.16;
    double fig2Y = 0.001;

    int fig2X_coord = mapC(fig2X,x_min,x_max,mat_x_min,mat_x_max);
    int fig2Y_coord = mapC(fig2Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Pelota", Point(fig2X_coord - delta,fig2Y_coord - delta - 10) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig2X_coord - delta, fig2Y_coord - delta),Point(fig2X_coord + delta, fig2Y_coord + delta),Scalar(255,0,0),2);

    double fig3X = 0.2;
    double fig3Y = 0.005;

    int fig3X_coord = mapC(fig3X,x_min,x_max,mat_x_min,mat_x_max);
    int fig3Y_coord = mapC(fig3Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Palo ancho", Point(fig3X_coord - delta,fig3Y_coord - delta - 20) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig3X_coord - delta, fig3Y_coord - delta),Point(fig3X_coord + delta, fig3Y_coord + delta),Scalar(255,0,0),2);



    double fig4X = 1.36;
    double fig4Y = 1.80;

    int fig4X_coord = mapC(fig4X,x_min,x_max,mat_x_min,mat_x_max);
    int fig4Y_coord = mapC(fig4Y,y_min,y_max,mat_y_min,mat_y_max);
    putText(graph,"Baston Basic", Point(fig4X_coord - delta*2,fig4Y_coord - delta*2 - 10) , FONT_HERSHEY_COMPLEX, 0.5, Scalar(255,255,255), 0.5,8,false );
    rectangle(graph, Point(fig4X_coord - delta*6, fig4Y_coord - delta*6),Point(fig4X_coord + delta*6, fig4Y_coord + delta*6),Scalar(255,0,0),2);



    // circle(graph, Point(fig1X_coord,fig1Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
    // circle(graph, Point(fig2X_coord,fig2Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
    // circle(graph, Point(fig3X_coord,fig3Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
    // circle(graph, Point(fig4X_coord,fig4Y_coord), 2 ,Scalar(0,0,255),CV_FILLED,8,0);
  
    // plot new point

    int newFigX = mapC(phi1,x_min,x_max,mat_x_min,mat_x_max);
    int newFigY = mapC(phi2,y_min,y_max,mat_y_min,mat_y_max);

    circle(graph, Point(newFigX,newFigY), 2 ,Scalar(0,0,255),CV_FILLED,8,0);

    // imshow("Grafica de regiones", graph);
}

int segmentationAndClassification(Mat bgrImage, int rBGR[], int &horAxisFig, int &verAxisFig, int &movAction, bool showImg, bool verbose)
{
    Mat bgrFilter;
    Mat binImage;
    Mat colormat;
    Mat graph;

    int recognizedObjects = 0;

    colorFilter(bgrImage,bgrFilter,rBGR);
    cvtColor(bgrFilter, bgrFilter, CV_BGR2GRAY);
    
    gray2threshold(bgrFilter,binImage,80);
    
    // each region is a vector of Point
    vector<vector<Point> > vp = mycontours(binImage,300,colormat);

    // vector to store moments
    vector<rMoments> vMoments;

    if(verbose) { printf("Number of regions: %lu \n\n", vp.size()); }

    graph.create(700,1000, CV_8UC3);
    graph.setTo(Scalar(30,30,30));

    for (int i = 0; i< vp.size(); i++) {
        vMoments.push_back(computeMoments(vp[i]));

        if(verbose) { printf("m00: %Lf  m10:  %Lf  m01: %Lf \n", vMoments[i].m00,vMoments[i].m10,vMoments[i].m01); }
        if(verbose) { printf("u10: %Lf  u01:  %Lf  u11: %Lf  u20: %Lf  u02: %Lf\n",vMoments[i].u10,vMoments[i].u01,vMoments[i].u11,vMoments[i].u20,vMoments[i].u02 ); }
        if(verbose) { printf("phi1: %Lf  phi2: %Lf  \n", vMoments[i].phi1, vMoments[i].phi2); }

        int x2 = vMoments[i].m10/vMoments[i].m00 + 100* cos(vMoments[i].theta);
        int y2 = vMoments[i].m01/vMoments[i].m00 + 100* sin(vMoments[i].theta); 
        int x3 = vMoments[i].m10/vMoments[i].m00 - 100* cos(vMoments[i].theta);
        int y3 = vMoments[i].m01/vMoments[i].m00 - 100* sin(vMoments[i].theta); 
        
        line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x2,y2), Scalar(0,255,0),2 );
        line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x3,y3), Scalar(0,255,0),2 );

        circle(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), 5 ,Scalar(0,0,255),CV_FILLED,8,0);

        if(verbose) { printf("Theta: %Lf \n\n",vMoments[i].theta); }
        if(showImg) { showRegionGraph(vMoments[i].phi1, vMoments[i].phi2, graph); }

        // Pelota de golf (Adelante)
        if(vMoments[i].phi1 >= 0.15 && vMoments[i].phi1 <= 0.17 && vMoments[i].phi2 >= 0 && vMoments[i].phi2 <= 0.0005)
        {
            if(verbose) { printf("Pelota de golf (Adelante)\n"); }
            verAxisFig = 1;
            recognizedObjects++;
        }
        // Palo de golf ancho (Atras)
        else if(vMoments[i].phi1 >= 0.19 && vMoments[i].phi1 <= 0.21 && vMoments[i].phi2 >= 0.003 && vMoments[i].phi2 <= 0.01)
        {
            if(verbose) { printf("Palo de golf ancho (Atras)\n"); }
            verAxisFig = 2;
            recognizedObjects++;
        }

        // Golfista (derecha)
        else if(vMoments[i].phi1 >= 0.36 && vMoments[i].phi1 <= 0.45 && vMoments[i].phi2 >= 0.04 && vMoments[i].phi2 <= 0.1)
        {
            if(verbose) { printf("Golfista (derecha) con theta %Lf\n", vMoments[i].theta); }
            horAxisFig = 1;
            if(vMoments[i].theta >= 0)
            {
                if(verbose) { printf("Ir hacia abajo\n"); }
                movAction = 2;
            }
            else
            {
                if(verbose) { printf("Ir hacia arriba\n"); }
                movAction = 1;
            }

            recognizedObjects++;
        }

        // Palo de golf delgado (izquierda)
        else if(vMoments[i].phi1 >= 0.9 && vMoments[i].phi2 >= 0.9)
        {
            if(verbose) { printf("Palo de golf delgado (izquierda) con theta %Lf\n", vMoments[i].theta); }
            horAxisFig = 2;
            if(vMoments[i].theta >= 0)
            {
                if(verbose) { printf("Ir hacia abajo\n"); }
                movAction = 2;
            }
            else
            {
                if(verbose) { printf("Ir hacia arriba\n"); }
                movAction = 1;
            }

            recognizedObjects++;
        }

    }

    if(showImg) { imshow("Segmentacion",colormat); }
    if(showImg) { imshow("Grafica de regiones", graph); }
    
    waitKey(100);

    if(verbose) { printf("Recognized objects: %d\n", recognizedObjects); }

    return recognizedObjects;
}

void genPathPivots(Mat &sourceImage, Mat &binImage,int piv_num, vector<Point> &piv_coords){
  int const map_offset = 20;
  int const middle_padding = 70;
  int x_map_max = sourceImage.size().width ;
  int y_map_max = sourceImage.size().height ;
  int x_map_min = map_offset;
  int y_map_min = map_offset;
      
  // generate random seed
  srand(time(NULL));

  // generate pivots and draw them
  for(int i = 0; i < piv_num - 2; i++){ // two pivots are already start point and end point
    int temp_x = rand()%(x_map_max - x_map_min) + x_map_min - map_offset;
    int temp_y = rand()%(y_map_max - y_map_min) + y_map_min - map_offset;

    // check if possition is not obstacle
    int val = binImage.at<uint8_t>(temp_y,temp_x);
    if(val == 0 ){
      // insert the piv coordenates
      piv_coords.push_back(Point(temp_x,temp_y)); 
      // draw pivs on the source image
      circle(sourceImage, Point(temp_x,temp_y), 4 ,Scalar(0,0,255),CV_FILLED,8,0);
    }
  }
}
int getDistance(Point p1, Point p2){
  return sqrt(pow(p1.x - p2.x,2)+ pow(p1.y - p2.y,2));
}

bool no_obstacle(Mat const &sourceImage,Mat const &binImage,Point p1, Point p2){
  
  Mat tempsow = sourceImage.clone();

  LineIterator it(binImage, p1, p2, 8);
  for(int i = 0; i < it.count; i++, ++it){
      Point pt= it.pos(); 
      circle(tempsow, pt, 2 ,Scalar(0,0,255),CV_FILLED,8,0);
      if(binImage.at<uint8_t>(pt.y,pt.x) > 0)
        return false;
  }
  // imshow("lineas",tempsow);
  // waitKey(5);

  return true;
}

bool distanceSort(pair<int,Point> p1, pair<int,Point> p2){
  return (p1.first < p2.first);
}

void genGraph(Mat const &sourceImage, Mat const &binImage, vector<Point> &piv_coords, int nn){
  // Mat drawImg = sourceImage.clone();
  Mat drawImg = sourceImage;

  // Adyacences matrix
  vector< vector<long> > mDistances;
  // initialize with MAX
  for(int i = 0; i < piv_coords.size(); i++){
    vector<long> temp;
    for(int j = 0; j < piv_coords.size(); j++){
      if(i == j){
        temp.push_back(0);
      }else{
        temp.push_back(INF);
      }
    }
    mDistances.push_back(temp);
  }

  // Compute the nn closest distances to each node
  for (int i = 0; i < piv_coords.size(); ++i){
    // Distance, Point
    vector<pair<int,Point> > nodes; 

    // find Distances to all other points
    for(int j = 0; j < piv_coords.size(); j++){
      // no obstacles between points
      if(no_obstacle(sourceImage, binImage,piv_coords[i], piv_coords[j]) && i != j){
        int distance = getDistance(piv_coords[i],piv_coords[j]);
        nodes.push_back(make_pair(distance,piv_coords[j]));
      }
    }
    // sort nodes by distance
    sort(nodes.begin(),nodes.end(),distanceSort);;
    
    // Add the n first points to the Adyacent matrix and draw connections
    for(int k = 0; k < nn ; k++){
      if(k < nodes.size()){
        // Draw in the image
        line(drawImg, piv_coords[i], nodes[k].second ,  Scalar(255,0,0),1);
      }
    }
    nodes.clear();

  }

  // prinf ady matrix
  // printf("Adyacences matrix\n");
  // for(int i = 0; i < mDistances.size(); i++){
  //  for(int j = 0; j < mDistances.size(); j++){
  //    printf(" %ld ", mDistances[i][j]);
  //  }
  //  printf("\n");
  // }
  imshow("Graph",drawImg);

}

vector<Point> my_dijkstra(Mat const &sourceImage, Mat const &binImage,vector<Point> piv_coords,int nn,int side_selection){
  
  int const middle_padding = 30;
  int x_map_max = sourceImage.size().width ;
  int y_map_max = sourceImage.size().height ;
  int const startnode = 0;
  // Adyacences matrix
  vector< vector<long> > mDistances;
  // initialize with MAX
  for(int i = 0; i < piv_coords.size(); i++){
    vector<long> temp;
    for(int j = 0; j < piv_coords.size(); j++){
      if(i == j){
        temp.push_back(0);
      }else{
        temp.push_back(INF);
      }
    }
    mDistances.push_back(temp);
  }

  // Compute the nn closest distances to each node
  for (int i = 0; i < piv_coords.size(); ++i){
    // Distance, Point
    vector<pair<int,Point> > nodes; 

    // find Distances to all other points
    for(int j = 0; j < piv_coords.size(); j++){
      // no obstacles between points
      if(no_obstacle(sourceImage, binImage,piv_coords[i], piv_coords[j]) && i != j){
        int distance = getDistance(piv_coords[i],piv_coords[j]);
        nodes.push_back(make_pair(distance,piv_coords[j]));
      }
    }
    // sort nodes by distance
    sort(nodes.begin(),nodes.end(),distanceSort);


    // Add the n first points to the Adyacent matrix and draw connections
    for(int k = 0; k < nn ; k++){
      if(k < nodes.size()){
        vector<Point>::iterator it;
        // search original order element
        it = find(piv_coords.begin(), piv_coords.end(),nodes[k].second);
        // fill the matriz;
        if(side_selection == GOING_RIGHT){
          if(nodes[k].second.x > x_map_max/2 + middle_padding){
            mDistances[i][distance(piv_coords.begin(),it)] = nodes[k].first; 
            mDistances[distance(piv_coords.begin(),it)][i] = nodes[k].first;
          }
        }
        else if(side_selection == GOING_LEFT){
          if(nodes[k].second.x < x_map_max/2 - middle_padding){
            mDistances[i][distance(piv_coords.begin(),it)] = nodes[k].first; 
            mDistances[distance(piv_coords.begin(),it)][i] = nodes[k].first;  
          }
        }
        else{
          mDistances[i][distance(piv_coords.begin(),it)] = nodes[k].first; 
          mDistances[distance(piv_coords.begin(),it)][i] = nodes[k].first;          
        }
      }
    }
    nodes.clear();
  }


  // path do destiny
  vector<Point> vPath;
  // copy Ady Mat
  vector< vector<long> > G= mDistances;
  int n = G.size();

    long cost[MAX_ARRAY][MAX_ARRAY],distance[MAX_ARRAY],pred[MAX_ARRAY];
    long visited[MAX_ARRAY],count,mindistance,nextnode,i,j;
    
    //pred[] stores the predecessor of each node
    //count gives the number of nodes seen so far
    //create the cost matrix
    for(i=0;i<n;i++)
        for(j=0;j<n;j++)
            if(G[i][j]==0)
                cost[i][j]=INF;
            else
                cost[i][j]=G[i][j];
    
    //initialize pred[],distance[] and visited[]
    for(i=0;i<n;i++)
    {
        distance[i]=cost[startnode][i];
        pred[i]=startnode;
        visited[i]=0;
    }
    
    distance[startnode]=0;
    visited[startnode]=1;
    count=1;
    
    while(count<n-1)
    {
        mindistance=INF;
        
        //nextnode gives the node at minimum distance
        for(i=0;i<n;i++)
            if(distance[i]<mindistance&&!visited[i])
            {
                mindistance=distance[i];
                nextnode=i;
            }
            
            //check if a better path exists through nextnode            
            visited[nextnode]=1;
            for(i=0;i<n;i++)
                if(!visited[i])
                    if(mindistance+cost[nextnode][i]<distance[i])
                    {
                        distance[i]=mindistance+cost[nextnode][i];
                        pred[i]=nextnode;
                    }
        count++;
    }
 
    // print the path and distance of each node
    for(i=0;i<2;i++)
        if(i!=startnode){
            // printf("\nPath=%ld",i); 
            vPath.push_back(piv_coords[1]) ; 
            j=i;
            do{
                j=pred[j];
                // insert the nodes into path vec
                vPath.push_back(piv_coords[j]);
                // printf("<-%ld",j);
            }while(j!=startnode);
    }

    return vPath;

}

#endif