/*
 * tsp.cpp
 *
 *  Created on: 13 Sep 2010
 *      Author: thomas
 */

#include "tsp.h"

int main()
{
	init();

	cout << "Create 3 or more points\nThen press g to ACTIVATE THE EVOLUTION, or q to quit" << endl;

	double bestDist = 0;

	while(true)
	{
		char key = cvWaitKey(0);

		if(key == 'g')
		{
			if(fullTownList.size() > 2)
			{
				if(!algorithmBegun)
				{
					algorithmBegun = true;

					cout << sysconf(_SC_NPROCESSORS_ONLN) << " core(s), making " << NUM_THREADS << " evolvers" << endl;

					myGeneticEvolvers = new GeneticEvolution[NUM_THREADS];
					solutions = new solution[NUM_THREADS];

					for(int i = 0; i < NUM_THREADS; i++)
					{
						myGeneticEvolvers[i].setMyID(i);
						myGeneticEvolvers[i].setNumTowns((int)fullTownList.size());
						myGeneticEvolvers[i].setTownList(&fullTownList);

						if(i == 0)
						{
							myGeneticEvolvers[i].setGenerationSize(128);
							myGeneticEvolvers[i].setMutationChance(1);
						}
						else if(i == 1)
						{
							myGeneticEvolvers[i].setGenerationSize(128);
							myGeneticEvolvers[i].setMutationChance(2);
						}
						else if(i == 2)
						{
							myGeneticEvolvers[i].setGenerationSize(64);
							myGeneticEvolvers[i].setMutationChance(1);
						}
						else if(i == 3)
						{
							myGeneticEvolvers[i].setGenerationSize(64);
							myGeneticEvolvers[i].setMutationChance(5);
						}
						else if(i > 3)
						{
							int num = rand() % 3;
							myGeneticEvolvers[i].setGenerationSize((int)pow(2, num == 0 ? 5 : num == 1 ? 6 : 7) * 2);
							myGeneticEvolvers[i].setMutationChance(5);
						}

						solutions[i] = myGeneticEvolvers[i].generateInitialPopulation();
					}

					double bestVal = numeric_limits<double>::max();
					int bestEvolver = 0;

					for(int i = 0; i < NUM_THREADS; i++)
					{
						if(solutions[i].distance < bestVal)
						{
							bestVal = solutions[i].distance;
							bestEvolver = i;
						}
					}

					drawNextSolution(&solutions[bestEvolver]);

					memcpy(bestSoFar->imageData, townMap->imageData, townMap->width * townMap->height * townMap->nChannels);
					cvShowImage("Best", bestSoFar);
					bestDist = solutions[bestEvolver].distance;
				}
				else
				{
					while(true)
					{
						boost::thread_group myEvolvers;

						for(int i = 0; i < NUM_THREADS; i++)
						{
							int selectionVal = rand() % 2;
							boost::thread * newThread = new boost::thread(boost::bind(&GeneticEvolution::getNextGeneration, &myGeneticEvolvers[i], &solutions[i], selectionVal));
							myEvolvers.add_thread(newThread);
						}

						myEvolvers.join_all();

						double bestVal = numeric_limits<double>::max();
						int bestEvolver = 0;

						for(int i = 0; i < NUM_THREADS; i++)
						{
							if(solutions[i].distance < bestVal)
							{
								bestVal = solutions[i].distance;
								bestEvolver = i;
							}
						}

						drawNextSolution(&solutions[bestEvolver]);

						if(NUM_THREADS > 1)
						{
							for(int i = 0; i < NUM_THREADS; i++)
							{
								int num = rand() % NUM_THREADS;

								while(num == bestEvolver)
								{
									num = rand() % NUM_THREADS;
								}

								myGeneticEvolvers[i].injectSolution(solutions[num], num);
								myGeneticEvolvers[i].injectSolution(solutions[bestEvolver], bestEvolver);
							}
						}

						if(solutions[bestEvolver].distance < bestDist)
						{
						    memcpy(bestSoFar->imageData, townMap->imageData, townMap->width * townMap->height * townMap->nChannels);
						    cvShowImage("Best", bestSoFar);
						    bestDist = solutions[bestEvolver].distance;
						}
					}
				}
			}
			else
			{
				cout << "Please sir, may I have some more [towns]?" << endl;
			}
		}
		else if(key == 'q')
		{
			break;
		}
	}

	cvReleaseImage(&townMap);
	cvReleaseImage(&bestSoFar);
	cvDestroyAllWindows();

	delete [] myGeneticEvolvers;
	delete [] solutions;

	return 0;
}

void init()
{
	townMap = cvCreateImage(cvSize(500, 500), IPL_DEPTH_8U, 3);
	bestSoFar = cvCreateImage(cvSize(500, 500), IPL_DEPTH_8U, 3);

	cvStartWindowThread();
    cvNamedWindow("Best", CV_WINDOW_AUTOSIZE);
    cvShowImage("Best", bestSoFar);
    cvNamedWindow("Plot", CV_WINDOW_AUTOSIZE);
    cvMoveWindow("Plot", 520, 0);
    cvShowImage("Plot", townMap);
    cvSetMouseCallback("Plot",&onMouse , 0);

    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.4, 0.4, 0, 1, 8);
}

void onMouse(int event, int x, int y, int flags, void * param)
{
	if(event == CV_EVENT_LBUTTONDOWN && !algorithmBegun)
	{
		town newTown = {x, y, fullTownList.size()};
		fullTownList.push_back(newTown);

		cvCircle(townMap ,cvPoint(x, y), 1, CV_RGB(255, 255, 255), 1, 8, 0);
		cvShowImage("Plot", townMap);
		cout << "New town added at: (" << x << ", " << y << "). Total towns: " << fullTownList.size() << endl;
	}
}

void drawNextSolution(solution * next)
{
	memset(townMap->imageData, 0, townMap->width * townMap->height * townMap->nChannels);

	for(unsigned int i = 0; i + 1 < next->townList.size(); i++)
	{
		cvLine(townMap, cvPoint(next->townList[i].x, next->townList[i].y), cvPoint(next->townList[i + 1].x, next->townList[i + 1].y), CV_RGB(128, 128, 128), 1, CV_AA, 0);
	}

	cvLine(townMap, cvPoint(next->townList[next->townList.size() - 1].x, next->townList[next->townList.size() - 1].y), cvPoint(next->townList[0].x, next->townList[0].y), CV_RGB(128, 128, 128), 1, CV_AA, 0);

	for(unsigned int i = 0; i < fullTownList.size(); i++)
	{
		cvCircle(townMap ,cvPoint(fullTownList[i].x, fullTownList[i].y), 1, CV_RGB(0, 255, 0), 1, 8, 0);
	}

	stringstream strs1, strs2;
	strs1 << "Generation: " << myGeneticEvolvers[0].getGeneration() * NUM_THREADS;
	strs2 << "Distance: " << next->distance;

	cvPutText(townMap, strs1.str().c_str(), cvPoint(20, 20), &font, CV_RGB(255, 255, 255));
	cvPutText(townMap, strs2.str().c_str(), cvPoint(20, townMap->height - 20), &font, CV_RGB(255, 255, 255));

	cvShowImage("Plot", townMap);
}

