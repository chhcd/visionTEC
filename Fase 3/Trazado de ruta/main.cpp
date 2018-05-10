
#define PI 3.14159265
#define START_POINT 360,700
#define END_POINT_1 360,70
#define END_POINT_2 360,360
#define N_PIVOTS 150
#define N_NEIGHBORS 5

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <queue>
#include <time.h>
#include <map>
#include <set>
#include <climits>
#include <utility>
#include <algorithm>
#include <stdio.h>
#include <unistd.h>
// Our lib
#include "rutinas.h"

using namespace std;
using namespace cv;

Mat img = imread("imagen_obstaculos.png");
// Mat img = imread("test2.png");
Mat colormat;
Mat drone = imread("parrot.jpg");

vector<Point> vec_pivots;
// Adyacences matrix
vector< vector<long> > mDistances;

int drone_rad = 25;

void plotDrone(Mat sourceImage, Point coord){
	resize(drone, drone, Size(2*drone_rad, 2*drone_rad));
	Mat tempShow = sourceImage.clone();
	
	// inside limits, not needed when Dieguin lombrin is done
	if(coord.x -  drone_rad > 0 && coord.y - drone_rad > 0  && coord.x +  drone_rad < sourceImage.cols &&  coord.y +  drone_rad < sourceImage.rows)
		drone.copyTo(tempShow(Rect(coord.x -  drone_rad,coord.y - drone_rad ,drone.cols,drone.rows)));
	
	imshow("Drone location", tempShow);
}


void simulatePath(Mat sourceImage, vector<Point> &vPath){
	for(int i = 0; i < vPath.size(); i++){
		plotDrone(sourceImage, vPath[i]);
		printf("%d--------\n",i );
		char c = 'a';
		while(c != 'p'){
			c = waitKey(100);
		}

	}

	destroyWindow("Drone location");
}

void computeRoutes(Mat sourceImage, Mat binImage, int side_selection) {

    // Add INITIAL POINT and FINAL POINT
    vec_pivots.push_back(Point(START_POINT)); circle(sourceImage, Point(START_POINT), 10 ,Scalar(200,200,0),CV_FILLED,8,0);
    vec_pivots.push_back(Point(END_POINT_1)); circle(sourceImage, Point(END_POINT_1), 10 ,Scalar(200,200,0),CV_FILLED,8,0);

    // pivoting the image
    genPathPivots(sourceImage,binImage,N_PIVOTS,vec_pivots,side_selection);

    // creating the graph by joining the pivots, draws the graph and reuturns Ady. Matrix
    mDistances = genGraph(sourceImage,binImage,vec_pivots, N_NEIGHBORS);
}

vector<Point> traceShortestPath(Mat &sourceImage)
{
    // Get the path 
    vector<Point> vPath = my_dijkstra(mDistances,vec_pivots,0);
    
    int i;
    printf("(%d,%d)", vPath[0].x,vPath[0].y);
    for(i = 1; i < vPath.size(); i++){
        printf("->(%d,%d)", vPath[i].x,vPath[i].y);
        line(sourceImage, vPath[i], vPath[i-1],  Scalar(0,255,0),3);
    }

    printf("\n");

    return vPath;
}

void widenObstacles(Mat sourceImage, Mat &destinationImage)
{
	// Delta it's a protection for the environment, for example, the drone wiggling
	int delta = 5;

	if (destinationImage.empty())
		destinationImage = sourceImage.clone();

	rectangle(destinationImage, Rect(5,39,sourceImage.cols - 10,drone_rad + delta), Scalar(140,140,140), -1);
	rectangle(destinationImage, Rect(5,39,drone_rad + delta,sourceImage.rows - 39 - 5), Scalar(140,140,140), -1);
	rectangle(destinationImage, Rect(5,sourceImage.rows - drone_rad - delta - 5,sourceImage.cols - 10,drone_rad + delta), Scalar(140,140,140), -1);
	rectangle(destinationImage, Rect(sourceImage.cols - drone_rad - delta - 5,39,drone_rad + delta,sourceImage.rows - 39 - 5), Scalar(140,140,140), -1);

	circle(destinationImage, Point(356,276), drone_rad + delta + 17, Scalar(0,200,0), -1);
	circle(destinationImage, Point(356,536), drone_rad + delta + 17, Scalar(0,200,0), -1);
	circle(destinationImage, Point(356,276), 17, Scalar(0,255,0), -1);
	circle(destinationImage, Point(356,536), 17, Scalar(0,255,0), -1);
}

int main() {
	Mat binImage;
	Mat obsImage;
	Mat grayImage;

	imshow("obstacles", img);
	
	widenObstacles(img, obsImage);
    namedWindow("Drone size");
    createTrackbar( "Drone radius", "Drone size", &drone_rad, 50, NULL);
	imshow("Drone size", obsImage);

	color2gray(obsImage,grayImage);
	gray2threshold(grayImage,binImage,240);
	imshow("binImage",binImage);

	// Call this dunction to perform the path from initial point to END_POINT_1 or END_POINT_2
	// Last param could be: GOING_LEFT, GOING_RIGHT, GOING_NORMAL
	computeRoutes(obsImage,binImage,GOING_LEFT);

	vector<Point> vPath = traceShortestPath(obsImage);

	// Uncomment this line to simulate the path
	simulatePath(obsImage, vPath);
	
	// wait to close program, while loop is cuz my vm has a weird bounce
	char c = waitKey(100);
	while(c != 'q'){
		c = waitKey(100);
	}

	destroyAllWindows();
	
	return 0;
}