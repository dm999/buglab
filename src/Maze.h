#ifndef MAZE_H
#define MAZE_H

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

void saveMazeWithWeights(const std::string& fileName, const Maze& maze)
{
    FILE * f = fopen(fileName.c_str(), "wt");
    if(f)
    {
        for(size_t q = 1; q < maxHeight - 1; ++q)
        {
            for(size_t w = 1; w < maxWidth - 1; ++w)
            {
                if(maze[q * maxWidth + w] == wall) fprintf(f, "%6s ", "#");
                else fprintf(f, "%6zd ", maze[q * maxWidth + w]);
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

#endif
