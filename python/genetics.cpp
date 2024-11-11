#include <array>
#include <vector>
#include <random>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

const size_t populationSize = 1000;
const size_t elite = 2;
const size_t populationSizeAndElite = populationSize + elite;

const float crossoverRate = 1.0f;
float mutationRate = 0.002f;
const float mutationRateFrom = 0.001f;
const float mutationRateTo = 0.004f;
const size_t mutationRatePeriod = 2000;

//size_t tournamentSize = 3;
size_t tournamentSize = 10;
//float selectionProbability = 0.85f;
float selectionProbability = 0.5f;


typedef uint64_t Score;

typedef std::array<bool, width * height> Chromo;
struct PopElement
{
    Chromo val;
    Score fitness;
};

typedef std::vector<PopElement> Population;

//rnd
std::random_device rd;
std::mt19937 gen(rd());

//https://github.com/alenic/realGA/blob/master/src/selection_tournament.cpp
class TournamentSelection
{
public:

    TournamentSelection(size_t tournamentSize, float selectionProbability = 0.85f) : mTournamentSize(tournamentSize), mSelectionProbability(selectionProbability)
    {
        //mTournamentSize = std::max<size_t>(2, static_cast<size_t>(static_cast<float>(populationSize) *0.1f ));
    }

    void select(const Population& population, size_t &indexA, size_t &indexB) const
    {
        std::vector<Score> tournamentFitness(mTournamentSize);
        std::vector<size_t> tournamentIndex(mTournamentSize);

        indexA = tournament(population, tournamentFitness, tournamentIndex);
        indexB = tournament(population, tournamentFitness, tournamentIndex);

        std::uniform_int_distribution<> dst(0, mTournamentSize - 1);

        while(indexA == indexB)
        {
            indexA = tournamentIndex[dst(gen)];
            indexB = tournamentIndex[dst(gen)];
        }
    }

    size_t tournament(const Population& population, std::vector<Score>& tournamentFitness, std::vector<size_t>& tournamentIndex) const
    {
        std::uniform_real_distribution<> dist(0.0f, 1.0f);
        std::uniform_int_distribution<> iDistElite(0, populationSizeAndElite - 1);

        // return the index of winner
        size_t i = 0;
        bool match = false;

        // sample random mTournamentSize indices from mPopulationSize indices, without replacement
        while(i < mTournamentSize)
        {
            size_t index = iDistElite(gen);
            // check if the index was choosen
            match = false;
            for(size_t j = 0; j < i; j++)
            {
                if(tournamentIndex[j] == index)
                {
                    match = true;
                    break;
                }
            }
            if(!match)
            {
                tournamentIndex[i] = index;
                tournamentFitness[i] = population[index].fitness;
                i++;
            }
        }

        size_t kth;
        size_t kthMinIndex;
        size_t localKthIndex;

        if(mSelectionProbability < 1.0f)
        {
            float chooseP = mSelectionProbability;
            // choose the winner with a probability p, 2nd with p*(1-p), 3rd with p*(1-p)*2,...
            for(kth = 1; kth<mTournamentSize; kth++)
            {
                if(dist(gen) < chooseP)
                {
                    break;
                }
                chooseP *= (1.0f - mSelectionProbability);
            }
            // Get k-th min value
            localKthIndex = argSelection(tournamentFitness, 0, mTournamentSize - 1, kth);
            kthMinIndex = tournamentIndex[localKthIndex];
        }
        else
        {
            localKthIndex = argMax(tournamentFitness, 0, mTournamentSize - 1);
            kthMinIndex = tournamentIndex[localKthIndex];
        }


        return kthMinIndex;
    }

private:

    size_t argMax(const std::vector<Score>& arr, int l, int r) const
    {
        Score maxValue = arr[0];
        size_t maxIndex = 0;
        for(size_t i = l + 1; i <= r; i++)
        {
            if(arr[i] > maxValue)
            {
                maxValue = arr[i];
                maxIndex = i;
            }
        }
        return maxIndex;
    }

    size_t argSelection(std::vector<Score>& arr, int l, int r, int kth) const
    {
        // k * O(n)
        Score maxValue;
        size_t maxIndex;
        // find min
        for(size_t k = 0; k<kth; k++)
        {
            maxValue = arr[k];
            maxIndex = k;
            for(int i = l + k + 1; i <= r; i++)
            {
                if(arr[i] > maxValue)
                {
                    maxValue = arr[i];
                    maxIndex = i;
                }
            }
            if(k < kth - 1) Swap(arr[k], arr[maxIndex]);
        }
        return maxIndex;
    }

    void Swap(Score &a, Score &b) const
    {
        Score c = a;
        a = b;
        b = c;
    }

    size_t mTournamentSize;

    float mSelectionProbability;
};

TournamentSelection tournamentSel(tournamentSize, selectionProbability);

