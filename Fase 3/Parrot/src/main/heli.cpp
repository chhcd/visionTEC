#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "SDL/SDL.h"

#include <stdlib.h>
#include "CHeli.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

/* Developed routines*/
#include "rutinasVision.h"

using namespace std;
using namespace cv;

/* Defining the drone radius */
int drone_rad = 25;

/* Declaring all the image matrices */
Mat bgrImage = Mat(240, 320, CV_8UC3);
Mat obsRealImage = imread("../src/imagen_obstaculos.png");
Mat drone = imread("../src/parrot.jpg");
Mat obsWidenImage = obsRealImage.clone();
Mat bHistogram;
Mat gHistogram;
Mat rHistogram;
Mat bgrFilter;
Mat hsvImage;
Mat hHistogram;
Mat sHistogram;
Mat vHistogram;
Mat hsvFilter;
Mat yiqImage;
Mat yHistogram;
Mat iHistogram;
Mat qHistogram;
Mat yiqFilter;
Mat grayImage;
Mat binImage;
Mat snapedMat;
Mat grayObstacles;
Mat binObstacles;
Mat routedImageEP1;
Mat routedImageEP2;

bool bSnap = false;
bool firstClick = true;
int rObjPoints[] = {0,0,0,0};

/* Setting the pixel coordinates and values in the different color models */
int Px=0, Py=0;
int vB=0, vG=0, vR=0;
int vH=0, vS=0, vV=0;
int vY=0, vI=0, vQ=0;

/* Setting the color mode range for the filter
   It's divided by channel 0 min, channel 0 max, channel 1 min, channel 1 max, channel 2 min, channel 2 max */
int rBGR[] = {0, 255, 0, 255, 0, 255};
int rHSV[] = {0, 255, 0, 255, 0, 255};
int rYIQ[] = {0, 255, 0, 255, 0, 255};

// needed outside
bool bEnRGB = false;
bool bEnYIQ = false;
bool bEnHSV = false;
bool freezeImage = false;
bool bEnBinarization = false;
bool bContinue = true;
bool bCalibrationMode = false;
bool bObjectDetectionMode = false;
bool bRouteMode = false;
bool bBasicBin = false;
bool bSegment = false;
bool bSize = false;
bool bRoutes = false;
bool bManualMovement = true;

/* Slider values for binarization */
int sliderBinValue=0;
int const SLIDER_MAX = 255;


CRawImage *image;
CHeli *heli;
float pitch, roll, yaw, height;
int hover=0;
// Joystick related
SDL_Joystick* m_joystick;
bool useJoystick;
int joypadRoll, joypadPitch, joypadVerticalSpeed, joypadYaw;
bool navigatedWithJoystick, joypadTakeOff, joypadLand, joypadHover, joypadAutonomous;
string ultimo = "init";

vector<Point> vec_pivots_EP1;
vector<Point> vec_pivots_EP2;

Mat imagenClick;

// Convert CRawImage to Mat
void rawToMat( Mat &destImage, CRawImage* sourceImage)
{   
    uchar *pointerImage = destImage.ptr(0);
    
    for (int i = 0; i < 240*320; i++)
    {
        pointerImage[3*i] = sourceImage->data[3*i+2];
        pointerImage[3*i+1] = sourceImage->data[3*i+1];
        pointerImage[3*i+2] = sourceImage->data[3*i];
    }
}


void mCoordinatesComponentVal(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            Px=x;
            Py=y;
            
            vB = bgrImage.at<Vec3b>(y, x)[0];
            vG = bgrImage.at<Vec3b>(y, x)[1];
            vR = bgrImage.at<Vec3b>(y, x)[2];

            vH = hsvImage.at<Vec3b>(y, x)[0];
            vS = hsvImage.at<Vec3b>(y, x)[1];
            vV = hsvImage.at<Vec3b>(y, x)[2];

            vY = yiqImage.at<Vec3b>(y, x)[0];
            vI = yiqImage.at<Vec3b>(y, x)[1];
            vQ = yiqImage.at<Vec3b>(y, x)[2];

            // Print Coordinates and Values
            cout <<"Pos X: "<<Px<<" Pos Y: "<<Py<<endl;
            cout <<"Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
            cout <<"Valor HSV: ("<<vH<<","<<vS<<","<<vV<<")"<<endl;
            cout <<"Valor YIQ: ("<<vY<<","<<vI<<","<<vQ<<")"<<endl<<endl;

            break;
        case CV_EVENT_MOUSEMOVE:
            break;
        case CV_EVENT_LBUTTONUP:
            break;
        case CV_EVENT_RBUTTONDOWN:
            break;
    }
    
}

