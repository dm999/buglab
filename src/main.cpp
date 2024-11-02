#include <cstdio>
#include <array>
#include <vector>
#include <cstring>
#include <random>

const size_t width = 29;
const size_t height = 19;

const size_t maxWidth = width + 2;
const size_t maxHeight = height + 2;

const size_t wall = 1000000000;

typedef std::array<size_t, maxWidth * maxHeight> Maze;

void initMaze(Maze& maze)
{
    memset(&maze[0], 0, sizeof(size_t) * maxWidth * maxHeight);

    //border
    for(size_t q = 0; q < maxWidth; ++q)
    {
        maze[q] = wall;
        maze[(maxHeight - 1) * maxWidth + q] = wall;
    }

    for(size_t q = 0; q < maxHeight; ++q)
    {
        maze[q * maxWidth] = wall;
        maze[q * maxWidth + maxWidth - 1] = wall;
    }
}

void printMaze(const Maze& maze)
{
    FILE * f = fopen("map.txt", "wt");
    if(f)
    {
        for(size_t q = 0; q < maxHeight; ++q)
        {
            for(size_t w = 0; w < maxWidth; ++w)
            {
                if(maze[q * maxWidth + w] == wall) fprintf(f, "#");
                else fprintf(f, ".");
            }

            fprintf(f, "\n");
        }

        fclose(f);
    }
}

bool isExitExists(const Maze& maze)
{
    Maze visited(maze);

    bool ok;
    visited[maxWidth + 1] = 1;

    do{
        ok = false;
        for(size_t i = 1; i < maxHeight - 1; i++)
        {
            for(size_t j = 1; j < maxWidth - 1; j++)
            {
                if(
                    (visited[i * maxWidth + j] == 0) &&
                    (
                        (visited[(i - 1) * maxWidth + j] == 1)        ||
                        (visited[(i + 1) * maxWidth + j] == 1)        ||
                        (visited[(i + 0) * maxWidth + j - 1] == 1)    ||
                        (visited[(i + 0) * maxWidth + j + 1] == 1)
                    )
                )
                {
                    ok = true;
                    visited[i * maxWidth + j] = 1;
                    if((i == height) && (j == width)) return true;
                }
            }
        }
    } while(ok);

    return false;
}

bool runMaze(Maze& maze, size_t& reward)
{
    reward = 0;

    if(!isExitExists(maze)) return false;

    int kx = 1, ky = 1;
    size_t n = 0;
    int dir = 0;
    int kx2, ky2, down, right, up, left, cur;

    while(1)
    {
        if((kx == 29) && (ky == 19))
        {
            reward = n;
            return true;
        }
        else
        {
            kx2 = kx;
            ky2 = ky;

            n++;

            down    = maze[(ky + 1) * maxWidth + kx];
            right   = maze[(ky + 0) * maxWidth + kx + 1];
            up      = maze[(ky - 1) * maxWidth + kx];
            left    = maze[(ky + 0) * maxWidth + kx - 1];

            if(dir == 0) cur = down;
            if(dir == 1) cur = right;
            if(dir == 2) cur = up;
            if(dir == 3) cur = left;

            if((cur <= down) && (cur <= right) && (cur <= up) && (cur <= left))
            {
                if(dir == 0) ky2++;
                if(dir == 1) kx2++;
                if(dir == 2) ky2--;
                if(dir == 3) kx2--;
            }
            else if((down <= right) && (down <= up) && (down <= left))
            {
                ky2++; 
                dir = 0;
            }
            else if((right <= down) && (right <= up) && (right <= left))
            {
                kx2++;
                dir = 1;
            }
            else if((up <= right) && (up <= down) && (up <= left))
            {
                ky2--;
                dir = 2;
            }
            else if((left <= right) && (left <= down) && (left <= up))
            {
                kx2--;
                dir = 3;
            }

            maze[ky * maxWidth + kx]++;
            kx = kx2; 
            ky = ky2;
        }
    }

    return true;
}

std::random_device rd;
typedef std::array<bool, width * height> Chromo;
struct PopElement
{
    Chromo val;
    float fitness;
};

typedef std::vector<PopElement> Population;

const size_t populationSize = 100;
const float crossoverRate = 0.7f;
const float mutationRate = 0.001f;

void initPopulation(Population& pop)
{
    pop.resize(populationSize);

    std::uniform_real_distribution dist(0.0f, 1.0f);

    for(size_t q = 0; q < populationSize; ++q)
    {
        for(size_t w = 0; w < width * height; ++w)
        {
            float rnd = dist(rd);
            if(rnd > 0.5f) pop[q].val[w] = true;
            else pop[q].val[w] = false;

            pop[q].fitness = 0.0f;
        }
    }
}

