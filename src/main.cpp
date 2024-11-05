#include <cstdio>
#include <array>
#include <vector>
#include <cstring>
#include <random>
#include <numeric>
#include <queue>
#include <string>
#include <fstream>
#include <iostream>

//bugged?
//#define BUGGED

//run only single game
//#define RUN_SINGLE_GAME_ONLY

//genetics
#if !defined(BUGGED)
const size_t populationSize = 1000;
const size_t elite = 100;
#else
const size_t populationSize = 10;
const size_t elite = 50;
#endif
const size_t populationSizeAndElite = populationSize + elite;
const float crossoverRate = 1.0f;
const float mutationRate = 0.002f;
//https://cstheory.stackexchange.com/questions/14758/tournament-selection-in-genetic-algorithms
enum SelectionAlgo
{
    AlgoRoulette,
    AlgoTournament
};
//SelectionAlgo SelAlgo = SelectionAlgo::AlgoRoulette;
SelectionAlgo SelAlgo = SelectionAlgo::AlgoTournament;
size_t tournamentSize = 4;

//diff evolution
const float CR = 0.5f;
const float F = 0.5f;

//parallel
#define PROCESS_THREADS
#define PROCESS_THREADS_GEN
const size_t threadsAmount = 10;
const size_t threadsAmountGen = 10;

//other
#if !defined(BUGGED)
size_t refreshEvery = 200;
#else
size_t refreshEvery = 0xFFFFFFFF;
#endif
size_t redefineSeedIfNoChangeEvery = 0xFFFFFFF;

//main
const size_t width = 29;
const size_t height = 19;

const size_t maxWidth = width + 2;
const size_t maxHeight = height + 2;

typedef int64_t MazeData;

const MazeData wall = 1000000000;
const MazeData externalWall = 0x7FFFFFFFFFFFFFFF;

typedef std::array<MazeData, maxWidth * maxHeight> Maze;
typedef uint64_t Score;

#if defined(BUGGED)
const char* State = "map_bug.txt";
#else
const char* State = "map.txt";
#endif

void initMaze(Maze& maze)
{
    memset(&maze[0], 0, sizeof(MazeData) * maxWidth * maxHeight);

    //border
    for(size_t q = 0; q < maxWidth; ++q)
    {
        maze[q] = externalWall;
        maze[(maxHeight - 1) * maxWidth + q] = externalWall;
    }

    for(size_t q = 0; q < maxHeight; ++q)
    {
        maze[q * maxWidth] = externalWall;
        maze[q * maxWidth + maxWidth - 1] = externalWall;
    }
}

void saveMaze(const Maze& maze)
{
    FILE * f = fopen(State, "wt");
    if(f)
    {
        for(size_t q = 0; q < maxHeight; ++q)
        {
            for(size_t w = 0; w < maxWidth; ++w)
            {
                if(maze[q * maxWidth + w] == wall || maze[q * maxWidth + w] == externalWall) fprintf(f, "#");
                else fprintf(f, ".");
            }

            fprintf(f, "\n");
        }

        fclose(f);
    }
}

void printMaze(const Maze& maze)
{
    for(size_t q = 0; q < maxHeight; ++q)
    {
        for(size_t w = 0; w < maxWidth; ++w)
        {
            if(maze[q * maxWidth + w] == wall) printf("#");
            else if(maze[q * maxWidth + w] == externalWall) printf("*");
            else printf(".");
        }
        printf("\n");
    }
}

