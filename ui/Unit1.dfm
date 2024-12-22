object BUIForm: TBUIForm
  Left = 0
  Top = 0
  BorderStyle = bsNone
  Caption = 'BUI'
  ClientHeight = 580
  ClientWidth = 950
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnMouseDown = FormMouseDown
  OnMouseMove = FormMouseMove
  OnMouseUp = FormMouseUp
  OnPaint = FormPaint
  PixelsPerInch = 96
  TextHeight = 13
  object ActionManager1: TActionManager
    Left = 32
    Top = 8
    StyleName = 'Platform Default'
    object Exit: TAction
      Caption = 'Exit'
      ShortCut = 27
      OnExecute = ExitExecute
    end
    object Recalc: TAction
      Caption = 'Recalc'
      ShortCut = 32
      OnExecute = RecalcExecute
    end
    object Load: TAction
      Caption = 'Load'
      ShortCut = 16460
      OnExecute = LoadExecute
    end
    object Open: TAction
      Caption = 'Open'
      ShortCut = 16463
      OnExecute = OpenExecute
    end
    object Save: TAction
      Caption = 'Save'
      ShortCut = 16467
      OnExecute = SaveExecute
    end
    object Help: TAction
      Caption = 'Help'
      ShortCut = 16456
      OnExecute = HelpExecute
    end
    object Undo: TAction
      Caption = 'Undo'
      ShortCut = 16474
      OnExecute = UndoExecute
    end
    object Redo: TAction
      Caption = 'Redo'
      ShortCut = 16473
      OnExecute = RedoExecute
    end
  end
  object OpenDialog1: TOpenDialog
    Filter = 'Text|*.txt'
    Options = [ofReadOnly, ofHideReadOnly, ofNoChangeDir, ofPathMustExist, ofFileMustExist, ofEnableSizing]
    Left = 32
    Top = 64
  end
  object SaveDialog1: TSaveDialog
    Filter = 'Text|*.txt'
    Options = [ofHideReadOnly, ofNoChangeDir, ofEnableSizing]
    Left = 32
    Top = 120
  end
end