void mSnapObj(int event, int x, int y, int flags, void* param)
{
    int iDelta = 40;
    switch (event)
    {
        case CV_EVENT_LBUTTONDOWN:
            if(firstClick)
            {
                
                line(snapedMat, Point(x,y), Point(x,y+20), CV_RGB(255,0,255),1,8,0);
                line(snapedMat, Point(x,y), Point(x+20,y), CV_RGB(0,255,255),1,8,0);
                imshow("Snaped",snapedMat);
                firstClick = false;
                rObjPoints[0] = x;
                rObjPoints[1] = y;
                // Print Coordinates and Values
                waitKey(0);
            }
            else
            {
                line(snapedMat, Point(x,y), Point(x,y-20), CV_RGB(255,0,255),1,8,0);
                line(snapedMat, Point(x,y), Point(x-20,y), CV_RGB(0,255,255),1,8,0);
                imshow("Snaped",snapedMat);
                rObjPoints[2] = x;
                rObjPoints[3] = y;

                int av0,av1,av2;
                regionAvg(bgrImage,rObjPoints[0],rObjPoints[1],rObjPoints[2],rObjPoints[3],av0,av1,av2);
                cout <<"Valor PROMEDIO RGB: ("<<av0<<","<<av1<<","<<av2<<")"<<endl;

                rBGR[0] = av0 - iDelta < 0 ?    0 : av0 - iDelta;
                rBGR[1] = av0 + iDelta > 255 ?  255 : av0 + iDelta;
                rBGR[2] = av1 - iDelta < 0 ?    0 : av1 - iDelta;
                rBGR[3] = av1 + iDelta > 255 ?  255 : av1 + iDelta;
                rBGR[4] = av2 - iDelta < 0 ?    0 : av2 - iDelta;
                rBGR[5] = av2 + iDelta > 255 ?  255 : av2 + iDelta;

                if(bEnHSV){
                    regionAvg(hsvImage,rObjPoints[0],rObjPoints[1],rObjPoints[2],rObjPoints[3],av0,av1,av2);
                    cout <<"Valor PROMEDIO HSV: ("<<av0<<","<<av1<<","<<av2<<")"<<endl;

                    rHSV[0] = av0 - iDelta < 0 ?    0 : av0 - iDelta;
                    rHSV[1] = av0 + iDelta > 255 ?  255 : av0 + iDelta;
                    rHSV[2] = av1 - iDelta < 0 ?    0 : av1 - iDelta;
                    rHSV[3] = av1 + iDelta > 255 ?  255 : av1 + iDelta;
                    rHSV[4] = av2 - iDelta < 0 ?    0 : av2 - iDelta;
                    rHSV[5] = av2 + iDelta > 255 ?  255 : av2 + iDelta;
                }
                if(bEnYIQ){
                    regionAvg(yiqImage,rObjPoints[0],rObjPoints[1],rObjPoints[2],rObjPoints[3],av0,av1,av2);
                    cout <<"Valor PROMEDIO YIQ: ("<<av0<<","<<av1<<","<<av2<<")"<<endl;

                    rYIQ[0] = av0 - iDelta < 0 ?    0 : av0 - iDelta;
                    rYIQ[1] = av0 + iDelta > 255 ?  255 : av0 + iDelta;
                    rYIQ[2] = av1 - iDelta < 0 ?    0 : av1 - iDelta;
                    rYIQ[3] = av1 + iDelta > 255 ?  255 : av1 + iDelta;
                    rYIQ[4] = av2 - iDelta < 0 ?    0 : av2 - iDelta;
                    rYIQ[5] = av2 + iDelta > 255 ?  255 : av2 + iDelta;
                }

                firstClick = true;
                waitKey(0);
            }
            break;
    }

}

