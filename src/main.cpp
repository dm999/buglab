#include <cstdio>
#include <array>
#include <cstring>

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

int main()
{
    Maze maze;
    initMaze(maze);

    //maze[1 * maxWidth + 2] = wall;
    //maze[2 * maxWidth + 2] = wall;
    //maze[2 * maxWidth + 1] = wall;

    //maze[18 * maxWidth + 28] = wall;
    //maze[18 * maxWidth + 29] = wall;
    //maze[19 * maxWidth + 28] = wall;

    printMaze(maze);

    size_t reward;
    bool isExit = runMaze(maze, reward);
    printf("reward = %zd\n", reward);

    return 0;
}