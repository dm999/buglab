#include <cstdio>
#include <cmath>
#include <array>
#include <vector>
#include <cstring>
#include <random>
#include <numeric>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>
#include <utility>

//run only single game
//#define RUN_SINGLE_GAME_ONLY
//#define SAVE_HEATMAP
//#define RUN_STEP_BY_STEP

//genetics
const size_t reshape = 1;
//const size_t reshape = 5;
const size_t populationSize = 1000 / reshape;
const size_t elite = 2;
const size_t populationSizeAndElite = populationSize + elite;
const float crossoverRate = 1.0f;
float mutationRate = 0.002f;
typedef std::pair<float, float> MutationFactorRange;
MutationFactorRange mutationRange = {0.001f, 0.004f};
//MutationFactorRange mutationRange = {0.001f, 0.01f};
//MutationFactorRange mutationRange = {0.004f, 0.01f};
//MutationFactorRange mutationRange = {0.008f, 0.02f};
const size_t mutationRatePeriod = 2000 * reshape;
//const float asyncMutationFactor = 1.0f;
const float asyncMutationFactor = 2.0f;
//const float asyncMutationFactor = 20.0f;
//#define SPATIAL_BASED_MUTATION
//https://cstheory.stackexchange.com/questions/14758/tournament-selection-in-genetic-algorithms
enum SelectionAlgo
{
    AlgoRoulette,
    AlgoTournament
};
SelectionAlgo SelAlgo = SelectionAlgo::AlgoRoulette;
//SelectionAlgo SelAlgo = SelectionAlgo::AlgoTournament;
//size_t tournamentSize = 3;
size_t tournamentSize = 10;
float selectionProbability = 0.85f;
//float selectionProbability = 0.5f;

//parallel
#define PROCESS_THREADS
const size_t threadsAmount = 10;

//other
#define VARIABLE_MUTATION_RATE
#define NEW_TOURNAMENT
#define STAT
size_t refreshEvery = 3000 * reshape;
size_t redefineSeedIfNoChangeEvery = 3000 * reshape;


const double pi = std::acos(-1);

//rnd
std::random_device rd;
std::mt19937 gen(rd());

#include "ThreadPool.h"
#include "Population.h"
#include "Maze.h"
#include "Tournament.h"
#include "Masks.h"


TournamentSelection tournamentSel(tournamentSize, selectionProbability);


void mutate(PopElement& elem)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    for(size_t w = 0; w < elem.val.size(); ++w)
    {
        if(dist(gen) < mutationRate)
        {
            elem.val[w] = !elem.val[w];
        }
    }
}

void asyncMutate(PopElement& elem)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    for(size_t w = 0; w < elem.val.size(); ++w)
    {
#if defined (SPATIAL_BASED_MUTATION)
        float spatialWeight = static_cast<float>(w) / (static_cast<float>(width) * static_cast<float>(height)) + 0.0001f;
#else
        float spatialWeight = 1.0f;
#endif
        float prob = dist(gen);
        if(elem.val[w] == true)
        {
            if(prob < mutationRate / spatialWeight)
            {
                elem.val[w] = false;
            }
        }
        else
        {
            if(prob < (mutationRate / (asyncMutationFactor / spatialWeight)))
            {
                elem.val[w] = true;
            }
        }
    }
}

void mutateWithMask(PopElement& elem)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    for(size_t w = 0; w < elem.val.size(); ++w)
    {
        if(mask[w])
        {
            if(dist(gen) < mutationRate)
            {
                elem.val[w] = !elem.val[w];
            }
        }
    }
}

std::pair<PopElement, PopElement> crossover(const PopElement& elemA, const PopElement& elemB)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    std::pair<PopElement, PopElement> ret(elemA, elemB);

    if(dist(gen) < crossoverRate)
    {
        size_t crossover = static_cast<size_t>(dist(gen) * width * height);

        std::copy(elemA.val.begin(), elemA.val.begin() + crossover, ret.first.val.begin());
        std::copy(elemB.val.begin() + crossover, elemB.val.end(), ret.first.val.begin() + crossover);

        std::copy(elemB.val.begin(), elemB.val.begin() + crossover, ret.second.val.begin());
        std::copy(elemA.val.begin() + crossover, elemA.val.end(), ret.second.val.begin() + crossover);
    }

    return ret;
}

