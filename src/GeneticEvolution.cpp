/*
 * GeneticEvolution.cpp
 *
 *  Created on: 14 Sep 2010
 *      Author: thomas
 */

#include "GeneticEvolution.h"

GeneticEvolution::GeneticEvolution()
{
	totalFitness = 0;
	generation = 0;
	generationSize = 32;
	numTowns = 0;
	mutationChance = 1;
	myId = 0;
	currentPopulation = 0;
}

GeneticEvolution::~GeneticEvolution()
{
	for(int i = 0; i < numTowns; i++)
	{
		  delete [] distanceMatrix[i];
	}
	delete [] distanceMatrix;

	for(int i = 0; i < generationSize / 2; i++)
	{
		  delete [] disparity[i];
	}
	delete [] disparity;

	delete [] alreadyPaired;
	delete [] alreadyFought;
	delete [] townVisited;
	delete [] currentPopulation;
}

void GeneticEvolution::setTownList(vector<town> * inTownList)
{
	fullTownList = *inTownList;
}

solution GeneticEvolution::generateInitialPopulation()
{
	distanceMatrix = new double * [numTowns];
	for(int i = 0; i < numTowns; i++)
	{
		  distanceMatrix[i] = new double[numTowns];
	}

	disparity = new int * [generationSize / 2];
	for(int i = 0; i < generationSize / 2; i++)
	{
		disparity[i] = new int[generationSize / 2];
	}

	alreadyFought = new bool[generationSize];

	alreadyPaired = new bool[generationSize / 2];

	townVisited = new bool[numTowns];

	fillDistanceMatrix();

	generateRandomPopulations(true);

	return currentPopulation[0];
}

void GeneticEvolution::generateRandomPopulations(bool resetCount)
{
	if(resetCount)
	{
		generation = 0;
	}

	for(int i = 0; i < generationSize; i++)
	{
		for(int j = 0; j < numTowns; j++)
		{
			townVisited[j] = false;
		}

		vector<town> newTownList;

		while((int)newTownList.size() < numTowns)
		{
			int nextTown = rand() % numTowns;

			if(!townVisited[nextTown])
			{
				newTownList.push_back(fullTownList[nextTown]);
				townVisited[nextTown] = true;
			}

		}

		solution newSolution = {newTownList, 0, 0, 0};
		newSolution.distance = getSolutionDistance(&newSolution);

		currentPopulation[i] = newSolution;
	}
}

void GeneticEvolution::getNextGeneration(solution * inSolution, int selectionMethod)
{
	newPopulation = new solution[generationSize];

	if(selectionMethod == ROULETTE) rouletteSelection();
	else if(selectionMethod == TOURNAMENT) tournamentSelection();

	reproduction();

	generation++;

	double bestVal = numeric_limits<double>::max();
	int bestEvolver = 0;

	for(int i = 0; i < generationSize; i++)
	{
		if(newPopulation[i].distance < bestVal)
		{
			bestVal = newPopulation[i].distance;
			bestEvolver = i;
		}
	}

	delete [] currentPopulation;
	currentPopulation = newPopulation;

	*inSolution = currentPopulation[bestEvolver];
}

void GeneticEvolution::injectSolution(solution inSolution, int notWhere)
{
	int index = notWhere;

	while(index == notWhere)
	{
		index = rand() % generationSize;
	}

	currentPopulation[index] = inSolution;
}

void GeneticEvolution::rouletteSelection()
{
	int wheelSize = 0;

	calculateProportionalFitness();

	for(int i = 0; i < generationSize; i++)
	{
		wheelSize += currentPopulation[i].probability;
	}

	rouletteWheel = new int[wheelSize];

	int wheelIndex = 0;

	//Fill in wheel
	for(int i = 0; i < generationSize; i++)
	{
		int j = 0;
		while(j < currentPopulation[i].probability)
		{
			rouletteWheel[wheelIndex++] = i;
			j++;
		}
	}

	for(int i = 0; i < generationSize; i++)
	{
		alreadyFought[i] = false;
	}


	double bestVal = numeric_limits<double>::max();
	int bestEvolver = 0;

	for(int i = 0; i < generationSize; i++)
	{
		if(currentPopulation[i].distance < bestVal)
		{
			bestVal = currentPopulation[i].distance;
			bestEvolver = i;
		}
	}
	newPopulation[0] = currentPopulation[bestEvolver];
	alreadyFought[bestEvolver] = true;

	//Take the (most probable) best half of the current population
	for(int i = 1; i < generationSize / 2; i++)
	{
		int selected = rouletteWheel[rand() % wheelSize];

		if(alreadyFought[selected])
		{
			i--;
			continue;
		}

		newPopulation[i] = currentPopulation[selected];
		alreadyFought[selected] = true;
	}

	delete [] rouletteWheel;
}

