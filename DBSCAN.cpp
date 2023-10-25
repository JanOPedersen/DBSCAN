// DBSCAN.cpp : main project file.

#include "stdafx.h"
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <kdtree.h>

using namespace System;
using namespace std;

ofstream dbscan_log;

/*class Point
{
public: static const int NOISE = -1;
public: static const int UNCLASSIFIED = 0;
public: double X, Y;
	int ClusterId = 0;
public: Point(double x, double y)
	{
		this->X = x;
		this->Y = y;
	}
public: void PrintPoint()
	{
		printf("(%f, %f) ", X, Y);
	}
public: static double DistanceSquared(Point p1, Point p2)
	{
		double diffX = p2.X - p1.X;
		double diffY = p2.Y - p1.Y;
		return diffX * diffX + diffY * diffY;
	}
};*/

bool operator==(Point p1, Point p2){
    return (p1.X == p2.X && p1.Y == p2.Y);
}

struct kdtree;
extern vector<list<Point*>> GetClusters(list<Point*>& points, double eps, int minPts);
extern bool ExpandCluster(list<Point*>& points, Point* p, int clusterId, double eps, int minPts);
extern std::list<Point*>* kd_data_list(struct kdres *rset);

kdtree *kd; // The KD tree

class DBSCAN
{
public:
	

	static void Main()
	{
		dbscan_log.open("DBSCAN_official.txt");

		list<Point*> points;
		kd = kd_create(2);
		// sample data
		points.push_back(&Point(0.0, 100.0));
		points.push_back(&Point(0.0, 200.0));
		points.push_back(&Point(0.0, 275.0));
		points.push_back(&Point(100.0, 150.0));
		points.push_back(&Point(200.0, 100.0));
		points.push_back(&Point(250.0, 200.0));
		points.push_back(&Point(0.0, 300.0));
		points.push_back(&Point(100.0, 200.0));
		points.push_back(&Point(600.0, 700.0));
		points.push_back(&Point(650.0, 700.0));
		points.push_back(&Point(675.0, 700.0));
		points.push_back(&Point(675.0, 710.0));
		points.push_back(&Point(675.0, 720.0));
		points.push_back(&Point(50.0, 400.0));
		double eps = 100.0; // epsilon from definition 13.1
		int minPts = 3; // Mmin from definition 13.2

		// Insert points into the KD tree
		list<Point*>::iterator i;
		for (i = points.begin(); i != points.end(); ++i)
		{
			double p[] = { (*i)->X, (*i)->Y };
			kd_insert(kd, p, *i);
		};

		// Print points to console
		system("cls");
		printf("The %i points are :\n\n", points.size());
		for (i = points.begin(); i != points.end(); ++i)
		{
			(*i)->PrintPoint();
		};
		printf("\n\n");

		//dbscan_log << "------------------------------------------" << endl;

		vector<list<Point*>> clusters = GetClusters(points, eps, minPts);
		

		//return;

		// Print points to console
		printf("The %i points are :\n\n",points.size());
		for (i = points.begin(); i != points.end(); ++i)
		{
			(*i)->PrintPoint();
		};
		printf("\n\n");
		int total = 0;

		for (int i = 0; i < clusters.size(); i++)
		{
			int count = clusters[i].size();
			total += count;
			printf("\nCluster %i consists of the following %i point(s) :\n\n",i + 1, count);
			list<Point*>::iterator j;
			for (j = clusters[i].begin(); j != clusters[i].end(); ++j){
				(*j)->PrintPoint();
			};
			printf("\n");
		}
		// print any points which are NOISE
		total = points.size() - total;
		if (total > 0)
		{
			printf("\nThe following %i point(s) is/are NOISE :\n\n", total);
			list<Point*>::iterator i;
			for (i = points.begin(); i != points.end(); ++i){
				if ((*i)->ClusterId == Point::NOISE) {
					(*i)->PrintPoint();
				}
			}
		}
		else
		{
			printf("\nNo points are NOISE\n");
		}
		_getch();
	}
};

bool compare_points(Point* first,Point* second){
	return first->ClusterId < second->ClusterId;
};