size_t epoch = 1;

Chromo bestTotal;
Score totalBestReward = 0;

size_t epochsWithoutUpdate = 0;

float sign = 1.0f;

Population pop(populationSizeAndElite);

const char* State = "map.txt";

void chromoFromMaze(const Maze& maze, Chromo& chromo)
{
    for(size_t q = 0; q < height; ++q)
    {
        for(size_t w = 0; w < width; ++w)
        {
            chromo[q * width + w] = maze[(q + 1) * maxWidth + w + 1] & wall;
        }
    }
}

void mutate(PopElement& elem)
{
    std::uniform_real_distribution<> dist(0.0f, 1.0f);

    for(size_t w = 0; w < elem.val.size(); ++w)
    {
        if(dist(gen) < mutationRate)
        {
            elem.val[w] = !elem.val[w];
        }
    }
}

std::pair<PopElement, PopElement> crossover(const PopElement& elemA, const PopElement& elemB)
{
    std::uniform_real_distribution<> dist(0.0f, 1.0f);

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

std::string printRewardFriendly(Score reward)
{
    std::string out = std::to_string(reward);
    std::string out2;
    for(size_t q = 0; q < out.size(); ++q)
    {
        out2.insert(out2.begin(), out[out.size() - q - 1]);
        if((q + 1) % 3 == 0 && q != out.size() - 1) out2.insert(out2.begin(), ' ');
    }
    return out2;
}

void loadState()
{
    std::string inputLine;

    std::ifstream file(State);
    if(file.is_open())
    {
        Maze maze;
        initMaze(maze);

        for(size_t q = 0; q < maxHeight; ++q)
        {
            getline(file, inputLine);
            for(size_t w = 0; w < maxWidth; ++w)
            {
                if(inputLine[w] == '#')
                {
                    maze[q * maxWidth + w] = wall;

                    if(q == 0 || w == 0 || q == maxHeight - 1 || w == maxWidth - 1)
                    {
                        maze[q * maxWidth + w] = externalWall;
                    }
                }
            }
        }

        for(size_t q = 0; q < populationSizeAndElite; ++q)
        {
            chromoFromMaze(maze, pop[q].val);
        }
    }
}

void gen_population(const uint64_t rewards[])
{
    
    
    //sawtooth
    {
        if(epoch % (mutationRatePeriod / 2) == 0) sign *= -1.0f;
        float fraction = static_cast<float>(epoch % (mutationRatePeriod / 2)) / static_cast<float>(mutationRatePeriod / 2);
        if(sign < 0.0f) fraction = 1.0f - fraction;
        mutationRate = mutationRateFrom + fraction * (mutationRateTo - mutationRateFrom);
    }

    Score bestReward = 0;
    {
        size_t bestIndex = 0;
        
        for(size_t q = 0; q < populationSize; ++q)
        {
            pop[q].fitness = rewards[q];
            
            if(rewards[q] > bestReward)
            {
                bestReward = rewards[q];
                bestIndex = q;
            }
        }
        
        if(bestReward > totalBestReward)
        {
            totalBestReward = bestReward;
            bestTotal = pop[bestIndex].val;
        }
    }
    
    Population pop2(pop);
    std::sort(pop2.begin(), pop2.end(), [](const PopElement& elemA, const PopElement& elemB){ return elemA.fitness < elemB.fitness; });
    printf("Sim epoch: %5zu reward: %10s  iter best: %10s  middle: %10s  100: %10s  mr: %.4f\n", epoch, printRewardFriendly(totalBestReward).c_str(), printRewardFriendly(bestReward).c_str(), printRewardFriendly(pop2[populationSize / 2].fitness).c_str(), printRewardFriendly(pop2[100].fitness).c_str(), mutationRate);
    
    //elitism
    for(size_t q = populationSize; q < populationSizeAndElite; ++q)
    {
        pop[q].val = bestTotal;
        pop[q].fitness = totalBestReward;
    }

    for(size_t q = 0; q < populationSizeAndElite; q += 2)
    {
        PopElement elemA;
        PopElement elemB;
        {
            size_t elA, elB;
            tournamentSel.select(pop, elA, elB);
            elemA = pop[elA];
            elemB = pop[elB];
        }
        std::pair<PopElement, PopElement> cross = crossover(elemA, elemB);
        mutate(cross.first);
        mutate(cross.second);
        
        pop[q + 0] = cross.first;
        pop[q + 1] = cross.second;
    }
    
    ++epoch;
}

uint64_t * get_population()
{

    uint64_t* array = static_cast<uint64_t*>(malloc(sizeof(uint64_t) * populationSize * width * height));
    
    for(size_t q = 0; q < populationSize; ++q)
    {
        for(size_t w = 0; w < width * height; ++w)
        {
            array[q * width * height + w] = pop[q].val[w];
        }
    }
    
    return array;
}