void GeneticEvolution::tournamentSelection()
{
	for(int i = 0; i < generationSize; i++)
	{
		alreadyFought[i] = false;
	}

	//Fight
	for(int i = 0; i < generationSize / 2; i++)
	{
		//Randomly select two parents
		int firstParent = getSingleParent(generationSize, alreadyFought);
		int secondParent = getSingleParent(generationSize, alreadyFought);

		if(currentPopulation[firstParent].distance < currentPopulation[secondParent].distance)
		{
			newPopulation[i] = currentPopulation[firstParent];
		}
		else
		{
			newPopulation[i] = currentPopulation[secondParent];
		}
	}
}

void GeneticEvolution::calculateProportionalFitness()
{
	totalFitness = 0;

	for(int i = 0; i < generationSize; i++)
	{
		//Reciprocal as we're attempting to minimise distance
		currentPopulation[i].fitness = 1 / currentPopulation[i].distance;
		totalFitness += currentPopulation[i].fitness;
	}

	//Normalise and size up our wheel
	for(int i = 0; i < generationSize; i++)
	{
		currentPopulation[i].probability = (int)((currentPopulation[i].fitness / totalFitness) * 10000);
	}
}

void GeneticEvolution::reproduction()
{
	for(int i = 0; i < generationSize / 2; i++)
	{
		alreadyPaired[i] = false;
	}

	calculateDisparity();

	//Crossover
	for(int i = generationSize / 2; i + 1 < generationSize; i+=2)
	{
		int firstParent = getSingleParent(generationSize / 2, alreadyPaired);

		int bestSecondParent = 0;
		int bestDisparity = 0;

		for(int j = 0; j < generationSize / 2; j++)
		{
			if(j != firstParent && disparity[firstParent][j] > bestDisparity && !alreadyPaired[j])
			{
				bestSecondParent = j;
				bestDisparity = disparity[firstParent][j];
			}
		}

		alreadyPaired[bestSecondParent] = true;

		int crossOverPoint = rand() % numTowns;

		vector<town> childTownsOne, childTownsTwo;

		solution childOne = {childTownsOne, 0, 0, 0};
		solution childTwo = {childTownsTwo, 0, 0, 0};

		int shiftVal = 0;

		//Crossover
		if(crossOverPoint > numTowns / 2)
		{
			int diff = crossOverPoint - (numTowns / 2);
			shiftSolution(&newPopulation[firstParent], DOWN, newPopulation[firstParent].townList[diff].townNum);
			shiftVal = shiftSolution(&newPopulation[bestSecondParent], DOWN, newPopulation[bestSecondParent].townList[diff].townNum);
		}
		else if(crossOverPoint < numTowns / 2)
		{
			int diff = (numTowns / 2) - crossOverPoint;
			shiftSolution(&newPopulation[firstParent], UP, newPopulation[firstParent].townList[newPopulation[firstParent].townList.size() - 1 - diff].townNum);
			shiftVal = shiftSolution(&newPopulation[bestSecondParent], UP, newPopulation[bestSecondParent].townList[newPopulation[bestSecondParent].townList.size() - 1 - diff].townNum);
		}

		for(int j = 0; j < numTowns / 2; j++)
		{
			childTownsOne.push_back(newPopulation[firstParent].townList[j]);
			childTownsTwo.push_back(newPopulation[bestSecondParent].townList[j]);
		}

		for(int j = numTowns / 2; j < numTowns; j++)
		{
			childTownsOne.push_back(newPopulation[bestSecondParent].townList[j]);
			childTownsTwo.push_back(newPopulation[firstParent].townList[j]);
		}

		childOne.townList = childTownsOne;
		childTwo.townList = childTownsTwo;

		if(shiftVal > 0)
		{
			shiftSolution(&childOne, DOWN, childOne.townList[shiftVal].townNum);
			shiftSolution(&childTwo, DOWN, childTwo.townList[shiftVal].townNum);
			shiftSolution(&newPopulation[firstParent], DOWN, newPopulation[firstParent].townList[shiftVal].townNum);
			shiftSolution(&newPopulation[bestSecondParent], DOWN, newPopulation[bestSecondParent].townList[shiftVal].townNum);
		}
		else if(shiftVal < 0)
		{
			shiftSolution(&childOne, UP, childOne.townList[childOne.townList.size() - 1 + shiftVal].townNum);
			shiftSolution(&childTwo, UP, childTwo.townList[childTwo.townList.size() - 1 + shiftVal].townNum);
			shiftSolution(&newPopulation[firstParent], UP, newPopulation[firstParent].townList[newPopulation[firstParent].townList.size() - 1 + shiftVal].townNum);
			shiftSolution(&newPopulation[bestSecondParent], UP, newPopulation[bestSecondParent].townList[newPopulation[bestSecondParent].townList.size() - 1 + shiftVal].townNum);
		}

		//Mutation
		mutate(&childOne);
		mutate(&childTwo);

		//Double point mutation
		if(mutationChance > 0 && rand() % (100 / mutationChance) == 1)
		{
			mutate(&childOne);
			mutate(&childTwo);
		}

		if(!isValidSolution(&childOne))
		{
			geneRepair(&childOne);
		}
		if(!isValidSolution(&childTwo))
		{
			geneRepair(&childTwo);
		}

		childOne.distance = getSolutionDistance(&childOne);
		childTwo.distance = getSolutionDistance(&childTwo);

		newPopulation[i] = childOne;
		newPopulation[i + 1] = childTwo;
	}
}