bool isExitExists(const Maze& maze)
{
#if !defined(BUGGED)
    if(maze[1 * maxWidth + 1] == wall) return false;//comment this line to get >2B score
#endif
    if(maze[height * maxWidth + width] == wall) return false;

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

bool runMaze(Maze& maze, Score& reward)
{
    reward = 0;

    if(!isExitExists(maze)) return false;

    int kx = 1, ky = 1;
    Score n = 0;
    int dir = 0;
    int kx2, ky2;
    MazeData down, right, up, left, cur;

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

//rnd
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution dist(0.0f, 1.0f);
std::uniform_int_distribution<> iDist(0, populationSize - 1);

typedef std::array<bool, width * height> Chromo;
struct PopElement
{
    Chromo val;
    Score fitness;
};

typedef std::vector<PopElement> Population;

void initPopulation(Population& pop)
{
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
            maze[(q + 1) * maxWidth + w + 1] = chromo[q * width + w] * wall;
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

Score runGame(const Chromo& chromo)
{
    Maze maze;
    initMaze(maze);

    mazeFromChromo(chromo, maze);

    Score reward;
    bool isExit = runMaze(maze, reward);

    return reward;
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



#include <mutex>
#include <thread>
#include <vector>
#include <condition_variable>
#include <functional>

//https://github.com/agruzdev/Yato/blob/master/modules/actors/yato/actors/private/thread_pool.h
class thread_pool
{
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_tasks;

    std::mutex m_mutex;
    std::condition_variable m_cvar;

    bool m_stop = false;

public:
    thread_pool(size_t threads_num) {
        auto thread_function = [this] {
            for(;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cvar.wait(lock, [this] { return m_stop || !m_tasks.empty(); });
                    if(m_stop && m_tasks.empty()) {
                        break;
                    }
                    if(!m_tasks.empty()) {
                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }
                }
                // Execute
                if(task) {
                    try {
                        task();
                    }
                    catch(std::exception & e) {
                        (void)e;
                        //TODO
                    }
                    catch(...) {
                        //TODO
                    }
                }
            }
        };

        for(size_t i = 0; i < threads_num; ++i) {
            m_threads.emplace_back(thread_function);
        }
    }

    ~thread_pool() {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
            m_cvar.notify_all();
        }
        for(auto & thread : m_threads) {
            thread.join();
        }
    }

    thread_pool(const thread_pool&) = delete;
    thread_pool& operator=(const thread_pool&) = delete;

    // Add task
    template <typename FunTy_, typename ... Args_>
    void enqueue(FunTy_ && function, Args_ && ... args) {
        std::unique_lock<std::mutex> lock(m_mutex);
        if(!m_stop) {
            m_tasks.emplace(std::bind(std::forward<FunTy_>(function), std::forward<Args_>(args)...));
        }
        else {
            //TODO
        }
        m_cvar.notify_one();
    }
};

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

void mutate(PopElement& elem)
{
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
    size_t selected = 0xFFFFFFFF;
    for(size_t q = 0; q < tournamentSize; ++q)
    {
        size_t selectedTmp = iDist(gen);
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

    while(1)
    {
        if(epoch % refreshEvery == 0)//refresh
        {
            printf("Refresh population with best chromo\n");
            for(size_t q = 0; q < populationSizeAndElite; ++q)
            {
                pop[q].val = bestTotal;
            }
        }

        if(epochsWithoutUpdate >= redefineSeedIfNoChangeEvery)
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
            resultUpdated = true;
            bestTotal = best;
            totalBestReward = bestReward;
            epochsWithoutUpdate = 0;

            {
                Maze maze;
                initMaze(maze);
                mazeFromChromo(bestTotal, maze);
                saveMaze(maze);
            }
        }

        //elitism
        for(size_t q = populationSize; q < populationSizeAndElite; ++q)
        {
            pop[q].val = bestTotal;
            pop[q].fitness = totalBestReward;
        }

        //printf("total best reward: %zu best reward: %zu ", totalBestReward, bestReward);
        std::cout << "reward: " << totalBestReward << " (" << printRewardFriendly(totalBestReward) << ")" << "  reward iter: " << bestReward << " ";

        Score totalFitness = 0;
        if(SelAlgo == SelectionAlgo::AlgoRoulette)
        {
            totalFitness = std::accumulate(pop.begin(), pop.end(), static_cast<Score>(0), [](Score init, const PopElement& elem){ return init + elem.fitness; });
        }

#if defined(PROCESS_THREADS_GEN)
        {
            thread_pool tPool(threadsAmountGen);

            auto processLambda = [&](size_t start, size_t end){
                for(size_t q = start; q < end; q += 2)
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
                        elemA = tournament(pop, tournamentSize);
                        elemB = tournament(pop, tournamentSize);
                    }
                    std::pair<PopElement, PopElement> cross = crossover(elemA, elemB);
                    mutate(cross.first);
                    mutate(cross.second);
                    //if(!resultUpdated || bestIndex != q || bestIndex != q + 1)
                    {
                        pop[q + 0] = cross.first;
                        pop[q + 1] = cross.second;
                    }
                }
            };

            size_t batch = populationSizeAndElite / threadsAmountGen;

            for(size_t q = 0; q < threadsAmountGen; ++q)
            {
                size_t start = q * batch;
                size_t end = (q + 1) * batch;
                tPool.enqueue(processLambda, start, end);
            }
        }
#else
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
                elemA = tournament(pop, tournamentSize);
                elemB = tournament(pop, tournamentSize);
            }
            std::pair<PopElement, PopElement> cross = crossover(elemA, elemB);
            mutate(cross.first);
            mutate(cross.second);
            //if(!resultUpdated || bestIndex != q || bestIndex != q + 1)
            {
                pop[q + 0] = cross.first;
                pop[q + 1] = cross.second;
            }
        }
