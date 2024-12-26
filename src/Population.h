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

template <typename T>
static T Clamp(T val, T minval, T maxval)
{
    //assert(minval <= maxval && "Invalid clamp range");
    return std::max(std::min(val, maxval), minval);
}

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
                if(chromo[(q - 1) * width + w - 1]) fprintf(f, "%9s ", "#");
                //if(maze[q * maxWidth + w] == wall) fprintf(f, "%9s ", "#");
                else fprintf(f, "%9zd ", maze[q * maxWidth + w]);
            }

            fprintf(f, "\n");
        }

        fclose(f);
    }
}


Score runGame(const Chromo& chromo, int& maxCruci, bool isSaveMazeWithWeights = false, bool isRunStepByStep = false)
{
    Maze maze;
    initMaze(maze);

    mazeFromChromo(chromo, maze);

    Score reward;
    bool isExit = runMaze(maze, reward);
    
#if 0
    Score maxVal = 0;
    if(isExit)
    {
        for(size_t q = 1; q < maxHeight - 1; ++q)
        {
            for(size_t w = 1; w < maxWidth - 1; ++w)
            {
                if(maze[q * maxWidth + w] != wall)
                {
                    if(maze[q * maxWidth + w] > maxVal) maxVal =  maze[q * maxWidth + w];
                }
            }
        }
    }
#endif

#if 1
    std::vector<MazeData> uniqueCruci;
    maxCruci = 0;
    if(isExit)
    {
        const float cruciNearThreshold = 1.3f;

        for(size_t q = 1; q < maxHeight - 1; ++q)
        {
            for(size_t w = 1; w < maxWidth - 1; ++w)
            {
                int countNearExpLarge = 0;

                MazeData cell = maze[q * maxWidth + w];

                if(cell == wall) continue;

                if(cell > maze[(q + 0) * maxWidth + w - 1] * cruciNearThreshold && maze[(q + 0) * maxWidth + w - 1] != wall && maze[(q + 0) * maxWidth + w - 1] != externalWall) ++countNearExpLarge;
                if(cell > maze[(q + 0) * maxWidth + w + 1] * cruciNearThreshold && maze[(q + 0) * maxWidth + w + 1] != wall && maze[(q + 0) * maxWidth + w + 1] != externalWall) ++countNearExpLarge;
                if(cell > maze[(q - 1) * maxWidth + w + 0] * cruciNearThreshold && maze[(q - 1) * maxWidth + w + 0] != wall && maze[(q - 1) * maxWidth + w + 0] != externalWall) ++countNearExpLarge;
                if(cell > maze[(q + 1) * maxWidth + w + 0] * cruciNearThreshold && maze[(q + 1) * maxWidth + w + 0] != wall && maze[(q + 1) * maxWidth + w + 0] != externalWall) ++countNearExpLarge;


                if(countNearExpLarge >= 3)
                {
                    const float uniqueThreshold = 0.1f;

                    bool isUniqeCruci = true;

                    for(size_t q = 0; q < uniqueCruci.size(); ++q)
                    {
                        if(std::abs((long long)uniqueCruci[q] - (long long)cell) < cell * uniqueThreshold)
                        {
                            isUniqeCruci = false;
                        }
                    }

                    if(isUniqeCruci)
                    {
                        uniqueCruci.push_back(cell);
                    }
                }
            }
        }
    }
    maxCruci = uniqueCruci.size();
#endif

    if(isSaveMazeWithWeights)
    {
        saveMazeWithWeights("w.txt", maze, chromo);
    }

    if(isRunStepByStep)
    {
        Maze mazeStep;
        initMaze(mazeStep);

        mazeFromChromo(chromo, mazeStep);

//#define SHOW_DIFF
#define STEP_FOLDER_NAME "1"

        MazeState state;
#if 0
        state.ky = 9;
        state.kx = 28;
        state.dir = 3;
        mazeStep[9 * maxWidth + 28] = 50;
#endif
#if 0
        //state.ky = 15;
        //state.kx = 12;
        //state.dir = 2;
        mazeStep[15 * maxWidth + 12] = 173;
        //mazeStep[15 * maxWidth + 2] = 1231;
#endif
        do{
            reward = state.n;
#if defined(SHOW_DIFF)
            Maze mazeStepBefore = mazeStep;
#endif
            //runMazeStepByStep(mazeStep, state, 1e11);
            runMazeStepByStep(mazeStep, state, 1e8);
            //runMazeStepByStep(mazeStep, state, 1e6);
            //runMazeStepByStep(mazeStep, state, 1e3);
            char buf[1000];
            sprintf(buf, "%010zu", state.n);
#if !defined(SHOW_DIFF)
            std::string fName = "./" STEP_FOLDER_NAME "/w_" + std::string(buf) + ".txt";
            saveMazeWithWeights(fName.c_str(), mazeStep, chromo);
#else
            std::string fName = "./" STEP_FOLDER_NAME "_diff/w_" + std::string(buf) + ".txt";
            if(reward != state.n)
            {
                Maze mazeDiff = mazeStep;
                for(size_t q = 0; q < maxWidth * maxHeight; ++q)
                {
                    mazeDiff[q] -= mazeStepBefore[q];
                }
                saveMazeWithWeights(fName.c_str(), mazeDiff, chromo);
            }
#endif
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
                int maxCruci;
                Score reward = runGame(pop[q].val, maxCruci);

                //regularizer
#if 0
                //rewards[q] = reward + maxCruci * 10;//std::pow(2, maxCruci);

                //rewards[q] = reward + maxCruci * 1e1;//second row (14 crusiforms)
                //rewards[q] = reward + maxCruci * 1e2;//third row (20 crusiforms)
                rewards[q] = reward + Clamp(maxCruci - 14, 0, 551) * 1e5;//third row (20 crusiforms)
                //rewards[q] = reward + Clamp(maxCruci - 20, 0, 551) * 1e5;//forth row (25 crusiforms)
#else
                rewards[q] = reward;
#endif

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
        int maxCruci;
        Score reward = runGame(pop[q].val, maxCruci);

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