void GeneticEvolution::calculateDisparity()
{
	disparityPopulation = (solution *)malloc(sizeof(solution) * generationSize / 2);

	memcpy(disparityPopulation, newPopulation, sizeof(solution) * generationSize / 2);

	for(int i = 0; i < generationSize / 2; i++)
	{
		shiftSolution(&disparityPopulation[i], DOWN, 0);
	}

	int firstIndices[numTowns];
	int secondIndices[numTowns];

	populationDisparity = 0;

	for(int i = 0; i < generationSize / 2; i++)
	{
		for(int j = 0; j < generationSize / 2; j++)
		{
			if(i == j)
			{
				disparity[i][j] = 0.0;
			}
			else
			{
				for(int k = 0; k < numTowns; k++)
				{
					firstIndices[disparityPopulation[i].townList[k].townNum] = k;
					secondIndices[disparityPopulation[j].townList[k].townNum] = k;
				}

				int disparitySum = 0;

				for(int k = 0; k < numTowns; k++)
				{
					disparitySum += abs(firstIndices[k] - secondIndices[k]);
				}

				disparity[i][j] = disparitySum;

				populationDisparity += disparitySum;
			}
		}
	}

	populationDisparity /= (generationSize / 2) * (generationSize / 2);

	if(populationDisparity < 40)
	{
		generateRandomPopulations(false);
		for(int i = 0; i < generationSize / 4; i++)
		{
			newPopulation[i] = currentPopulation[rand() % generationSize];
		}

		memcpy(disparityPopulation, newPopulation, sizeof(solution) * generationSize / 2);

		for(int i = 0; i < generationSize / 2; i++)
		{
			shiftSolution(&disparityPopulation[i], DOWN, 0);
		}

		for(int i = 0; i < generationSize / 2; i++)
		{
			for(int j = 0; j < generationSize / 2; j++)
			{
				if(i == j)
				{
					disparity[i][j] = 0.0;
				}
				else
				{
					for(int k = 0; k < numTowns; k++)
					{
						firstIndices[disparityPopulation[i].townList[k].townNum] = k;
						secondIndices[disparityPopulation[j].townList[k].townNum] = k;
					}

					int disparitySum = 0;

					for(int k = 0; k < numTowns; k++)
					{
						disparitySum += abs(firstIndices[k] - secondIndices[k]);
					}

					disparity[i][j] = disparitySum;
				}
			}
		}
	}

	free(disparityPopulation);
}

