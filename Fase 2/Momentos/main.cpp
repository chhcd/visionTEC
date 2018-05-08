#include <iostream>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <climits>
#include "rutinas.h"
#define PI 3.14159265
using namespace std;
using namespace cv;

typedef long double ldd;

ldd getOrdinaryMoment(vector<Point>, int , int );
ldd getCentralizedMoment(vector<Point>, int,  ldd, int, ldd);
rMoments computeMoments(vector<Point>);

int main(){

	VideoCapture cap(0);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 480);

	if(!cap.isOpened())
		return -1;

	while(1){
		// Mat img = imread("muestra0.jpg",CV_LOAD_IMAGE_GRAYSCALE );
		Mat img = imread("muestra15.jpg");
		Mat frame;
		Mat binImage;
		Mat colormat;
		Mat yiq;
		Mat yiqFilter;
		int rYIQ[6] = {63,123,140,200,150,210};
		int rBGR[6] = {40,100,140,200,150,210};
		
		// cap >> frame;
		// cvtColor(frame, img, CV_BGR2GRAY);

		// color2yiq(frame,yiq);
		colorFilter(img,yiqFilter,rBGR);
		cvtColor(yiqFilter, yiqFilter, CV_BGR2GRAY);


		// imshow("img", yiqFilter);
		
		imshow("img", img);
		// color2yiq(img,yiq);

		
		gray2threshold(yiqFilter,binImage,30);
		imshow("bin",binImage);
		
		vector<vector<Point> > vp;
		// each region is a vector of Point
		vp = mycontours(binImage,50,colormat);

		// vector to store moments
		vector<rMoments> vMoments;


		printf("Number of regions: %lu \n\n", vp.size()); 


		for (int i = 0; i< vp.size(); i++){
			vMoments.push_back(computeMoments(vp[i]));

			printf("m00: %Lf  m10:  %Lf  m01: %Lf \n", vMoments[i].m00,vMoments[i].m10,vMoments[i].m01);
			printf("u10: %Lf  u01:  %Lf  u11: %Lf  u20: %Lf  u02: %Lf\n",vMoments[i].u10,vMoments[i].u01,vMoments[i].u11,vMoments[i].u20,vMoments[i].u02 );
			printf("phi1: %Lf  phi2: %Lf  \n", vMoments[i].phi1, vMoments[i].phi2);

			int x2 = vMoments[i].m10/vMoments[i].m00 + 100* cos(vMoments[i].theta);
	 		int y2 = vMoments[i].m01/vMoments[i].m00 + 100* sin(vMoments[i].theta); 
			int x3 = vMoments[i].m10/vMoments[i].m00 - 100* cos(vMoments[i].theta);
	 		int y3 = vMoments[i].m01/vMoments[i].m00 - 100* sin(vMoments[i].theta); 
	 		
	 		line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x2,y2), Scalar(0,255,0),2 );
	 		line(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), Point(x3,y3), Scalar(0,255,0),2 );

			printf("Theta: %Lf \n\n",vMoments[i].theta); 
			circle(colormat, Point(vMoments[i].m10/vMoments[i].m00, vMoments[i].m01/vMoments[i].m00), 5 ,Scalar(0,0,255),CV_FILLED,8,0);

		}

		// // wait to close the program

		// imshow("color", colormat);
		uint8_t k = '0';
		k = waitKey(100);
		if(k == 'q')
			break;
		// while(k != 27){
		// 	k = waitKey(3);
		// }
	}

	destroyAllWindows();

	return 0;
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