PopElement roulette(const Population& pop, Score totalFitness)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    Score slice = dist(gen) * totalFitness;

    Score fitnesThreshold = 0;
    for(size_t q = 0; q < populationSizeAndElite; ++q)
    {
        fitnesThreshold += pop[q].fitness;
        if(fitnesThreshold >= slice)
        {
            return pop[q];
        }
    }

    return pop[populationSizeAndElite - 1];
}

PopElement tournament(const Population& pop, size_t tournamentSize)
{
    std::uniform_int_distribution<> iDistElite(0, populationSizeAndElite - 1);

    size_t selected = 0xFFFFFFFF;
    for(size_t q = 0; q < tournamentSize; ++q)
    {
        size_t selectedTmp = iDistElite(gen);
        if(selected == 0xFFFFFFFF)
        {
            selected = selectedTmp;
        }
        else
        {
            if(pop[selectedTmp].fitness > pop[selected].fitness)
            {
                selected = selectedTmp;
            }
        }
    }

    return pop[selected];
}

void geneticSearch(Population& pop)
{

    size_t epoch = 1;

    Chromo bestTotal;
    Score totalBestReward = 0;
    size_t bestIndex;
    bool resultUpdated = false;

    size_t epochsWithoutUpdate = 0;

    float sign = 1.0f;

    while(1)
    {
#if defined(VARIABLE_MUTATION_RATE)
#if 0
        mutationRate = mutationRange.first + (std::sin(pi * (((static_cast<float>(epoch) / static_cast<float>(mutationRatePeriod)) * 360.0f) - 90.0f) / 180.0f) * 0.5f + 0.5f) * (mutationRange.second - mutationRange.first);
#else
        //sawtooth
        {
            if(epoch % (mutationRatePeriod / 2) == 0) sign *= -1.0f;
            float fraction = static_cast<float>(epoch % (mutationRatePeriod / 2)) / static_cast<float>(mutationRatePeriod / 2);
            if(sign < 0.0f) fraction = 1.0f - fraction;
            mutationRate = mutationRange.first + fraction * (mutationRange.second - mutationRange.first);
        }
#endif
#endif

        if(epoch % refreshEvery == 0 && !resultUpdated)//refresh
        {
            printf("Refresh population with best chromo\n");
            for(size_t q = 0; q < populationSizeAndElite; ++q)
            {
                pop[q].val = bestTotal;
            }
        }

        if(epoch % redefineSeedIfNoChangeEvery == 0 && !resultUpdated)
        //if(epochsWithoutUpdate >= redefineSeedIfNoChangeEvery)
        {
            epochsWithoutUpdate = 0;
            gen = std::mt19937(rd());
            printf("Refresh rnd generator\n");
        }

        printf("Epoch: %5zd ", epoch);

        std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();

        Chromo best;
        Score bestReward = assignFitness(pop, best, bestIndex);
        ++epochsWithoutUpdate;
        if(bestReward > totalBestReward)
        {
            bestTotal = best;
            totalBestReward = bestReward;
            epochsWithoutUpdate = 0;

            {
                Maze maze;
                initMaze(maze);
                mazeFromChromo(bestTotal, maze);
                saveMaze(maze);
            }

            if(epoch != 1)
            {
                resultUpdated = true;

                FILE * f = fopen("mr.txt", "at");
                if(f)
                {
                    fprintf(f, "%zu %f\n", totalBestReward, mutationRate);
                    fclose(f);
                }
            }
        }

        //elitism
        for(size_t q = populationSize; q < populationSizeAndElite; ++q)
        {
            pop[q].val = bestTotal;
            pop[q].fitness = totalBestReward;
        }

#if defined(STAT)
        Population pop2(pop);
        std::sort(pop2.begin(), pop2.end(), [](const PopElement& elemA, const PopElement& elemB){ return elemA.fitness < elemB.fitness; });
        printf("reward: %10s  iter best: %10s  middle: %10s  %zu: %10s  mr: %.4f ", printRewardFriendly(totalBestReward).c_str(), printRewardFriendly(bestReward).c_str(), printRewardFriendly(pop2[populationSize / 2].fitness).c_str(), 100 / reshape, printRewardFriendly(pop2[100 / reshape].fitness).c_str(), mutationRate);
#else
        //printf("total best reward: %zu best reward: %zu ", totalBestReward, bestReward);
        printf("reward: %10s iter best: %10s ", printRewardFriendly(totalBestReward).c_str(), printRewardFriendly(bestReward).c_str());
        //std::cout << "reward: " << printRewardFriendly(totalBestReward) << "  reward iter: " << printRewardFriendly(bestReward) << " ";
#endif

        Score totalFitness = 0;
        if(SelAlgo == SelectionAlgo::AlgoRoulette)
        {
            totalFitness = std::accumulate(pop.begin(), pop.end(), static_cast<Score>(0), [](Score init, const PopElement& elem){ return init + elem.fitness; });
        }


        for(size_t q = 0; q < populationSizeAndElite; q += 2)
        {
            PopElement elemA;
            PopElement elemB;
            if(SelAlgo == SelectionAlgo::AlgoRoulette)
            {
                elemA = roulette(pop, totalFitness);
                elemB = roulette(pop, totalFitness);
            }
            else if(SelAlgo == SelectionAlgo::AlgoTournament)
            {
#if defined(NEW_TOURNAMENT)
                size_t elA, elB;
                tournamentSel.select(pop, elA, elB);
                elemA = pop[elA];
                elemB = pop[elB];
#else
                elemA = tournament(pop, tournamentSize);
                elemB = tournament(pop, tournamentSize);
#endif
            }
            std::pair<PopElement, PopElement> cross = crossover(elemA, elemB);
            
            //mutate(cross.first);
            //mutate(cross.second);
            
            //asyncMutate(cross.first);
            //asyncMutate(cross.second);

            mutateWithMask(cross.first);
            mutateWithMask(cross.second);


            //if(!resultUpdated || bestIndex != q || bestIndex != q + 1)
            {
                pop[q + 0] = cross.first;
                pop[q + 1] = cross.second;
            }
        }

        long long timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        float ms = static_cast<float>(timeTaken) / 1000.0f / 1000.0f;

        printf("epoch time = %5.1fs\n", ms);

        ++epoch;
    }

}