int GeneticEvolution::shiftSolution(solution * inSolution, int direction, int markerVal)
{
	int shiftVal = 0;

	if(direction == DOWN)
	{
		while((int)inSolution->townList[0].townNum != markerVal)
		{
			town temp = inSolution->townList[0];
			inSolution->townList.erase(inSolution->townList.begin());
			inSolution->townList.push_back(temp);
			shiftVal--;
		}
	}
	else if(direction == UP)
	{
		while((int)inSolution->townList[inSolution->townList.size() - 1].townNum != markerVal)
		{
			town temp = inSolution->townList[inSolution->townList.size() - 1];
			inSolution->townList.pop_back();
			inSolution->townList.insert(inSolution->townList.begin(), temp);
			shiftVal++;
		}
	}

	return shiftVal;
}

int GeneticEvolution::getSingleParent(int searchSize, bool * searchSpace)
{
	int newParent;

	while(true)
	{
		newParent = rand() % searchSize;
		if(!searchSpace[newParent])
		{
			searchSpace[newParent] = true;
			break;
		}
	}

	return newParent;
}

void GeneticEvolution::mutate(solution * inSolution)
{
	if(mutationChance > 0 && rand() % (100 / mutationChance) == 1)
	{
		int swapOne = 0;
		int swapTwo = 0;

		while(swapOne == swapTwo)
		{
			swapOne = rand() % numTowns;
			swapTwo = rand() % numTowns;
		}

		town temp = inSolution->townList[swapOne];
		inSolution->townList[swapOne] = inSolution->townList[swapTwo];
		inSolution->townList[swapTwo] = temp;
	}
}

void GeneticEvolution::geneRepair(solution * brokenSolution)
{
	int townHits[numTowns];

	vector<int> unvisited;

	for(int i = 0; i < numTowns; i++)
	{
		townHits[i] = 0;
	}

	for(unsigned int i = 0; i < brokenSolution->townList.size(); i++)
	{
		townHits[brokenSolution->townList[i].townNum]++;
	}

	for(int i = 0; i < numTowns; i++)
	{
		if(townHits[i] == 0)
		{
			unvisited.push_back(i);
		}
	}

	for(unsigned int i = 0; i < brokenSolution->townList.size() && unvisited.size() > 0; i++)
	{
		if(townHits[brokenSolution->townList[i].townNum] > 1)
		{
			int newTown = rand() % unvisited.size();
			brokenSolution->townList[i] = fullTownList[unvisited[newTown]];
			unvisited.erase(unvisited.begin() + newTown);
			townHits[brokenSolution->townList[i].townNum]--;
			i--;
		}
	}

	if(!isValidSolution(brokenSolution))
	{
		geneRepair(brokenSolution);
	}
}

void GeneticEvolution::fillDistanceMatrix()
{
	for(int i = 0; i < numTowns; i++)
	{
		for(int j = 0; j < numTowns; j++)
		{
			distanceMatrix[i][j] = distance(fullTownList[i].x, fullTownList[i].y, fullTownList[j].x, fullTownList[j].y);
		}
	}
}

inline double GeneticEvolution::distance(int x1, int y1, int x2, int y2)
{
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool GeneticEvolution::isValidSolution(solution * inSolution)
{
	if((int)inSolution->townList.size() != numTowns)
	{
		return false;
	}

	vector<town> sorted = inSolution->townList;

	quickSortTown(sorted.data(), 0, (int)sorted.size());

	for(unsigned int i = 0; i < sorted.size(); i++)
	{
		if(sorted[i].townNum != i)
		{
			return false;
		}
	}

	return true;
}

void GeneticEvolution::swapTown(town * a, town * b)
{
	town temp = *a;
	*a = *b;
	*b = temp;
}

void GeneticEvolution::quickSortTown(town * townArray, int beg, int end)
{
	if(end > beg + 1)
	{
		int piv = townArray[beg].townNum, l = beg + 1, r = end;

		while(l < r)
		{
			if((int)townArray[l].townNum <= piv)
			{
				l++;
			}
			else
			{
				swapTown(&townArray[l], &townArray[--r]);
			}
		}

		swapTown(&townArray[--l], &townArray[beg]);
		quickSortTown(townArray, beg, l);
		quickSortTown(townArray, r, end);
	}
}

double GeneticEvolution::getSolutionDistance(solution * inSolution)
{
	double sum = 0.0;

	for(unsigned int i = 0; i + 1 < inSolution->townList.size(); i++)
	{
		sum += distanceMatrix[inSolution->townList[i].townNum][inSolution->townList[i + 1].townNum];
	}

	sum += distanceMatrix[inSolution->townList[inSolution->townList.size() - 1].townNum][inSolution->townList[0].townNum];
	return sum;
}
