#ifndef POPULATION_H
#define POPULATION_H

#include <array>

#include "Maze.h"

typedef std::array<bool, width * height> Chromo;
struct PopElement
{
    Chromo val;
    Score fitness;
};

typedef std::vector<PopElement> Population;

void initPopulation(Population& pop)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    pop.resize(populationSizeAndElite);

    for(size_t q = 0; q < populationSizeAndElite; ++q)
    {
        for(size_t w = 0; w < width * height; ++w)
        {
            float rnd = dist(gen);
            if(rnd > 0.5f) pop[q].val[w] = true;
            else pop[q].val[w] = false;

            pop[q].fitness = 1;
        }
    }
}


void mazeFromChromo(const Chromo& chromo, Maze& maze)
{
    for(size_t q = 0; q < height; ++q)
    {
        for(size_t w = 0; w < width; ++w)
        {
            if(chromo[q * width + w]) maze[(q + 1) * maxWidth + w + 1] = wall;
        }
    }
}

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

void loadState(Population& pop)
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

        printMaze(maze);
    }
}

void saveMazeWithWeights(const std::string& fileName, const Maze& maze, const Chromo& chromo)
{
    FILE * f = fopen(fileName.c_str(), "wt");
    if(f)
    {
        for(size_t q = 1; q < maxHeight - 1; ++q)
        {
            for(size_t w = 1; w < maxWidth - 1; ++w)
            {
                if(chromo[(q - 1) * width + w - 1]) fprintf(f, "%6s ", "#");
                //if(maze[q * maxWidth + w] == wall) fprintf(f, "%6s ", "#");
                else fprintf(f, "%6zd ", maze[q * maxWidth + w]);
            }

            fprintf(f, "\n");
        }

        fclose(f);
    }
}


Score runGame(const Chromo& chromo, bool isSaveMazeWithWeights = false, bool isRunStepByStep = false)
{
    Maze maze;
    initMaze(maze);

    mazeFromChromo(chromo, maze);

    Score reward;
    bool isExit = runMaze(maze, reward);

    if(isSaveMazeWithWeights)
    {
        saveMazeWithWeights("w.txt", maze, chromo);
    }

    if(isRunStepByStep)
    {
        Maze mazeStep;
        initMaze(mazeStep);

        mazeFromChromo(chromo, mazeStep);

        MazeState state;
        do{
            reward = state.n;
            runMazeStepByStep(mazeStep, state, 1000000);
            std::string fName = "w_" + std::to_string(state.n) + ".txt";
            saveMazeWithWeights(fName.c_str(), mazeStep, chromo);
        }while(reward != state.n);


    }

    return reward;
}


Score assignFitness(Population& pop, Chromo& best, size_t& bestIndex)
{
    Score bestReward = 0;

#if defined(PROCESS_THREADS)
    std::vector<Score> rewards(populationSize);

    {
        thread_pool tPool(threadsAmount);

        auto processLambda = [&](size_t start, size_t end){
            for(size_t q = start; q < end; ++q)
            {
                Score reward = runGame(pop[q].val);

                rewards[q] = reward;

                pop[q].fitness = reward;
                if(reward == 0) pop[q].fitness = 1;
            }
        };

        size_t batch = populationSize / threadsAmount;

        for(size_t q = 0; q < threadsAmount; ++q)
        {
            size_t start = q * batch;
            size_t end = (q + 1) * batch;
            tPool.enqueue(processLambda, start, end);
        }
    }

    auto maxEl = std::max_element(rewards.begin(), rewards.end());
    bestIndex = maxEl - rewards.begin();
    bestReward = (*maxEl);
    best = pop[bestIndex].val;
#else
    for(size_t q = 0; q < populationSize; ++q)
    {
        Score reward = runGame(pop[q].val);

        if(reward > bestReward)
        {
            bestReward = reward;
            best = pop[q].val;
            bestIndex = q;
        }

        pop[q].fitness = reward;
        if(reward == 0) pop[q].fitness = 1;
    }
#endif

    return bestReward;
}
#endif