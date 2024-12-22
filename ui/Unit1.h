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

private:

    bool checkCalcIsInProgress() const;

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