vector<list<Point*>> GetClusters(list<Point*>& points, double eps, int minPts)
{
	if (points.size() == 0){
		return vector<list<Point*>>(0);
	};
	vector<list<Point*>> clusters = vector<list<Point*>>(0);
	eps *= eps; // square eps
	int clusterId = 1;
	list<Point*>::iterator i;
	for (i = points.begin(); i != points.end(); ++i)
	{
		Point* p = *i;
		if (p->ClusterId == Point::UNCLASSIFIED)
		{
			// log the found points
			// NOTE: 15-12-2016, GetRegion(...) looks like it works
			list<Point*>::iterator i0;
			dbscan_log << "---------------------- Before ExpandCluster" << endl;
			for (i0 = points.begin(); i0 != points.end(); ++i0)
			{
				dbscan_log << (*i0)->X << " " << (*i0)->Y << " ClusterID: " << (*i0)->ClusterId << endl;
			}

			if (ExpandCluster(points, p, clusterId, eps, minPts)) clusterId++;

			// log the found points
			// NOTE: 15-12-2016, GetRegion(...) looks like it works
			list<Point*>::iterator i1;
			dbscan_log << "---------------------- After ExpandCluster" << endl;
			for (i1 = points.begin(); i1 != points.end(); ++i1)
			{
				dbscan_log << (*i1)->X << " " << (*i1)->Y << " ClusterID: " << (*i1)->ClusterId << endl;
			}
		}
	}
	// sort out points into their clusters, if any
	points.sort(compare_points);
	int maxClusterId = points.back()->ClusterId;
	if (maxClusterId < 1) return clusters; // no clusters, so list is empty
	list<Point*> null_list;
	for (int i = 0; i < maxClusterId; i++) clusters.push_back(null_list);

	for (i = points.begin(); i != points.end(); ++i){
		if ((*i)->ClusterId>0) clusters[(*i)->ClusterId-1].push_back(*i);
	}

	return clusters;
}

list<Point*> GetRegion(list<Point*>& points, Point* p, double eps){
	
	// Use KD tree for the query, so only traverse part of the KD tree
	kdres *result_set;
	double pt[] = { p->X, p->Y };
	result_set = kd_nearest_range(kd, pt, sqrt(eps));
	list<Point*>* res = kd_data_list(result_set);

	// log the found points
	list<Point*>::iterator i;
	dbscan_log << "GetRegion for (" << p->X << " " << p->Y << ")" << endl;
	for (i = res->begin(); i != res->end(); ++i)
	{
		dbscan_log << (*i)->X << " " << (*i)->Y << " ClusterID: " << (*i)->ClusterId << endl;
	}

	return *res;
};

bool ExpandCluster(list<Point*>& points, Point* p, int clusterId, double eps, int minPts)
{
	list<Point*> seeds = GetRegion(points, p, eps);

	dbscan_log << "minPts: " << minPts << " seeds.size: " << seeds.size() << endl;

	if (seeds.size() < minPts) // no core point
	{
		p->ClusterId = Point::NOISE;
		return false;
	}
	else // all points in seeds are density reachable from point 'p'
	{
		list<Point*>::iterator i;
		for (i = seeds.begin(); i != seeds.end(); ++i){
			(*i)->ClusterId = clusterId;
		};

		dbscan_log << "p: " << p->X << " " << p->Y << " seeds size: " << seeds.size() << endl;

		seeds.remove(p);

		// log seeds
		// NOTE: 15-12-2016, GetRegion(...) looks like it works
		//list<Point*>::iterator i;
		dbscan_log << "seeds: " << endl;
		for (i = seeds.begin(); i != seeds.end(); ++i)
		{
			dbscan_log << (*i)->X << " " << (*i)->Y << "  ClusterId: " << (*i)->ClusterId << endl;
		}

		while (seeds.size() > 0)
		{
			Point* currentP=*seeds.begin();
			list<Point*> result = GetRegion(points, currentP, eps);

			dbscan_log << "Seed size: " << seeds.size() << " result size: " << result.size() << endl;

			if (result.size() >= minPts)
			{
				list<Point*>::iterator i;
				for (i = result.begin(); i != result.end(); ++i){
					Point* resultP=*i;

					dbscan_log << "resultP->ClusterId: " << resultP->ClusterId << endl;

					if (resultP->ClusterId == Point::UNCLASSIFIED || resultP->ClusterId == Point::NOISE){
						if (resultP->ClusterId == Point::UNCLASSIFIED){

							

							seeds.push_back(resultP);
							resultP->ClusterId = clusterId;
						}
					}
				}
			};
			dbscan_log << "currentP: " << currentP->X << " " << currentP->Y << " seeds size: " << seeds.size() << endl;
			seeds.remove(currentP);
			dbscan_log << " seeds size: " << seeds.size() << endl;
		};
		return true;
	}
	return true;
}

int main(int argc, char** argv)
{
	DBSCAN::Main();
	//_getch();
    return 0;
}
