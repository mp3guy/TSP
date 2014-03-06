/*
 * tsp.h
 *
 *  Created on: 13 Sep 2010
 *      Author: thomas
 */

#include "GeneticEvolution.h"
#include <opencv2/opencv.hpp>
#include "stdlib.h"
#include <boost/thread.hpp>

using namespace std;

const int NUM_THREADS = sysconf(_SC_NPROCESSORS_ONLN) * 2;

IplImage * townMap;
IplImage * bestSoFar;

CvFont font;

GeneticEvolution * myGeneticEvolvers;

vector<town> fullTownList;

solution * solutions;

bool algorithmBegun = false;

void init();
void onMouse(int event, int x, int y, int flags, void * param);
void drawNextSolution(solution * next);
