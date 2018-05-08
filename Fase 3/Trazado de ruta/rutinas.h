#define X_MAX 320
#define Y_MAX 240
#define MAX_ARRAY 300
#define INF 9999
#define GOING_LEFT 0
#define GOING_RIGHT 1
#define GOING_NORMAL 2


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
using namespace std;
using namespace cv;



void gray2threshold(const Mat &sourceImage, Mat &binImage, uint8_t threshold_value){
  /* 
  	Last param is type
  	0: Binary
    1: Binary Inverted
    2: Threshold Truncated
    3: Threshold to Zero
    4: Threshold to Zero Inverted
   */

  threshold( sourceImage, binImage, threshold_value, 255,1 );
}



void genPathPivots(Mat &sourceImage, Mat &binImage,int piv_num, vector<Point> &piv_coords, int side_selection){
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
			if(side_selection == GOING_LEFT){
				if(temp_x < x_map_max/2 - middle_padding)
					piv_coords.push_back(Point(temp_x,temp_y));
			}
			else if(side_selection == GOING_RIGHT){
				if(temp_x > x_map_max/2+ middle_padding)
					piv_coords.push_back(Point(temp_x,temp_y));
			}
			else{
				piv_coords.push_back(Point(temp_x,temp_y));	
			}
			
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

vector< vector<long> > genGraph(Mat const &sourceImage, Mat const &binImage, vector<Point> piv_coords, int nn){
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
				vector<Point>::iterator it;
				// search original order element
				it = find(piv_coords.begin(), piv_coords.end(),nodes[k].second);
				// fill the matriz;
				mDistances[i][distance(piv_coords.begin(),it)] = nodes[k].first; 
				mDistances[distance(piv_coords.begin(),it)][i] = nodes[k].first;
				// Draw in the image
				line(drawImg, piv_coords[i], nodes[k].second ,  Scalar(255,0,0),1);
			}
		}
		nodes.clear();

	}

	// prinf ady matrix
	// printf("Adyacences matrix\n");
	// for(int i = 0; i < mDistances.size(); i++){
	// 	for(int j = 0; j < mDistances.size(); j++){
	// 		printf(" %ld ", mDistances[i][j]);
	// 	}
	// 	printf("\n");
	// }
	imshow("Graph",drawImg);

	return mDistances;
}

vector<Point> my_dijkstra(vector<vector <long> > G,vector<Point> vec_pivots,int startnode){
 	
 	// path do destiny
 	vector<Point> vPath;

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
            vPath.push_back(vec_pivots[1]) ; 
            j=i;
            do{
                j=pred[j];
                // insert the nodes into path vec
                vPath.push_back(vec_pivots[j]);
                // printf("<-%ld",j);
            }while(j!=startnode);
    }

    return vPath;

}