#endif

        long long timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        float ms = static_cast<float>(timeTaken) / 1000.0f / 1000.0f;

        printf("epoch time = %5.1fs\n", ms);

        ++epoch;
    }

}

//https://en.wikipedia.org/wiki/Differential_evolution
void diffEvolutionSearch(Population& pop)
{
    size_t epoch = 1;

    Score totalBestReward = 0;

    {
        Chromo best;
        size_t bestIndex;
        Score bestReward = assignFitness(pop, best, bestIndex);
        if(bestReward > totalBestReward)
        {
            totalBestReward = bestReward;
        }
    }

    while(1)
    {
        printf("Epoch: %5zd ", epoch);

        std::uniform_int_distribution<> iDist(0, populationSize - 1);
        std::uniform_real_distribution rDist(0.0f, 1.0f);

        Score bestReward = 0;

        std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();

        for(size_t q = 0; q < populationSize; ++q)
        {
            size_t indexA = iDist(gen), indexB = iDist(gen), indexC = iDist(gen);

            while(indexA == q) indexA = iDist(gen);
            while(indexB == q || indexB == indexA) indexB = iDist(gen);
            while(indexC == q || indexC == indexA || indexC == indexB) indexC = iDist(gen);

            size_t R = static_cast<size_t>(rDist(gen) * width * height) - 1;

            Chromo newElem;
            newElem = pop[q].val;

            //https://arxiv.org/pdf/1812.03513
            for(size_t w = 0; w < newElem.size(); ++w)
            {
                float r = rDist(gen);
                float f = rDist(gen);

                if(pop[indexB].val[w] != pop[indexC].val[w] && f < F)
                {
                    newElem[w] = !pop[indexA].val[w];
                }
                else
                {
                    newElem[w] = pop[indexA].val[w];
                }

                if(r > CR || w == R)
                {
                    newElem[w] = pop[q].val[w];
                }

                //if(r < CR || w == R)
                //{
                    //newElem[w] = static_cast<bool>((static_cast<float>(pop[indexA].val[w]) + F * (static_cast<float>(pop[indexB].val[w]) - static_cast<float>(pop[indexC].val[w]))) + 0.5f);
                //}
            }

            Score reward = runGame(newElem);

            if(reward > bestReward)
            {
                bestReward = reward;
            }

            if(reward >= pop[q].fitness)
            {
                pop[q].fitness = reward;
                pop[q].val = newElem;

                if(reward > totalBestReward)
                {
                    totalBestReward = reward;

                    {
                        Maze maze;
                        initMaze(maze);
                        mazeFromChromo(pop[q].val, maze);
                        saveMaze(maze);
                    }
                }
            }
        }

        long long timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
        float ms = static_cast<float>(timeTaken) / 1000.0f / 1000.0f;

        std::cout << "reward: " << totalBestReward << " (" << printRewardFriendly(totalBestReward) << ")" << "  reward iter: " << bestReward << " ";
        printf("epoch time = %5.1f\n", ms);

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
    }

#if defined(PROCESS_THREADS)
    printf("Threads: %zd\n", threadsAmount);
    if(threadsAmount > populationSize || populationSize % threadsAmount != 0)
    {
        printf("Incorrect threads amount\n");
        return 0;
    }
#endif

#if defined(PROCESS_THREADS_GEN)
    printf("Threads for gen: %zd\n", threadsAmountGen);
    if(threadsAmountGen > populationSizeAndElite || populationSizeAndElite % threadsAmountGen != 0 || (populationSizeAndElite / threadsAmountGen) % 2 != 0)
    {
        printf("Incorrect threads for gen amount\n");
        return 0;
    }
#endif

    geneticSearch(pop);
    //diffEvolutionSearch(pop);
#else

    Population pop;
    initPopulation(pop);
    loadState(pop);
    printf("Run single game: ");
    std::chrono::steady_clock::time_point timeStart = std::chrono::steady_clock::now();
    Score reward = runGame(pop[0].val);
    long long timeTaken = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - timeStart).count();
    float ms = static_cast<float>(timeTaken) / 1000.0f / 1000.0f;
    std::cout << "reward = " << reward << " (" << printRewardFriendly(reward) << ") ";
    printf("time = %.1fs\n", ms);

#endif
    return 0;
}