#include <cstdio>
#include <cstdlib>
#include <array>
#include <queue>

//parallel
#define PROCESS_THREADS
size_t threadsAmount;

const size_t width = 29;
const size_t height = 19;

//const size_t width = 5;
//const size_t height = 3;

const size_t maxWidth = width + 2;
const size_t maxHeight = height + 2;

typedef int64_t MazeData;

const MazeData wall = 1000000000;
const MazeData externalWall = 0x7FFFFFFFFFFFFFFF;

typedef std::array<MazeData, maxWidth * maxHeight> Maze;
typedef uint64_t Score;


bool isExitExists(const Maze& maze)
{
    if(maze[1 * maxWidth + 1] == wall) return false;
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
                    (visited[(i - 1) * maxWidth + j] == 1) ||
                        (visited[(i + 1) * maxWidth + j] == 1) ||
                        (visited[(i + 0) * maxWidth + j - 1] == 1) ||
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
        if((kx == width) && (ky == height))
        {
            reward = n;
            return true;
        }
        else
        {
            kx2 = kx;
            ky2 = ky;

            n++;

            down = maze[(ky + 1) * maxWidth + kx];
            right = maze[(ky + 0) * maxWidth + kx + 1];
            up = maze[(ky - 1) * maxWidth + kx];
            left = maze[(ky + 0) * maxWidth + kx - 1];

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

typedef std::array<bool, width * height> Chromo;

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

Score runGame(const Chromo& chromo, Maze& maze)
{
    mazeFromChromo(chromo, maze);

    Score reward;
    runMaze(maze, reward);

    return reward;
}


uint64_t * assignFitness(int batches, int taskSize, const uint8_t task[])
{
    typedef std::vector<Chromo> Population;
    
    Population pop(batches);
    for (size_t q = 0; q < batches; q++)
    {
        for (size_t w = 0; w < taskSize; w++)
        {
            pop[q][w] = static_cast<bool>(task[q * taskSize + w]);
        }
    }
    
    uint64_t* res = static_cast<uint64_t*>(malloc(sizeof(uint64_t) * batches * (taskSize + 1)));
    
#if defined(PROCESS_THREADS)
    size_t batchLocal = batches / threadsAmount;
    
    {
        thread_pool tPool(threadsAmount);

        auto processLambda = [&](size_t start, size_t end){
            for(size_t q = start; q < end; ++q)
            {
                Maze maze;
                initMaze(maze);
                
                Score reward = runGame(pop[q], maze);
                
                res[q * (taskSize + 1)] = reward;
                
                for(size_t w = 1; w < maxHeight - 1; ++w)
                {
                    for(size_t e = 1; e < maxWidth - 1; ++e)
                    {
                        res[q * (taskSize + 1) + (w - 1) * width + e] = maze[w * maxWidth + e];
                    }
                }
            }
        };

        for(size_t q = 0; q < threadsAmount; ++q)
        {
            size_t start = q * batchLocal;
            size_t end = (q + 1) * batchLocal;
            tPool.enqueue(processLambda, start, end);
        }
    }
    
    if(batches % threadsAmount != 0)
    {
        for(size_t q = batchLocal * threadsAmount; q < batches; ++q)
        {
            Maze maze;
            initMaze(maze);
            
            Score reward = runGame(pop[q], maze);
            
            res[q * (taskSize + 1)] = reward;
            
            for(size_t w = 1; w < maxHeight - 1; ++w)
            {
                for(size_t e = 1; e < maxWidth - 1; ++e)
                {
                    res[q * (taskSize + 1) + (w - 1) * width + e] = maze[w * maxWidth + e];
                }
            }
        }
    }

#else
    for(size_t q = 0; q < batches; ++q)
    {
        Maze maze;
        initMaze(maze);
        
        Score reward = runGame(pop[q], maze);
        
        res[q * (taskSize + 1)] = reward;
        
        for(size_t w = 1; w < maxHeight - 1; ++w)
        {
            for(size_t e = 1; e < maxWidth - 1; ++e)
            {
                res[q * (taskSize + 1) + (w - 1) * width + e] = maze[w * maxWidth + e];
            }
        }
    }
#endif

    return res;
}

uint64_t * compute(int batches, int taskSize, const uint8_t task[], int threads)
{
    threadsAmount = threads;
    
    //~ printf("%d %d\n", batches, threads);
    
#if 0
    for (size_t q = 0; q < batches; q++)
    {
        for (size_t w = 0; w < taskSize; w++)
        {
            printf("%u ", task[q * taskSize + w]);
        }
        printf("\n");
    }
    printf("\n");
#endif
    
    uint64_t* array = assignFitness(batches, taskSize, task);

    return array;
}