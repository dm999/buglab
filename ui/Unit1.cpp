//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"

#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <iostream>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;


const size_t PaddingTop = 33;
const size_t PaddingLeft = 35;
const size_t CellHeight = 26;
const size_t CellWidth = 30;

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

bool buttonClicked = false;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
    : TForm(Owner)
{
    mBMP = new Graphics::TBitmap();
    mBMP->Width = Form1->Width;
    mBMP->Height = Form1->Height;

    initMaze(maze);
    //loadState(maze);
    mazeW = maze;
    //saveMaze(maze);

    //Maze tmp = maze;
    //runMaze(tmp, score);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ExitExecute(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void TForm1::Pnt()
{
    mBMP->Canvas->Brush->Color = clBlack;
    mBMP->Canvas->Pen->Color = clRed;
    mBMP->Canvas->Rectangle(0, 0, mBMP->Width - 1, mBMP->Height - 1);


    for(int q = 1; q <= height; ++q)
    {
        for(int w = 1; w <= width; ++w)
        {
            if(maze[q * maxWidth + w] == wall)
            {
                 mBMP->Canvas->Brush->Color = clGray;//RGB(128, 128, 128);
                 mBMP->Canvas->Pen->Color = clGray;//RGB(128, 128, 128);

                 mBMP->Canvas->Rectangle(PaddingLeft + CellWidth * (w - 1), PaddingTop + CellHeight * (q - 1), PaddingLeft + CellWidth * w, PaddingTop + CellHeight * q);

                 mBMP->Canvas->Pen->Color = clBlack;
                 mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * (w - 1), PaddingTop + CellHeight * (q - 1));
                 mBMP->Canvas->LineTo(PaddingLeft + CellWidth * w, PaddingTop + CellHeight * q);

                 mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * (w - 1), PaddingTop + CellHeight * q);
                 mBMP->Canvas->LineTo(PaddingLeft + CellWidth * w, PaddingTop + CellHeight * (q - 1));
            }
            else
            {
                 mBMP->Canvas->Brush->Color = clBlack;
                 mBMP->Canvas->Pen->Color = clBlack;
                 mBMP->Canvas->Rectangle(PaddingLeft + CellWidth * (w - 1), PaddingTop + CellHeight * (q - 1), PaddingLeft + CellWidth * w, PaddingTop + CellHeight * q);

                 AnsiString num = "";
                 mBMP->Canvas->Font->Color = clRed;
                 mBMP->Canvas->Font->Size = 7;
                 if(mazeW[q * maxWidth + w] < 1000)
                 {
                     num =  IntToStr((long long)mazeW[q * maxWidth + w]);
                 }
                 else if(mazeW[q * maxWidth + w] < 1000000)
                 {
                     num =  IntToStr((long long)mazeW[q * maxWidth + w] / 1000) + "k";
                 }
                 else if(mazeW[q * maxWidth + w] < 1000000000)
                 {
                     num =  IntToStr((long long)mazeW[q * maxWidth + w] / 1000000) + "M";
                 }

                 int tWidth = mBMP->Canvas->TextWidth(num);
                 int tHeight = mBMP->Canvas->TextWidth(num);
                 mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * (w - 1) + CellWidth / 2 - tWidth / 2, PaddingTop + CellHeight * (q - 1) + CellHeight / 2 - tHeight / 2, num);
            }


        }
    }

    mBMP->Canvas->Font->Size = 8;
    mBMP->Canvas->Brush->Color = clBlack;
    mBMP->Canvas->Pen->Color = clRed;
    mBMP->Canvas->Font->Color = clRed;

    for(int q = 0; q <= 19; ++q)
    {
        if(q == 0 || q == 19)
        {
            mBMP->Canvas->MoveTo(PaddingLeft, PaddingTop + CellHeight * q);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * 29, PaddingTop + CellHeight * q);
        }
        if(q < 19)
        {
             int tHeight = mBMP->Canvas->TextHeight(IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft - 20, PaddingTop + CellHeight * q + CellHeight / 2 - tHeight / 2, IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * 29 + 10, PaddingTop + CellHeight * q + CellHeight / 2 - tHeight / 2, IntToStr(19 - q));
        }
    }

    for(int q = 0; q <= 29; ++q)
    {
        if(q == 0 || q == 29)
        {
            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * q, PaddingTop);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * q, PaddingTop + CellHeight * 19);
        }
        if(q < 29)
        {
             int tWidth = mBMP->Canvas->TextWidth(IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * q + CellWidth / 2 - tWidth / 2, PaddingTop - 20, IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * q + CellWidth / 2 - tWidth / 2, PaddingTop + CellHeight * 19 + 10, IntToStr(29 - q));
        }
    }

    mBMP->Canvas->TextOutA(10, Form1->Height - 20, "Score: " + AnsiString(printRewardFriendly(score).c_str()));
}

void TForm1::Blt()
{
     Form1->Canvas->CopyRect(TRect(0,0,Form1->Width,Form1->Height),mBMP->Canvas,TRect(0,0,Form1->Width,Form1->Height));
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormPaint(TObject *Sender)
{
    Pnt();
    Blt();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RecalcExecute(TObject *Sender)
{
    score = 0;
    Pnt();
    Blt();
    mazeW = maze;
    runMaze(mazeW, score);
    Pnt();
    Blt();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadExecute(TObject *Sender)
{
    //loadState(maze, "d:/Mj/GitWC/Buglab/ui/map.txt");
    //mazeW = maze;
    //score = 0;
    //Pnt();
    //Blt();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::OpenExecute(TObject *Sender)
{
    //if(OpenDialog1->InitialDir == "") OpenDialog1->InitialDir = GetCurrentDir();
    if(OpenDialog1->Execute())
    {
        loadState(maze, OpenDialog1->FileName.t_str());
        mazeW = maze;
        score = 0;
        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
    if(Button == mbLeft && buttonClicked)
    {
        if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * 29 && Y >= PaddingTop && Y <= PaddingTop + CellHeight * 19)
        {
            int x = (X - PaddingLeft) / CellWidth;
            int y = (Y - PaddingTop) / CellHeight;
            if(maze[(y + 1) * maxWidth + x + 1] == wall)
            {
                maze[(y + 1) * maxWidth + x + 1] = 0;
            }
            else
            {
                maze[(y + 1) * maxWidth + x + 1] = wall;
            }

            //mazeW = maze;
            score = 0;
            Pnt();
            Blt();
        }
    }

    buttonClicked = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
    if(Button == mbLeft)
    {
        buttonClicked = true;
    }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::SaveExecute(TObject *Sender)
{
    if(SaveDialog1->Execute())
    {
        saveMaze(maze, SaveDialog1->FileName.t_str());
    }
}
//---------------------------------------------------------------------------

