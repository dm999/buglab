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
TBUIForm *BUIForm;


#include "BuglabRoutiness.h"

const size_t PaddingTop = 33;
const size_t PaddingLeft = 35;
const size_t CellHeight = 26;
const size_t CellWidth = 30;

bool mouseDown = false;
bool mouseMoved = false;
bool doRecalc = true;
MazeData initialOnClick;

TCriticalSection *Lock;
TThread *Thread = NULL;
//---------------------------------------------------------------------------
__fastcall TBUIForm::TBUIForm(TComponent* Owner)
    : TForm(Owner)
{
    mBMP = new Graphics::TBitmap();
    mBMP->Width = BUIForm->Width;
    mBMP->Height = BUIForm->Height;

    initMaze(maze);
    mazeW = maze;

    Lock = new TCriticalSection();

    calcIsInProgress = false;
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::ExitExecute(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void TBUIForm::Pnt()
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo)
        {
            mBMP->Canvas->TextOutA(10, BUIForm->Height - 20, "Score: calculating...");
            return;
        }
    }

    mBMP->Canvas->Brush->Color = clBlack;
    mBMP->Canvas->Pen->Color = clRed;
    mBMP->Canvas->Rectangle(0, 0, mBMP->Width, mBMP->Height);


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
                     if(mazeW[q * maxWidth + w] < 10000)
                     {
                         char buf[1024];
                         std::sprintf(buf, "%.1f", static_cast<float>(mazeW[q * maxWidth + w]) / 1000.0f);
                         num =  AnsiString(buf) + "k";
                     }
                     else
                         num =  IntToStr((long long)mazeW[q * maxWidth + w] / 1000) + "k";
                 }
                 else if(mazeW[q * maxWidth + w] < 1000000000)
                 {
                     if(mazeW[q * maxWidth + w] < 10000000)
                     {
                         char buf[1024];
                         std::sprintf(buf, "%.1f", static_cast<float>(mazeW[q * maxWidth + w] / 1000) / 1000.0f);
                         num =  AnsiString(buf) + "M";
                     }
                     else
                         num =  IntToStr((long long)mazeW[q * maxWidth + w] / 1000000) + "M";
                 }

                 int tWidth = mBMP->Canvas->TextWidth(num);
                 int tHeight = mBMP->Canvas->TextHeight(num);
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

    mBMP->Canvas->TextOutA(10, BUIForm->Height - 20, "Score: " + AnsiString(printRewardFriendly(score).c_str()));
}

void TBUIForm::Blt()
{
     BUIForm->Canvas->CopyRect(TRect(0,0,BUIForm->Width,BUIForm->Height),mBMP->Canvas,TRect(0,0,BUIForm->Width,BUIForm->Height));
}

//---------------------------------------------------------------------------
void __fastcall TBUIForm::FormPaint(TObject *Sender)
{
    Pnt();
    Blt();
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::RecalcExecute(TObject *Sender)
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo) return;
    }

    if(doRecalc)
    {
        doRecalc = false;

        score = 0;
        Pnt();
        Blt();
        mazeW = maze;

        Lock->Acquire();
        calcIsInProgress = true;
        Lock->Release();

        if(Thread) delete Thread;

        Thread = new TCalcThread(true, Lock, BUIForm);
        Thread->Start();

        //runMaze(mazeW, score);

        Pnt();
        Blt();
    }
    else // clear
    {
        doRecalc = true;

        mazeW = maze;
        score = 0;
        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::LoadExecute(TObject *Sender)
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo) return;
    }

    if(OpenDialog1->FileName != "")
    {
        doRecalc = true;

        loadState(maze, OpenDialog1->FileName.t_str());
        mazeW = maze;
        score = 0;
        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::OpenExecute(TObject *Sender)
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo) return;
    }

    if(OpenDialog1->Execute())
    {
        doRecalc = true;

        OpenDialog1->InitialDir = ExtractFilePath(OpenDialog1->FileName);

        loadState(maze, OpenDialog1->FileName.t_str());
        mazeW = maze;
        score = 0;
        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo) return;
    }

    if(Button == mbLeft && mouseDown && !mouseMoved)
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

            doRecalc = true;

            score = 0;
            Pnt();
            Blt();
        }
    }

    mouseDown = false;
    mouseMoved = false;
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo) return;
    }

    if(Button == mbLeft)
    {
        mouseDown = true;

        if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * 29 && Y >= PaddingTop && Y <= PaddingTop + CellHeight * 19)
        {
            int x = (X - PaddingLeft) / CellWidth;
            int y = (Y - PaddingTop) / CellHeight;
            initialOnClick = maze[(y + 1) * maxWidth + x + 1];
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::SaveExecute(TObject *Sender)
{
    {
        bool continueFoo = true;
        Lock->Acquire();
        if(calcIsInProgress) continueFoo = false;
        Lock->Release();
        if(!continueFoo) return;
    }

    if(SaveDialog1->Execute())
    {
        SaveDialog1->InitialDir = ExtractFilePath(SaveDialog1->FileName);
        OpenDialog1->FileName = SaveDialog1->FileName;
        saveMaze(maze, SaveDialog1->FileName.t_str());
    }
}
//---------------------------------------------------------------------------


void __fastcall TBUIForm::FormMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y)
{
    static int x_recent = 0;
    static int y_recent = 0;

    if(mouseDown)
    {
        if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * 29 && Y >= PaddingTop && Y <= PaddingTop + CellHeight * 19)
        {
            {
                bool continueFoo = true;
                Lock->Acquire();
                if(calcIsInProgress) continueFoo = false;
                Lock->Release();
                if(!continueFoo) return;
            }

            int x = (X - PaddingLeft) / CellWidth;
            int y = (Y - PaddingTop) / CellHeight;

            if(x != x_recent || y != y_recent)
            {
                mouseMoved = true;

                x_recent = x;
                y_recent = y;

                if(initialOnClick == wall)
                {
                    maze[(y + 1) * maxWidth + x + 1] = 0;
                }
                else
                {
                    maze[(y + 1) * maxWidth + x + 1] = wall;
                }

                doRecalc = true;

                score = 0;
                Pnt();
                Blt();
            }
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::HelpExecute(TObject *Sender)
{
    Application->MessageBoxA(WideString("Hotkeys:\n"
        "Ctrl-H - show help\n"
        "Ctrl-O - open file\n"
        "Ctrl-S - save file\n"
        "Ctrl-L - reload opened file\n"
        "Space - calculate score / clear\n"
        "Escape - exit").c_bstr(), WideString("Help").c_bstr(), MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------