int main()
{
    if(sizeof(size_t) < 8)
    {
        printf("32bit application is slower, suggest to use 64bit\n");
    }

#if !defined(RUN_SINGLE_GAME_ONLY)

    Population pop;
    initPopulation(pop);
    loadState(pop);

    printf("Total population size: %zd, among them elite: %zd\n", populationSizeAndElite, elite);

    if(populationSizeAndElite % 2 != 0)
    {
        printf("Population size is incorrect, should be even\n");
        return 0;
    }

#if defined(PROCESS_THREADS)
    printf("Threads: %zd\n", threadsAmount);
    if(threadsAmount > populationSize || populationSize % threadsAmount != 0)
    {
        printf("Incorrect threads amount\n");
        return 0;
    }
#endif

    geneticSearch(pop);
#else

    Population pop;
    initPopulation(pop);
    loadState(pop);
    printf("Run single game: ");
    std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
#if !defined(SAVE_HEATMAP)
    Score reward = runGame(pop[0].val);
#else
#if !defined(RUN_STEP_BY_STEP)
    Score reward = runGame(pop[0].val, true);
#else
    Score reward = runGame(pop[0].val, false, true);
#endif
#endif
    long long timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    float ms = static_cast<float>(timeTaken) / 1000.0f / 1000.0f;
    std::cout << "reward = " << reward << " (" << printRewardFriendly(reward) << ") ";
    printf("time = %.1fs\n", ms);

#endif
    return 0;
}