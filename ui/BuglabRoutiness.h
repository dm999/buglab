#ifndef BUGLAB_ROUTINESS_H
#define BUGLAB_ROUTINESS_H

#include "CalcThread.h"

const size_t width = 29;
const size_t height = 19;
const size_t maxWidth = width + 2;
const size_t maxHeight = height + 2;


typedef unsigned long long MazeData;
typedef unsigned long long Score;

typedef std::vector<MazeData> Maze;

const MazeData wall = 1000000000;
const MazeData externalWall = 0xFFFFFFFFFFFFFFFF;


void initMaze(Maze& maze)
{
    maze.resize(maxWidth * maxHeight);

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

void loadState(Maze& maze, const std::string& fileName)
{
    std::string inputLine;

    std::ifstream file(fileName.c_str());
    if(file.is_open())
    {
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
    }
}

void saveMaze(const Maze& maze, const std::string& fileName)
{
    std::FILE * f = std::fopen(fileName.c_str(), "wt");
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

bool isExitExists(const Maze& maze)
{
    if(maze[1 * maxWidth + 1] != 0) return false;
    if(maze[height * maxWidth + width] != 0) return false;

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


bool runMaze(Maze& maze, Score& reward, TCalcThread * Thread)
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
        if(Thread->isTerminated()) return false;

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

std::string printRewardFriendly(Score reward)
{
    std::string out = IntToStr((long long)reward).t_str();
    std::string out2;
    for(size_t q = 0; q < out.size(); ++q)
    {
        out2.insert(out2.begin(), out[out.size() - q - 1]);
        if((q + 1) % 3 == 0 && q != out.size() - 1) out2.insert(out2.begin(), ' ');
    }
    return out2;
}

Maze maze;
Maze mazeW;
Score score = 0;

bool calcIsInProgress;



void __fastcall TCalcThread::Execute()
{
    const HRESULT iniResult = CoInitializeEx(0, COINIT_MULTITHREADED);

    if (!((iniResult == S_OK) || (iniResult == S_FALSE))) {
        ShowMessage("!");
        return;
    }

    try {
        Maze mazeWLocal = maze;
        Score scoreLocal = 0;
        bool finished = runMaze(mazeWLocal, scoreLocal, this);

        FLock->Acquire();
        calcIsInProgress = false;
        if(finished)
        {
            score = scoreLocal;
            mazeW = mazeWLocal;
        }
        FLock->Release();
    }
    catch (Exception &ex) {
        ShowMessage("!");
    }
    CoUninitialize();

    Synchronize(repaint);
    //Form->Repaint();
}

void __fastcall TCalcThread::repaint()
{
    Form->FormResize(NULL);
    //Form->Pnt();
    //Form->Blt();

    Form->Timer1->Enabled = false;
}

TCalcThread::TCalcThread(bool CreateSuspended, TCriticalSection *ALock, TBUIForm * f) : TThread(CreateSuspended)
{
    FLock = ALock;
    Form = f;
}


#endif