void widenObstacles(Mat sourceImage, Mat &destinationImage)
{
    // Delta it's a protection for the environment, for example, the drone wiggling
    int delta = 5;

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

void computeRoutes(Mat sourceImage, Mat binImage, Point endPoint, vector<Point> &vec_pivots) {

    // Add INITIAL POINT and FINAL POINT
    vec_pivots.clear();
    vec_pivots.push_back(endPoint); circle(sourceImage, endPoint, 10 ,Scalar(200,200,0),CV_FILLED,8,0);
    vec_pivots.push_back(START_POINT); circle(sourceImage, START_POINT, 10 ,Scalar(200,200,0),CV_FILLED,8,0);

    // pivoting the image
    genPathPivots(sourceImage,binImage,N_PIVOTS,vec_pivots);

    // creating the graph by joining the pivots, draws the graph 
    genGraph(sourceImage,binImage,vec_pivots, N_NEIGHBORS);
}

vector<Point> traceShortestPath(Mat &sourceImage, Mat &binImage, int side_selection, vector<Point> &vec_pivots)
{
    // Get the path 
    vector<Point> vPath = my_dijkstra(sourceImage,binImage,vec_pivots,N_NEIGHBORS, side_selection);
    
    int i;
    printf("(%d,%d)", vPath[0].x,vPath[0].y);
    for(i = 1; i < vPath.size(); i++){
        printf("->(%d,%d)", vPath[i].x,vPath[i].y);
        line(sourceImage, vPath[i], vPath[i-1],  Scalar(0,255,0),3);
    }

    printf("\n");

    return vPath;
}

void vuelo_Atras_Derecha() {

    //hover
    //heli->setAngles(pitch, roll, yaw, height, hover);
    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout<<"Eleva"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 1000.0, 1);
    usleep(800000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout <<"Derecha" << endl;
    heli->setAngles(0.0, 4500.0, 0.0, 0.0, 0.0);
    usleep(1500000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1500000);

    cout<<"Atras"<<endl;
    heli->setAngles(4000, 0.0, 0.0, 0.0, 0.0);
    usleep(2500000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout <<"Izquierda" << endl;
    heli->setAngles(0.0, -2000.0, 0.0, 0.0, 0.0);
    usleep(2800000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);


    cout << "land" << endl;
    heli->land();
    usleep(1000000);

}

void vuelo_Mitad_Derecha() {

    //hover
    //heli->setAngles(pitch, roll, yaw, height, hover);
    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout <<"Derecha" << endl;
    heli->setAngles(0.0, 3000.0, 0.0, 0.0, 0.0);
    usleep(2000000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1500000);

    cout<<"Atras"<<endl;
    heli->setAngles(3000, 0.0, 0.0, 0.0, 0.0);
    usleep(1600000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout <<"Izquierda" << endl;
    heli->setAngles(-500.0, -5000.0, 0.0, 0.0, 0.0);
    usleep(1000000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(2000000);

    cout << "land" << endl;
    heli->land();
    usleep(1000000);

}

void vuelo_Atras_Izquierda() {

    //hover
    //heli->setAngles(pitch, roll, yaw, height, hover);
    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout <<"Izquierda" << endl;
    heli->setAngles(0.0, -4300.0, 0.0, 0.0, 0.0);
    usleep(1900000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1500000);

    cout<<"Atras"<<endl;
    heli->setAngles(4000, 0.0, 0.0, 0.0, 0.0);
    usleep(2500000);


    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1500000);

    cout <<"Derecha" << endl;
    heli->setAngles(0.0, 3100.0, 0.0, 0.0, 0.0);
    usleep(2200000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);


    cout << "land" << endl;
    heli->land();
    usleep(1000000);

}

void vuelo_Mitad_Izquierda() {

   //hover
    //heli->setAngles(pitch, roll, yaw, height, hover);
    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);

    cout <<"Izquierda" << endl;
    heli->setAngles(0.0, -4300.0, 0.0, 0.0, 0.0);
    usleep(1900000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1500000);

    cout<<"Atras"<<endl;
    heli->setAngles(3000, 0.0, 0.0, 0.0, 0.0);
    usleep(1600000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1500000);

    cout <<"Derecha" << endl;
    heli->setAngles(0.0, 3100.0, 0.0, 0.0, 0.0);
    usleep(2200000);

    cout<<"hover"<<endl;
    heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
    usleep(1000000);


    cout << "land" << endl;
    heli->land();
    usleep(1000000);


}

void calibrationMode(uint8_t key)
{
    if(!freezeImage){
        //image is captured
        heli->renewImage(image);

        // Copy to OpenCV Mat
        rawToMat(bgrImage, image);

        //// TODO CHANGE
        // bgrImage = imread("../fotosVision/muestra29.jpg", CV_LOAD_IMAGE_COLOR);

        /* Obtain a new frame from camera */
        //camera.read(bgrImage);

        /* Calling routines to convert color spaces*/
        color2gray(bgrImage,grayImage);
        color2yiq(bgrImage,yiqImage);
        cvtColor(bgrImage,hsvImage,CV_BGR2HSV);
    }

    /* Show images */
    if (bEnRGB)
    {
        namedWindow("RGB");
        setMouseCallback("RGB", mCoordinatesComponentVal);
        imshow("RGB", bgrImage);

        imageHistogram(bgrImage, bHistogram, "RGB", 0);
        Mat overlayB;
        bHistogram.copyTo(overlayB);
        rectangle(overlayB, Rect(2*vB,0,2,400), Scalar(255,0,0), -1);
        rectangle(overlayB, Rect(2*vB,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayB, 0.7, bHistogram, 0.3, 0, bHistogram);
        namedWindow("Blue Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Blue Histogram", bHistogram);

        imageHistogram(bgrImage, gHistogram, "RGB", 1);
        Mat overlayG;
        gHistogram.copyTo(overlayG);
        rectangle(overlayG, Rect(2*vG,0,2,400), Scalar(0,255,0), -1);
        rectangle(overlayG, Rect(2*vG,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayG, 0.7, gHistogram, 0.3, 0, gHistogram);
        namedWindow("Green Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Green Histogram", gHistogram);

        imageHistogram(bgrImage, rHistogram, "RGB", 2);
        Mat overlayR;
        rHistogram.copyTo(overlayR);
        rectangle(overlayR, Rect(2*vR,0,2,400), Scalar(0,0,255), -1);
        rectangle(overlayR, Rect(2*vR,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayR, 0.7, rHistogram, 0.3, 0, rHistogram);
        namedWindow("Red Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Red Histogram", rHistogram);

        namedWindow("RGB Filter");
        colorFilter(bgrImage, bgrFilter, rBGR);
        createTrackbar( "B Min Value", "RGB Filter", &rBGR[0], SLIDER_MAX, NULL);
        createTrackbar( "B Max Value", "RGB Filter", &rBGR[1], SLIDER_MAX, NULL);
        createTrackbar( "G Min Value", "RGB Filter", &rBGR[2], SLIDER_MAX, NULL);
        createTrackbar( "G Max Value", "RGB Filter", &rBGR[3], SLIDER_MAX, NULL);
        createTrackbar( "R Min Value", "RGB Filter", &rBGR[4], SLIDER_MAX, NULL);
        createTrackbar( "R Max Value", "RGB Filter", &rBGR[5], SLIDER_MAX, NULL);
        imshow("RGB Filter", bgrFilter);
    }

    if (bEnHSV)
    {
        namedWindow("HSV");
        setMouseCallback("HSV", mCoordinatesComponentVal);
        imshow("HSV", hsvImage);

        imageHistogram(hsvImage, hHistogram, "HSV", 0);
        Mat overlayH;
        hHistogram.copyTo(overlayH);
        rectangle(overlayH, Rect(2*vH,0,2,400), Scalar(255,0,0), -1);
        rectangle(overlayH, Rect(2*vH,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayH, 0.7, hHistogram, 0.3, 0, hHistogram);
        namedWindow("Hue Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Hue Histogram", hHistogram);

        imageHistogram(hsvImage, sHistogram, "HSV", 1);
        Mat overlayS;
        sHistogram.copyTo(overlayS);
        rectangle(overlayS, Rect(2*vS,0,2,400), Scalar(0,255,0), -1);
        rectangle(overlayS, Rect(2*vS,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayS, 0.7, sHistogram, 0.3, 0, sHistogram);
        namedWindow("Saturation Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Saturation Histogram", sHistogram);

        imageHistogram(hsvImage, vHistogram, "HSV", 2);
        Mat overlayV;
        vHistogram.copyTo(overlayV);
        rectangle(overlayV, Rect(2*vV,0,2,400), Scalar(0,0,255), -1);
        rectangle(overlayV, Rect(2*vV,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayV, 0.7, vHistogram, 0.3, 0, vHistogram);
        namedWindow("Value Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Value Histogram", vHistogram);

        namedWindow("HSV Filter");
        colorFilter(hsvImage, hsvFilter, rHSV);
        createTrackbar( "H Min Value", "HSV Filter", &rHSV[0], SLIDER_MAX, NULL);
        createTrackbar( "H Max Value", "HSV Filter", &rHSV[1], SLIDER_MAX, NULL);
        createTrackbar( "S Min Value", "HSV Filter", &rHSV[2], SLIDER_MAX, NULL);
        createTrackbar( "S Max Value", "HSV Filter", &rHSV[3], SLIDER_MAX, NULL);
        createTrackbar( "V Min Value", "HSV Filter", &rHSV[4], SLIDER_MAX, NULL);
        createTrackbar( "V Max Value", "HSV Filter", &rHSV[5], SLIDER_MAX, NULL);
        imshow("HSV Filter", hsvFilter);
    }

    if (bEnYIQ)
    {
        namedWindow("YIQ");
        setMouseCallback("YIQ", mCoordinatesComponentVal);
        imshow("YIQ", yiqImage);

        imageHistogram(yiqImage, yHistogram, "YIQ", 0);
        Mat overlayY;
        yHistogram.copyTo(overlayY);
        rectangle(overlayY, Rect(2*vY,0,2,400), Scalar(255,0,0), -1);
        rectangle(overlayY, Rect(2*vY,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayY, 0.7, yHistogram, 0.3, 0, yHistogram);
        namedWindow("Luma Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Luma Histogram", yHistogram);

        imageHistogram(yiqImage, iHistogram, "YIQ", 1);
        Mat overlayI;
        iHistogram.copyTo(overlayI);
        rectangle(overlayI, Rect(2*vI,0,2,400), Scalar(0,255,0), -1);
        rectangle(overlayI, Rect(2*vI,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayI, 0.7, iHistogram, 0.3, 0, iHistogram);
        namedWindow("In-phase Histogram", CV_WINDOW_AUTOSIZE);
        imshow("In-phase Histogram", iHistogram);

        imageHistogram(yiqImage, qHistogram, "YIQ", 2);
        Mat overlayQ;
        qHistogram.copyTo(overlayQ);
        rectangle(overlayQ, Rect(2*vQ,0,2,400), Scalar(0,0,255), -1);
        rectangle(overlayQ, Rect(2*vQ,420,2,50), Scalar(255,255,255), -1);
        addWeighted(overlayQ, 0.7, qHistogram, 0.3, 0, qHistogram);
        namedWindow("Quadrature Histogram", CV_WINDOW_AUTOSIZE);
        imshow("Quadrature Histogram", qHistogram);

        namedWindow("YIQ Filter");
        colorFilter(yiqImage, yiqFilter, rYIQ);
        createTrackbar( "Y Min Value", "YIQ Filter", &rYIQ[0], SLIDER_MAX, NULL);
        createTrackbar( "Y Max Value", "YIQ Filter", &rYIQ[1], SLIDER_MAX, NULL);
        createTrackbar( "I Min Value", "YIQ Filter", &rYIQ[2], SLIDER_MAX, NULL);
        createTrackbar( "I Max Value", "YIQ Filter", &rYIQ[3], SLIDER_MAX, NULL);
        createTrackbar( "Q Min Value", "YIQ Filter", &rYIQ[4], SLIDER_MAX, NULL);
        createTrackbar( "Q Max Value", "YIQ Filter", &rYIQ[5], SLIDER_MAX, NULL);
        imshow("YIQ Filter", yiqFilter);
    }

    if (bEnBinarization)
    {
        /* Create Trackbar */
        gray2threshold(grayImage,binImage,sliderBinValue);
        namedWindow("Gray Binarization");
        createTrackbar( "Threshold", "Gray Binarization", &sliderBinValue, SLIDER_MAX, NULL);
        imshow("Gray", grayImage);
        imshow("Gray Binarization", binImage);
    }

    if(bSnap)
    {   

        colorFilter(bgrImage,snapedMat,rBGR);
        namedWindow("Snaped");
        setMouseCallback("Snaped",mSnapObj);
        imshow("Snaped",snapedMat);
    }

    switch (key)
    {
        case 'p':
            freezeImage = !freezeImage;
            break;
        case 'r':
            if(bEnRGB){ destroyWindow("RGB"); destroyWindow("Blue Histogram"); destroyWindow("Green Histogram"); destroyWindow("Red Histogram"); destroyWindow("RGB Filter"); }
            bEnRGB = !bEnRGB;
            break;
        case 'h':
            if(bEnHSV){ destroyWindow("HSV"); destroyWindow("Hue Histogram"); destroyWindow("Saturation Histogram"); destroyWindow("Value Histogram"); destroyWindow("HSV Filter"); }
            bEnHSV = !bEnHSV;
            break;
        case 'y':
            if(bEnYIQ){ destroyWindow("YIQ"); destroyWindow("Luma Histogram"); destroyWindow("In-phase Histogram"); destroyWindow("Quadrature Histogram"); destroyWindow("YIQ Filter"); }
            bEnYIQ = !bEnYIQ;
            break;
        case 'b':
            if(bEnBinarization){ destroyWindow("Gray"); destroyWindow("Gray Binarization"); }
            bEnBinarization = !bEnBinarization;
            break;

        case 'f':
            if(bSnap){ destroyWindow("Snaped"); }
            bSnap = !bSnap;
            break;
    }
}

void objectDetectionMode(uint8_t key)
{
    if(bManualMovement)
    {
        //// TODO TODO
        //image is captured
        heli->renewImage(image);

        // Copy to OpenCV Mat
        rawToMat(bgrImage, image);

        // bgrImage = imread("../fotosVision/muestra29.jpg", CV_LOAD_IMAGE_COLOR);

        imshow("Current image", bgrImage);

        switch (key)
        {
            // Parrot commands
            case 'a': yaw = -2000.0; break;
            case 'd': yaw = 2000.0; break;
            case 'w': height = -2000.0; break;
            case 's': height = 2000.0; break;
            case 't': heli->takeoff(); break;
            case 'e': heli->land(); break;
            case 'z': heli->switchCamera(0); break;
            case 'x': heli->switchCamera(1); break;
            case 'c': heli->switchCamera(2); break;
            case 'v': heli->switchCamera(3); break;
            case 'j': roll = -2000.0; break;
            case 'l': roll = 2000.0; break;
            case 'i': pitch = -2000.0; break;
            case 'k': pitch = 2000.0; break;
            case 'o': hover = (hover + 1) % 2; break;
            case 'm': bManualMovement = false; break;
            case 'b':
                if(bBasicBin){ destroyWindow("Filtro"); destroyWindow("Binarizado"); }
                    bBasicBin = !bBasicBin;
                break;
            case 'f':
                if(bSegment){ destroyWindow("Segmentacion"); destroyWindow("Grafica de regiones"); }
                    bSegment = !bSegment;
                break;
        }

        if(bBasicBin)
        {
            Mat bgrFilter;
            Mat binImage;

            GaussianBlur(bgrImage,bgrImage,Size(7,7), 0, 0);
            colorFilter(bgrImage,bgrFilter,rBGR);
            cvtColor(bgrFilter, bgrFilter, CV_BGR2GRAY);

            imshow("Filtro", bgrFilter);
            
            gray2threshold(bgrFilter,binImage,80);
            
            dilate(binImage, binImage, Mat(), Point(-1, -1), 1, 1, 1);
            imshow("Binarizado",binImage);
        }

        if(bSegment)
        {
            int horAxisFig;
            int verAxisFig;
            int movAction;

            segmentationAndClassification(bgrImage, rBGR, horAxisFig, verAxisFig, movAction, true, true);
        }

        if (useJoystick)
        {
            SDL_Event event;
            SDL_PollEvent(&event);

            joypadRoll = SDL_JoystickGetAxis(m_joystick, 2);
            joypadPitch = SDL_JoystickGetAxis(m_joystick, 3);
            joypadVerticalSpeed = SDL_JoystickGetAxis(m_joystick, 1);
            joypadYaw = SDL_JoystickGetAxis(m_joystick, 0);
            joypadTakeOff = SDL_JoystickGetButton(m_joystick, 1);
            joypadLand = SDL_JoystickGetButton(m_joystick, 2);
            joypadHover = SDL_JoystickGetButton(m_joystick, 0);
            joypadAutonomous = SDL_JoystickGetButton(m_joystick,3);
        }

        // prints the drone telemetric data, helidata struct contains drone angles, speeds and battery status
        printf("===================== Parrot Basic Example =====================\n\n");
        fprintf(stdout, "Angles  : %.2lf %.2lf %.2lf \n", helidata.phi, helidata.psi, helidata.theta);
        fprintf(stdout, "Speeds  : %.2lf %.2lf %.2lf \n", helidata.vx, helidata.vy, helidata.vz);
        fprintf(stdout, "Battery : %.0lf \n", helidata.battery);
        fprintf(stdout, "Hover   : %d \n", hover);
        fprintf(stdout, "Joypad  : %d \n", useJoystick ? 1 : 0);
        fprintf(stdout, "  Roll    : %d \n", joypadRoll);
        fprintf(stdout, "  Pitch   : %d \n", joypadPitch);
        fprintf(stdout, "  Yaw     : %d \n", joypadYaw);
        fprintf(stdout, "  V.S.    : %d \n", joypadVerticalSpeed);
        fprintf(stdout, "  TakeOff : %d \n", joypadTakeOff);
        fprintf(stdout, "  Land    : %d \n", joypadLand);
        fprintf(stdout, "Navigating with Joystick: %d \n", navigatedWithJoystick ? 1 : 0);
        cout<<"Pos X: "<<Px<<" Pos Y: "<<Py<<" Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
        

        if (joypadTakeOff) {
            heli->takeoff();
        }
        if (joypadLand) {
            heli->land();
        }

        bManualMovement = joypadAutonomous ? false : bManualMovement;
        
        hover = joypadHover ? !hover : hover;

        //setting the drone angles
        if (joypadRoll != 0 || joypadPitch != 0 || joypadVerticalSpeed != 0 || joypadYaw != 0)
        {
            heli->setAngles(joypadPitch/10, joypadRoll/5, joypadYaw/3, joypadVerticalSpeed/2, hover);
            navigatedWithJoystick = true;
        }
        else
        {
            heli->setAngles(pitch, roll, yaw, height, hover);
            navigatedWithJoystick = false;
        } 
    }
    else
    {
        int horAxisFig;
        int verAxisFig;
        int movAction;

        int recognizedObjects = segmentationAndClassification(bgrImage, rBGR, horAxisFig, verAxisFig, movAction, true, false);

        if(recognizedObjects != 2)
        {
            printf("Command not recognized, try again\n");
            bManualMovement = true;
        }
        else
        {
            printf("Modo automatico activado\n");
        }

        if(!bManualMovement)
        {
            // Movimientos del parrot

            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            //Pitch Atras
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"pitch"<<endl;
            heli->setAngles(3000, 0.0, 0.0, 0.0, 0.0);
            usleep(2500000);

            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);


            if (horAxisFig==1)
            {
                //Derecha
                //listo
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout <<" Roll right" << endl;
                heli->setAngles(0.0, 3000.0, 0.0, 0.0, 0.0);
                usleep(2500000);

            }

            else if (horAxisFig==2)
            {      
                //Izquierda
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Return" << endl;
                heli->setAngles(0.0, -3000.0, 0.0, 0.0, 0.0);
                usleep(2500000);
            }


            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            if (movAction==1)
            {
                //Elevar
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Lift" << endl;
                heli->setAngles(0.0, 0.0, 0.0, -17000.0, 0.0);
                usleep(3000000);
            }

            else if (movAction==2)
            {
                //Bajar
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Down" << endl;
                heli->setAngles(0.0, 0.0, 0.0, 9000.0, 0.0);
                usleep(3000000);
            }

            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            if (verAxisFig==1)
            {
                //Pitch Adelante
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout<<"pitch"<<endl;
                heli->setAngles(-3000, 0.0, 0.0, 0.0, 0.0);
                usleep(2500000);
            }
            else if (verAxisFig==2)
            {
                //Pitch Atras
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout<<"pitch"<<endl;
                heli->setAngles(3000, 0.0, 0.0, 0.0, 0.0);
                usleep(2500000);
            }

            
            //hover
            //heli->setAngles(pitch, roll, yaw, height, hover);
            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(4000000);

            cout << "land" << endl;
            heli->land();
            usleep(10000000);

            bManualMovement = true;

        }
    }
}

void routeMode(uint8_t key)
{   
    if(bManualMovement)
    {
        //// TODO TODO
        //image is captured
        heli->renewImage(image);

        // Copy to OpenCV Mat
        rawToMat(bgrImage, image);

        // bgrImage = imread("../fotosVision/muestra29.jpg", CV_LOAD_IMAGE_COLOR);

        imshow("Current image", bgrImage);

        switch (key)
        {
            // Parrot commands
            case 'a': yaw = -2000.0; break;
            case 'd': yaw = 2000.0; break;
            case 'w': height = -2000.0; break;
            case 's': height = 2000.0; break;
            case 't': heli->takeoff(); break;
            case 'e': heli->land(); break;
            case 'z': heli->switchCamera(0); break;
            case 'x': heli->switchCamera(1); break;
            case 'c': heli->switchCamera(2); break;
            case 'v': heli->switchCamera(3); break;
            case 'j': roll = -2000.0; break;
            case 'l': roll = 2000.0; break;
            case 'i': pitch = -2000.0; break;
            case 'k': pitch = 2000.0; break;
            case 'o': hover = (hover + 1) % 2; break;
            case 'm': bManualMovement = false; break;
            case 'b':
                if(bSize){ destroyWindow("Drone size"); }
                bSize = !bSize;
                break;
            case 'f':
                if(bRoutes){ destroyWindow("Graph"); }
                else
                {
                    color2gray(obsWidenImage,grayObstacles);
                    threshold(grayObstacles, binObstacles, 240, 255, 1);

                    routedImageEP1 = obsWidenImage.clone();
                    routedImageEP2 = obsWidenImage.clone();

                    // Call this function to perform the path from initial point to END_POINT_1 or END_POINT_2
                    computeRoutes(routedImageEP1,binObstacles,END_POINT_1, vec_pivots_EP1);
                    computeRoutes(routedImageEP2,binObstacles,END_POINT_2, vec_pivots_EP2);
                }

                bRoutes = !bRoutes;
                break;
        }

        if(bSize)
        {
            widenObstacles(obsRealImage, obsWidenImage);
            namedWindow("Drone size");
            createTrackbar( "Drone radius", "Drone size", &drone_rad, 50, NULL);
            imshow("Drone size", obsWidenImage);

            color2gray(obsWidenImage,grayObstacles);
            threshold(grayObstacles, binObstacles, 240, 255, 1);
            imshow("Binary environment", binObstacles);
        }

        if (useJoystick)
        {
            SDL_Event event;
            SDL_PollEvent(&event);

            joypadRoll = SDL_JoystickGetAxis(m_joystick, 2);
            joypadPitch = SDL_JoystickGetAxis(m_joystick, 3);
            joypadVerticalSpeed = SDL_JoystickGetAxis(m_joystick, 1);
            joypadYaw = SDL_JoystickGetAxis(m_joystick, 0);
            joypadTakeOff = SDL_JoystickGetButton(m_joystick, 1);
            joypadLand = SDL_JoystickGetButton(m_joystick, 2);
            joypadHover = SDL_JoystickGetButton(m_joystick, 0);
            joypadAutonomous = SDL_JoystickGetButton(m_joystick,3);
        }

        // prints the drone telemetric data, helidata struct contains drone angles, speeds and battery status
        printf("===================== Parrot Basic Example =====================\n\n");
        fprintf(stdout, "Angles  : %.2lf %.2lf %.2lf \n", helidata.phi, helidata.psi, helidata.theta);
        fprintf(stdout, "Speeds  : %.2lf %.2lf %.2lf \n", helidata.vx, helidata.vy, helidata.vz);
        fprintf(stdout, "Battery : %.0lf \n", helidata.battery);
        fprintf(stdout, "Hover   : %d \n", hover);
        fprintf(stdout, "Joypad  : %d \n", useJoystick ? 1 : 0);
        fprintf(stdout, "  Roll    : %d \n", joypadRoll);
        fprintf(stdout, "  Pitch   : %d \n", joypadPitch);
        fprintf(stdout, "  Yaw     : %d \n", joypadYaw);
        fprintf(stdout, "  V.S.    : %d \n", joypadVerticalSpeed);
        fprintf(stdout, "  TakeOff : %d \n", joypadTakeOff);
        fprintf(stdout, "  Land    : %d \n", joypadLand);
        fprintf(stdout, "Navigating with Joystick: %d \n", navigatedWithJoystick ? 1 : 0);
        cout<<"Pos X: "<<Px<<" Pos Y: "<<Py<<" Valor RGB: ("<<vR<<","<<vG<<","<<vB<<")"<<endl;
        
        if (joypadTakeOff) {
            heli->takeoff();
        }
        if (joypadLand) {
            heli->land();
        }

        bManualMovement = joypadAutonomous ? false : bManualMovement;
        
        // hover = joypadHover ? !hover : hover;

        hover = joypadHover ? 1 : 0;

        //setting the drone angles
        if (joypadRoll != 0 || joypadPitch != 0 || joypadVerticalSpeed != 0 || joypadYaw != 0)
        {
            heli->setAngles(joypadPitch/10, joypadRoll/5, joypadYaw/3, joypadVerticalSpeed/2, hover);
            navigatedWithJoystick = true;
        }
        else
        {
            heli->setAngles(pitch, roll, yaw, height, hover);
            navigatedWithJoystick = false;
        } 
    }
    else
    {
        int horAxisFig;
        int verAxisFig;
        int movAction;

        widenObstacles(obsRealImage, obsWidenImage);

        int recognizedObjects = segmentationAndClassification(bgrImage, rBGR, horAxisFig, verAxisFig, movAction, true, false);

        if(recognizedObjects != 2)
        {
            printf("Command not recognized, try again\n");
            bManualMovement = true;
        }
        else
        {
            printf("Modo automatico activado\n");
        }

        if(!bManualMovement)
        {
            // Movimientos del parrot

            cout<<"hover"<< endl;
            heli->setAngles(0.0, 0.0, 0.0, 0.0, 1);
            usleep(1500000);

            if (movAction==1)
            {
                //Elevar
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Lift" << endl;
                heli->setAngles(0.0, 0.0, 0.0, -2500.0, 1.0);
                usleep(1000000);
            }

            else if (movAction==2)
            {
                //Bajar
                //heli->setAngles(pitch, roll, yaw, height, hover);
                cout << "Down" << endl;
                heli->setAngles(0.0, 0.0, 0.0, 2500.0, 1.0);
                usleep(1000000);
            } 

            // En medio y derecha
            if(verAxisFig == 1 && horAxisFig == 1)
            {
                vector<Point> vPath = traceShortestPath(routedImageEP1,binImage,GOING_RIGHT,vec_pivots_EP1);
                imshow("Selected route", routedImageEP1);
                vuelo_Mitad_Derecha();
            }

            // En medio e izquierda
            else if(verAxisFig == 1 && horAxisFig == 2)
            {
                vector<Point> vPath = traceShortestPath(routedImageEP1,binImage,GOING_LEFT,vec_pivots_EP1);
                imshow("Selected route", routedImageEP1);
                vuelo_Mitad_Izquierda();
            }

            // Atras y derecha
            else if(verAxisFig == 2 && horAxisFig == 1)
            {
                vector<Point> vPath = traceShortestPath(routedImageEP2,binImage,GOING_RIGHT,vec_pivots_EP2);
                imshow("Selected route", routedImageEP2);
                vuelo_Atras_Derecha();
            }

            // Atras e izquierda
            else if(verAxisFig == 2 && horAxisFig == 2)
            {
                vector<Point> vPath = traceShortestPath(routedImageEP2,binImage,GOING_LEFT,vec_pivots_EP2);
                imshow("Selected route", routedImageEP2);
                vuelo_Atras_Izquierda();
            }

            bManualMovement = true;

        }
    }
}

void showCalibrationMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"1. Regresar al menu de funciones", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"p. Cambiar modo continuo / modo congelado", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"r. Imagen RGB", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"h. Imagen HSV", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"y. Imagen YIQ", Point(15,250) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"b. Imagen gris / binarizacion", Point(15,290) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"f. Snap Object", Point(15,330) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,370) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de calibracion", menu);
}

void showFunctionMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"1. Calibracion de camara y thresholds", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"2. Rutina deteccion de objetos y movimiento", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"3. Rutina planeacion y seguimiento de rutas", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de funciones", menu);
}

void showObjectDetectionMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"2. Regresar al menu de funciones", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"b. Binarizacion", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"f. Segmentacion de imagen", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"m. Iniciar rutina", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,250) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de deteccion de objetos", menu);
}


void showRouteMenu()
{
    /* Show interface menu to user*/
    Mat menu;
    menu.create(600,600,CV_8UC3);
    menu.setTo(Scalar(200,200,200));
    putText(menu,"Menu", Point(250,55) , FONT_HERSHEY_SIMPLEX, 1.5, Scalar(0,0,0), 2,8,false );
    putText(menu,"3. Regresar al menu de funciones", Point(15,90) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"b. Tamano del drone", Point(15,130) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"f. Planeacion de trayectorias", Point(15,170) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"m. Iniciar rutina", Point(15,210) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    putText(menu,"q. Terminar", Point(15,250) , FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0,0,0), 2,8,false );
    imshow("Menu de planeacion y seguimiento de rutas", menu);
}

// ------------------------------------------ //
//                  MAIN
// ------------------------------------------ //

int main(int argc,char* argv[])
{
    //establishing connection with the quadcopter
    heli = new CHeli();
    
    //this class holds the image from the drone 
    image = new CRawImage(320,240);
    
    // Initial values for control   
    pitch = roll = yaw = height = 0.0;
    joypadPitch = joypadRoll = joypadYaw = joypadVerticalSpeed = 0.0;

    // Initialize joystick
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    useJoystick = SDL_NumJoysticks() > 0;
    if (useJoystick)
    {
        SDL_JoystickClose(m_joystick);
        m_joystick = SDL_JoystickOpen(0);
    }

    /* Show interface menu to user*/
    showFunctionMenu();

    // showRegionGraph(0.38,0.07);

    while (bContinue)
    {
        uint8_t key = waitKey(3);
        switch (key)
        {
            /* Exit -> 'ESC' key or q key*/
            case 'q':
            case 27:
                cout << "Terminating program" << endl;
                bContinue = false;
                break;
            case '1':
                if(bCalibrationMode)
                {
                    bCalibrationMode = 0;
                    destroyWindow("Menu de calibracion");
                    showFunctionMenu();
                }
                else
                {
                    bCalibrationMode = 1;
                    destroyWindow("Menu de funciones");
                    showCalibrationMenu();
                }
                break;
            case '2':
                if(bObjectDetectionMode)
                {
                    bObjectDetectionMode = 0;
                    destroyWindow("Menu de deteccion de objetos");
                    showFunctionMenu();
                }
                else
                {
                    bObjectDetectionMode = 1;
                    destroyWindow("Menu de funciones");
                    showObjectDetectionMenu();
                }
                break;
            case '3':
                if(bRouteMode)
                {
                    bRouteMode = 0;
                    destroyWindow("Menu de planeacion y seguimiento de rutas");
                    showFunctionMenu();
                }
                else
                {
                    bRouteMode = 1;
                    destroyWindow("Menu de funciones");
                    showRouteMenu();
                }
                break;
        }

        if(bCalibrationMode) calibrationMode(key);
        if(bObjectDetectionMode) objectDetectionMode(key);
        if(bRouteMode) routeMode(key);

        usleep(15000);
    }
    
    heli->land();
    SDL_JoystickClose(m_joystick);
    delete heli;
    delete image;
    return 0;
}
