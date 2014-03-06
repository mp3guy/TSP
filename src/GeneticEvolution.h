/*
 * GeneticEvolution.h
 *
 *  Created on: 14 Sep 2010
 *      Author: thomas
 */

#ifndef GENETICEVOLUTION_H_
#define GENETICEVOLUTION_H_

#include <iostream>
#include <math.h>
#include <string.h>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

struct town
{
	int x, y;
	unsigned int townNum;
};

struct solution
{
	vector<town> townList;
	double distance;
	double fitness;
	int probability;
};

enum
{
	ROULETTE = 0,
	TOURNAMENT = 1
};

class GeneticEvolution
{
	public:
		GeneticEvolution();
		virtual ~GeneticEvolution();

		solution generateInitialPopulation();
		void setTownList(vector<town> * inTownList);
		void getNextGeneration(solution * inSolution, int selectionMethod);

		void injectSolution(solution inSolution, int notWhere);

		void setNumTowns(int inTowns)
		{
			numTowns = inTowns;
		}

		void setGenerationSize(int inGen)
		{
			if(currentPopulation != 0)
			{
				delete [] currentPopulation;
			}

			generationSize = inGen;

			currentPopulation = new solution[generationSize];
		}

		void setMutationChance(int inMut)
		{
			mutationChance = inMut;
		}

		void setMyID(int inId)
		{
			myId = inId;
		}

		int getMyId()
		{
		    return myId;
		}

		int getGeneration()
		{
			return generation;
		}

private:
		int myId;
		int generation;
		int generationSize;
		int numTowns;
		int mutationChance;
		double populationDisparity;
		double totalFitness;

		double ** distanceMatrix;
		bool * townVisited;

		enum
		{
			UP = 0,
			DOWN = 1
		};

		void generateRandomPopulations(bool resetCount);
		void calculateProportionalFitness();
		void rouletteSelection();
		void tournamentSelection();
		void reproduction();
		int getSingleParent(int searchSize, bool * searchSpace);
		void mutate(solution * inSolution);
		void geneRepair(solution * brokenSolution);
		void calculateDisparity();
		int shiftSolution(solution * inSolution, int direction, int markerVal);

		void swapTown(town * a, town * b);
		void quickSortTown(town * t, int beg, int end);

		void fillDistanceMatrix();
		bool isValidSolution(solution * inSolution);
		double getSolutionDistance(solution * inSolution);
		inline double distance(int x1, int y1, int x2, int y2);

		solution * currentPopulation;
		solution * newPopulation;
		solution * disparityPopulation;
		int * rouletteWheel;
		bool * alreadyPaired;
		bool * alreadyFought;
		int ** disparity;

		vector<town> fullTownList;
};

#endif /* GENETICEVOLUTION_H_ */
