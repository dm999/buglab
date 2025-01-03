//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "Unit1.h"

#include <string>
#include <vector>
#include <deque>
#include <fstream>
#include <cstdio>
#include <iostream>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TBUIForm *BUIForm;


#include "BuglabRoutiness.h"

const size_t PaddingTopRef = 33;
const size_t PaddingLeftRef = 35;

size_t PaddingTop = PaddingTopRef;
size_t PaddingLeft = PaddingLeftRef;

size_t CellHeight = 0;
size_t CellWidth = 0;

bool mouseDown = false;
bool mouseRDown = false;
bool mouseMoved = false;
bool doRecalc = true;
MazeData initialOnClick;

TCriticalSection *Lock;
TThread *Thread = NULL;

const size_t maxCtrlZHistorty = 100;
std::deque<Maze> ctrlZ;
std::deque<Maze> ctrlY;

AnsiString curFileName = "";

long long secondsPassed = 0;

int oldX = 0;
int oldY = 0;

int currCellX = -1;
int currCellY = -1;

int nonWallsCount = 551;
std::vector<MazeData> uniqueCruci;

bool isCross = false;
//---------------------------------------------------------------------------
__fastcall TBUIForm::TBUIForm(TComponent* Owner)
    : TForm(Owner)
{
    mBMP = new Graphics::TBitmap();
    mBMP->Width = BUIForm->ClientWidth;
    mBMP->Height = BUIForm->ClientHeight;

    initMaze(maze);
    mazeW = maze;

    Lock = new TCriticalSection();

    calcIsInProgress = false;

    CellHeight = (BUIForm->ClientHeight - PaddingTop * 2) / 19;
    CellWidth = (BUIForm->ClientWidth - PaddingLeft * 2) / 29;
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::ExitExecute(TObject *Sender)
{

    if(!checkCalcIsNotInProgress())
    {
         if(Thread)
         {
            Thread->Terminate();
         }

         doRecalc = true;
    }
    else
    {
         if(Thread)
         {
            Thread->Terminate();
         }

         Close();
    }
}
//---------------------------------------------------------------------------
bool TBUIForm::checkCalcIsNotInProgress() const
{
    bool continueFoo = true;
    Lock->Acquire();
    if(calcIsInProgress) continueFoo = false;
    Lock->Release();
    return continueFoo;
}
//---------------------------------------------------------------------------
void TBUIForm::calcNonWalls() const
{
    nonWallsCount = 0;
    uniqueCruci.clear();

    for(int q = 1; q <= height; ++q)
    {
        for(int w = 1; w <= width; ++w)
        {
            if(maze[q * maxWidth + w] != wall) ++nonWallsCount;
        }
    }
}
//---------------------------------------------------------------------------
void TBUIForm::processUniqueCruci(const MazeData& cell) const
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
//---------------------------------------------------------------------------
int TBUIForm::getFontSize() const
{
    int size = (BUIForm->Height / 100) + 1;

    return size;
}
//---------------------------------------------------------------------------
void TBUIForm::Pnt()
{
    mBMP->Canvas->Brush->Color = clBtnFace;
    mBMP->Canvas->Brush->Style = bsSolid;
    //mBMP->Canvas->Pen->Color = clRed;
    mBMP->Canvas->Pen->Color = clBtnFace;
    mBMP->Canvas->Rectangle(0, 0, mBMP->Width, mBMP->Height);

    uniqueCruci.clear();

    for(int q = 1; q <= height; ++q)
    {
        for(int w = 1; w <= width; ++w)
        {
            const MazeData cell = mazeW[q * maxWidth + w];

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
                 //mBMP->Canvas->Font->Color = clRed;
                 mBMP->Canvas->Font->Color = clSilver;

                 bool isCruciForm = false;

                 int countExpLarge = 0;
                 int countNearExpLarge = 0;

                 const float cruciThreshold = 1.8f;
                 const float cruciNearThreshold = 1.3f;

                 if(cell > mazeW[(q + 0) * maxWidth + w - 1] * cruciThreshold) ++countExpLarge;
                 if(cell > mazeW[(q + 0) * maxWidth + w + 1] * cruciThreshold) ++countExpLarge;
                 if(cell > mazeW[(q - 1) * maxWidth + w + 0] * cruciThreshold) ++countExpLarge;
                 if(cell > mazeW[(q + 1) * maxWidth + w + 0] * cruciThreshold) ++countExpLarge;

                 if(cell > mazeW[(q + 0) * maxWidth + w - 1] * cruciNearThreshold) ++countNearExpLarge;
                 if(cell > mazeW[(q + 0) * maxWidth + w + 1] * cruciNearThreshold) ++countNearExpLarge;
                 if(cell > mazeW[(q - 1) * maxWidth + w + 0] * cruciNearThreshold) ++countNearExpLarge;
                 if(cell > mazeW[(q + 1) * maxWidth + w + 0] * cruciNearThreshold) ++countNearExpLarge;

                 if(countNearExpLarge >= 3)
                 {
                     mBMP->Canvas->Font->Color = clYellow;

                     if(countExpLarge >= 3) mBMP->Canvas->Font->Color = clLime;

                     processUniqueCruci(cell);
                 }

                 mBMP->Canvas->Font->Size = getFontSize();
                 if(cell < 1000)
                 {
                     num =  IntToStr((long long)cell);
                 }
                 else if(cell < 1000000)
                 {
                     if(cell < 10000)
                     {
                         char buf[1024];
                         std::sprintf(buf, "%.1f", static_cast<float>(cell) / 1000.0f);
                         num =  AnsiString(buf) + "k";
                     }
                     else
                         num =  IntToStr((long long)cell / 1000) + "k";
                 }
                 else if(cell < 1000000000)
                 {
                     if(cell < 10000000)
                     {
                         char buf[1024];
                         std::sprintf(buf, "%.1f", static_cast<float>(cell / 1000) / 1000.0f);
                         num =  AnsiString(buf) + "M";
                     }
                     else
                         num =  IntToStr((long long)cell / 1000000) + "M";
                 }

                 int tWidth = mBMP->Canvas->TextWidth(num);
                 int tHeight = mBMP->Canvas->TextHeight(num);
                 mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * (w - 1) + CellWidth / 2 - tWidth / 2, PaddingTop + CellHeight * (q - 1) + CellHeight / 2 - tHeight / 2, num);
            }


        }
    }

    mBMP->Canvas->Font->Size = getFontSize() + 1;
    //mBMP->Canvas->Brush->Color = clBlack;
    mBMP->Canvas->Brush->Color = clBtnFace;
    mBMP->Canvas->Pen->Color = clRed;
    mBMP->Canvas->Font->Color = clRed;
    //mBMP->Canvas->Brush->Style = bsClear;
    mBMP->Canvas->Pen->Style = psDot;

    if(isCross)
    {
         mBMP->Canvas->Pen->Style = psDot;

        //cross horiz
        if(currCellY != -1)
        {
            mBMP->Canvas->MoveTo(PaddingLeft, PaddingTop + CellHeight * currCellY);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * width, PaddingTop + CellHeight * currCellY);

            if(currCellY < height)
            {
                mBMP->Canvas->MoveTo(PaddingLeft, PaddingTop + CellHeight * (currCellY + 1));
                mBMP->Canvas->LineTo(PaddingLeft + CellWidth * width, PaddingTop + CellHeight * (currCellY + 1));
            }

            mBMP->Canvas->Pen->Style = psSolid;

            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * currCellX, PaddingTop + CellHeight * currCellY);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * (currCellX + 1), PaddingTop + CellHeight * currCellY);

            if(currCellY < height)
            {
                mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * currCellX, PaddingTop + CellHeight * (currCellY + 1));
                mBMP->Canvas->LineTo(PaddingLeft + CellWidth * (currCellX + 1), PaddingTop + CellHeight * (currCellY + 1));
            }
        }


        mBMP->Canvas->Pen->Style = psDot;

        //cross vert
        if(currCellX != -1)
        {
            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * currCellX, PaddingTop);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * currCellX, PaddingTop + CellHeight * height);

            if(currCellX < width)
            {
                mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * (currCellX + 1), PaddingTop);
                mBMP->Canvas->LineTo(PaddingLeft + CellWidth * (currCellX + 1), PaddingTop + CellHeight * height);
            }

            mBMP->Canvas->Pen->Style = psSolid;

            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * currCellX, PaddingTop + CellHeight * currCellY);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * currCellX, PaddingTop + CellHeight * (currCellY + 1));

            if(currCellX < width)
            {
                mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * (currCellX + 1), PaddingTop + CellHeight * currCellY);
                mBMP->Canvas->LineTo(PaddingLeft + CellWidth * (currCellX + 1), PaddingTop + CellHeight * (currCellY + 1));
            }
        }
    }

    mBMP->Canvas->Pen->Style = psSolid;

    //grids
    for(int q = 0; q <= height; ++q)
    {
        if(q == 0 || q == height)
        {
            mBMP->Canvas->MoveTo(PaddingLeft, PaddingTop + CellHeight * q);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * width, PaddingTop + CellHeight * q);
        }
        else
        {
            mBMP->Canvas->MoveTo(PaddingLeft - 2, PaddingTop + CellHeight * q);
            mBMP->Canvas->LineTo(PaddingLeft + 3, PaddingTop + CellHeight * q);

            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * width - 2, PaddingTop + CellHeight * q);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * width + 3, PaddingTop + CellHeight * q);
        }

        if(q < height)
        {
             int tHeight = mBMP->Canvas->TextHeight(IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft - 20, PaddingTop + CellHeight * q + CellHeight / 2 - tHeight / 2, IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * width + 10, PaddingTop + CellHeight * q + CellHeight / 2 - tHeight / 2, IntToStr((int)height - q));
        }
    }

    for(int q = 0; q <= width; ++q)
    {
        if(q == 0 || q == width)
        {
            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * q, PaddingTop);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * q, PaddingTop + CellHeight * height);
        }
        else
        {
            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * q, PaddingTop - 2);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * q, PaddingTop + 3);

            mBMP->Canvas->MoveTo(PaddingLeft + CellWidth * q, PaddingTop + CellHeight * height - 2);
            mBMP->Canvas->LineTo(PaddingLeft + CellWidth * q, PaddingTop + CellHeight * height + 3);
        }

        if(q < width)
        {
             int tWidth = mBMP->Canvas->TextWidth(IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * q + CellWidth / 2 - tWidth / 2, PaddingTop - 20, IntToStr(q + 1));
             mBMP->Canvas->TextOutA(PaddingLeft + CellWidth * q + CellWidth / 2 - tWidth / 2, PaddingTop + CellHeight * height + 10, IntToStr((int)width - q));
        }
    }

    if(!checkCalcIsNotInProgress())
    {
        //mBMP->Canvas->Brush->Color = clBtnFace;
        //mBMP->Canvas->Brush->Style = bsSolid;
        //mBMP->Canvas->Pen->Color = clBtnFace;
        //mBMP->Canvas->Rectangle(1, BUIForm->ClientHeight - getFontSize() * 2.5, mBMP->Width - 1, mBMP->Height - 1);

        if(secondsPassed > 0)
            mBMP->Canvas->TextOutA(10, BUIForm->ClientHeight - getFontSize() * 2.5, "Score: calculating... "  + IntToStr((long long)secondsPassed));
        else
            mBMP->Canvas->TextOutA(10, BUIForm->ClientHeight - getFontSize() * 2.5, "Score: calculating...");
        //return;
    }
    else
    {
        if(curFileName != "")
            mBMP->Canvas->TextOutA(10, BUIForm->ClientHeight - getFontSize() * 2.5, "Score: " + AnsiString(printRewardFriendly(score).c_str()) + " (" + curFileName + ")" + " [" + IntToStr(nonWallsCount) + "]" + " {" + IntToStr((int)uniqueCruci.size()) + "}");
        else
             mBMP->Canvas->TextOutA(10, BUIForm->ClientHeight - getFontSize() * 2.5, "Score: " + AnsiString(printRewardFriendly(score).c_str()) + " [" + IntToStr(nonWallsCount) + "]" + " {" + IntToStr((int)uniqueCruci.size()) + "}");
    }
}
//---------------------------------------------------------------------------
void TBUIForm::Blt()
{
     BUIForm->Canvas->CopyRect(TRect(0,0,BUIForm->ClientWidth,BUIForm->ClientHeight),mBMP->Canvas,TRect(0,0,BUIForm->ClientWidth,BUIForm->ClientHeight));
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
    if(!checkCalcIsNotInProgress()) return;

    if(doRecalc)
    {
        doRecalc = false;

        secondsPassed = 0;

        score = 0;
        Pnt();
        Blt();
        mazeW = maze;

        Lock->Acquire();
        calcIsInProgress = true;
        Lock->Release();

        Timer1->Enabled = true;

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
    if(!checkCalcIsNotInProgress()) return;

    if(OpenDialog1->FileName != "")
    {
        doRecalc = true;

        ctrlZ.clear();
        ctrlY.clear();

        loadState(maze, OpenDialog1->FileName.t_str());
        mazeW = maze;
        score = 0;
        calcNonWalls();

        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::OpenExecute(TObject *Sender)
{
    if(!checkCalcIsNotInProgress()) return;

    if(OpenDialog1->Execute())
    {
        curFileName = ExtractFileName(OpenDialog1->FileName);
        BUIForm->Caption = "BUI: " + curFileName;

        doRecalc = true;

        ctrlZ.clear();
        ctrlY.clear();

        OpenDialog1->InitialDir = ExtractFilePath(OpenDialog1->FileName);

        loadState(maze, OpenDialog1->FileName.t_str());
        mazeW = maze;
        score = 0;
        calcNonWalls();

        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
    if(Button == mbLeft)
    {
        if(!checkCalcIsNotInProgress()) return;

        mouseDown = true;

        if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * 29 && Y >= PaddingTop && Y <= PaddingTop + CellHeight * 19)
        {
            int x = (X - PaddingLeft) / CellWidth;
            int y = (Y - PaddingTop) / CellHeight;
            initialOnClick = maze[(y + 1) * maxWidth + x + 1];
        }
    }

#if 0
    if(Button == mbRight)
    {
        mouseRDown = true;

        oldX = X;
        oldY = Y;

        BUIForm->Cursor = crSizeAll;
    }
#endif
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::SaveExecute(TObject *Sender)
{
    if(!checkCalcIsNotInProgress()) return;

    if(SaveDialog1->Execute())
    {
        AnsiString fileExt = ExtractFileExt(SaveDialog1->FileName);
        if(fileExt == "")
        {
            SaveDialog1->FileName = SaveDialog1->FileName + ".txt";
        }
        curFileName = ExtractFileName(SaveDialog1->FileName);
        BUIForm->Caption = "BUI: " + curFileName;

        SaveDialog1->InitialDir = ExtractFilePath(SaveDialog1->FileName);
        OpenDialog1->FileName = SaveDialog1->FileName;
        saveMaze(maze, SaveDialog1->FileName.t_str());

        Pnt();
        Blt();
    }
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y)
{
    if(Button == mbLeft && mouseDown && !mouseMoved)
    {

        if(!checkCalcIsNotInProgress()) return;

        if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * 29 && Y >= PaddingTop && Y <= PaddingTop + CellHeight * 19)
        {
            int x = (X - PaddingLeft) / CellWidth;
            int y = (Y - PaddingTop) / CellHeight;

            ctrlY.clear();
            while(ctrlZ.size() > maxCtrlZHistorty)ctrlZ.pop_front();
            ctrlZ.push_back(maze);

            if(maze[(y + 1) * maxWidth + x + 1] == wall)
            {
                maze[(y + 1) * maxWidth + x + 1] = 0;
            }
            else
            {
                maze[(y + 1) * maxWidth + x + 1] = wall;
            }

            doRecalc = true;

            calcNonWalls();

            score = 0;
            Pnt();
            Blt();
        }
    }

    BUIForm->Cursor = crDefault;

    mouseDown = false;
    mouseRDown = false;
    mouseMoved = false;
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::FormMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y)
{
    static int x_recent = 0;
    static int y_recent = 0;

    currCellX = -1;
    currCellY = -1;

    if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * width && Y >= PaddingTop && Y <= PaddingTop + CellHeight * height)
    {
        currCellX = (X - PaddingLeft) / CellWidth;
        currCellY = (Y - PaddingTop) / CellHeight;
    }

    if(mouseDown)
    {
        if(X >= PaddingLeft && X <= PaddingLeft + CellWidth * width && Y >= PaddingTop && Y <= PaddingTop + CellHeight * height)
        {
            if(!checkCalcIsNotInProgress()) return;

            int x = (X - PaddingLeft) / CellWidth;
            int y = (Y - PaddingTop) / CellHeight;

            if(x != x_recent || y != y_recent)
            {
                mouseMoved = true;

                x_recent = x;
                y_recent = y;

                ctrlY.clear();
                while(ctrlZ.size() > maxCtrlZHistorty)ctrlZ.pop_front();
                ctrlZ.push_back(maze);

                if(initialOnClick == wall)
                {
                    maze[(y + 1) * maxWidth + x + 1] = 0;
                }
                else
                {
                    maze[(y + 1) * maxWidth + x + 1] = wall;
                }

                doRecalc = true;

                calcNonWalls();

                score = 0;
            }
        }
    }

#if 0
    if(mouseRDown)
    {
        BUIForm->Left += X - oldX;
        BUIForm->Top += Y - oldY;
    }
#endif

    Pnt();
    Blt();
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::HelpExecute(TObject *Sender)
{
    Application->MessageBoxA(WideString("Hotkeys:\n"
        "Ctrl-H - show help\n"
        "Ctrl-O - open file\n"
        "Ctrl-S - save file\n"
        "Ctrl-L - reload opened file\n"
        "Ctrl-N - new\n"
        "Ctrl-Z - undo\n"
        "Ctrl-Y - redo\n"
        "Space - calculate score / clear\n"
        "Escape - stop calculation / exit").c_bstr(), WideString("Help").c_bstr(), MB_ICONINFORMATION);
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::UndoExecute(TObject *Sender)
{
    if(!checkCalcIsNotInProgress()) return;

    if(ctrlZ.empty()) return;

    ctrlY.push_back(maze);
    maze = ctrlZ[ctrlZ.size() - 1];
    ctrlZ.pop_back();

    doRecalc = true;

    calcNonWalls();

    Pnt();
    Blt();
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::RedoExecute(TObject *Sender)
{
    if(!checkCalcIsNotInProgress()) return;

    if(ctrlY.empty()) return;

    ctrlZ.push_back(maze);
    maze = ctrlY[ctrlY.size() - 1];
    ctrlY.pop_back();

    doRecalc = true;

    calcNonWalls();

    Pnt();
    Blt();
}
//---------------------------------------------------------------------------


void __fastcall TBUIForm::Timer1Timer(TObject *Sender)
{
    ++secondsPassed;

    Pnt();
    Blt();
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::NewExecute(TObject *Sender)
{
    initMaze(maze);
    mazeW = maze;
    score = 0;

    curFileName = "";

    OpenDialog1->FileName = "";
    SaveDialog1->FileName = "";

    doRecalc = true;

    calcNonWalls();

    Pnt();
    Blt();
}
//---------------------------------------------------------------------------
void __fastcall TBUIForm::FormMouseLeave(TObject *Sender)
{
    BUIForm->Cursor = crDefault;

    mouseDown = false;
    mouseRDown = false;
    mouseMoved = false;
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::FormResize(TObject *Sender)
{
    mBMP->Width = BUIForm->ClientWidth;
    mBMP->Height = BUIForm->ClientHeight;

    CellHeight = (BUIForm->ClientHeight - PaddingTop * 2 - 20) / 19;
    CellWidth = (BUIForm->ClientWidth - PaddingLeft * 2) / 29;

    BUIForm->Pnt();
    BUIForm->Blt();
}
//---------------------------------------------------------------------------

void __fastcall TBUIForm::Cross1Click(TObject *Sender)
{
    isCross = !isCross;

    Pnt();
    Blt();
}
//---------------------------------------------------------------------------