void mazeFromChromo(const Chromo& chromo, Maze& maze)
{
    for(size_t q = 0; q < height; ++q)
    {
        for(size_t w = 0; w < width; ++w)
        {
            maze[(q + 1) * maxWidth + w + 1] = chromo[q * width + w] * wall;
        }
    }
}

size_t runGame(const Chromo& chromo)
{
    Maze maze;
    initMaze(maze);

    mazeFromChromo(chromo, maze);

    //maze[1 * maxWidth + 2] = wall;
    //maze[2 * maxWidth + 2] = wall;
    //maze[2 * maxWidth + 1] = wall;

    //maze[18 * maxWidth + 28] = wall;
    //maze[18 * maxWidth + 29] = wall;
    //maze[19 * maxWidth + 28] = wall;

    //printMaze(maze);


    size_t reward;
    bool isExit = runMaze(maze, reward);

    return reward;
}



size_t assignFitness(Population& pop, Chromo& best)
{
    size_t bestReward = 0;

    for(size_t q = 0; q < populationSize; ++q)
    {
        size_t reward = runGame(pop[q].val);

        if(reward > bestReward)
        {
            bestReward = reward;
            best = pop[q].val;
        }

        pop[q].fitness = reward;
        if(reward == 0) pop[q].fitness = 0.1f;
    }

    return bestReward;
}

void mutate(PopElement& elem)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    for(size_t w = 0; w < elem.val.size(); ++w)
    {
        if(dist(rd) < mutationRate)
        {
            elem.val[w] = !elem.val[w];
        }
    }
}

std::pair<PopElement, PopElement> crossover(const PopElement& elemA, const PopElement& elemB)
{
    std::pair<PopElement, PopElement> ret(elemA, elemB);

    std::uniform_real_distribution dist(0.0f, 1.0f);

    if(dist(rd) < crossoverRate)
    {
        size_t crossover = static_cast<size_t>(dist(rd) * width * height);

        std::copy(elemA.val.begin(), elemA.val.begin() + crossover, ret.first.val.begin());
        std::copy(elemB.val.begin() + crossover, elemB.val.end(), ret.first.val.begin() + crossover);

        std::copy(elemB.val.begin(), elemB.val.begin() + crossover, ret.second.val.begin());
        std::copy(elemA.val.begin() + crossover, elemA.val.end(), ret.second.val.begin() + crossover);
    }

    return ret;
}

PopElement roulette(const Population& pop, float totalFitness)
{
    std::uniform_real_distribution dist(0.0f, 1.0f);

    float slice = dist(rd) * totalFitness;

    float fitnesThreshold = 0.0f;
    for(size_t q = 0; q < populationSize; ++q)
    {
        fitnesThreshold += pop[q].fitness;
        if(fitnesThreshold >= slice)
        {
            return pop[q];
        }
    }

    return pop[populationSize - 1];
}

int main()
{

    Population pop;
    initPopulation(pop);

    size_t epoch = 1;
    const size_t maxEpoch = 100000;

    Chromo bestTotal;
    size_t totalBestReward = 0;

    while(epoch < maxEpoch)
    {
        printf("Epoch: %zd\n", epoch);

        Chromo best;
        size_t bestReward = assignFitness(pop, best);
        if(bestReward > totalBestReward)
        {
            bestTotal = best;
            totalBestReward = bestReward;
        }

        printf("total best reward: %zd best reward: %zd\n", totalBestReward, bestReward);

        float totalFitness = 0.0f;
        for(size_t q = 0; q < populationSize; ++q)
        {
            totalFitness += pop[q].fitness;
        }
        
        for(size_t q = 0; q < populationSize; q += 2)
        {
            PopElement elemA = roulette(pop, totalFitness);
            PopElement elemB = roulette(pop, totalFitness);
            std::pair<PopElement, PopElement> cross = crossover(elemA, elemB);
            mutate(cross.first);
            mutate(cross.second);
            pop[q + 0] = cross.first;
            pop[q + 1] = cross.second;
        }

        ++epoch;
    }

    //size_t reward = runGame(pop[0].val);
    //printf("reward = %zd\n", reward);
    {
        Maze maze;
        initMaze(maze);
        mazeFromChromo(bestTotal, maze);
        printMaze(maze);
    }

    return 0;
}