//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ActnList.hpp>
#include <ActnMan.hpp>
#include <PlatformDefaultStyleActnCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>

typedef unsigned long long MazeData;
//---------------------------------------------------------------------------
class TBUIForm : public TForm
{
__published:	// IDE-managed Components
    TActionManager *ActionManager1;
    TAction *Exit;
    TAction *Recalc;
    TAction *Load;
    TAction *Open;
    TOpenDialog *OpenDialog1;
    TSaveDialog *SaveDialog1;
    TAction *Save;
    TAction *Help;
    TAction *Undo;
    TAction *Redo;
    TTimer *Timer1;
    TAction *New;
    TMainMenu *MainMenu1;
    TMenuItem *File;
    TMenuItem *New1;
    TMenuItem *Open1;
    TMenuItem *Save1;
    TMenuItem *Load1;
    TMenuItem *N1;
    TMenuItem *Exit1;
    TMenuItem *Edit1;
    TMenuItem *Calculate1;
    TMenuItem *Undo1;
    TMenuItem *Redo1;
    TMenuItem *Help1;
    TMenuItem *Hotkeys1;
    TMenuItem *Cross1;
    void __fastcall ExitExecute(TObject *Sender);
    void __fastcall FormPaint(TObject *Sender);
    void __fastcall RecalcExecute(TObject *Sender);
    void __fastcall LoadExecute(TObject *Sender);
    void __fastcall OpenExecute(TObject *Sender);
    void __fastcall FormMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
    void __fastcall FormMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
    void __fastcall SaveExecute(TObject *Sender);
    void __fastcall FormMouseMove(TObject *Sender, TShiftState Shift, int X, int Y);
    void __fastcall HelpExecute(TObject *Sender);
    void __fastcall UndoExecute(TObject *Sender);
    void __fastcall RedoExecute(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall NewExecute(TObject *Sender);
    void __fastcall FormMouseLeave(TObject *Sender);
    void __fastcall FormResize(TObject *Sender);
    void __fastcall Cross1Click(TObject *Sender);

private:

    bool checkCalcIsNotInProgress() const;

    void calcNonWalls() const;
    void processUniqueCruci(const MazeData& cell) const;

    int getFontSize() const;

public:

       Graphics::TBitmap * mBMP;

       void Pnt();
       void Blt();

    __fastcall TBUIForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TBUIForm *BUIForm;
//---------------------------------------------------------------------------
#endif
