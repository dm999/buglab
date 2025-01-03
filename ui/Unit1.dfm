object BUIForm: TBUIForm
  Left = 0
  Top = 0
  Caption = 'BUI'
  ClientHeight = 553
  ClientWidth = 942
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  Menu = MainMenu1
  OldCreateOrder = False
  Position = poScreenCenter
  OnMouseDown = FormMouseDown
  OnMouseLeave = FormMouseLeave
  OnMouseMove = FormMouseMove
  OnMouseUp = FormMouseUp
  OnPaint = FormPaint
  OnResize = FormResize
  PixelsPerInch = 96
  TextHeight = 13
  object ActionManager1: TActionManager
    Left = 32
    Top = 8
    StyleName = 'Platform Default'
    object Exit: TAction
      Caption = 'Exit'
      OnExecute = ExitExecute
    end
    object Recalc: TAction
      Caption = 'Recalc'
      OnExecute = RecalcExecute
    end
    object Load: TAction
      Caption = 'Load'
      OnExecute = LoadExecute
    end
    object Open: TAction
      Caption = 'Open'
      OnExecute = OpenExecute
    end
    object Save: TAction
      Caption = 'Save'
      OnExecute = SaveExecute
    end
    object Help: TAction
      Caption = 'Help'
      OnExecute = HelpExecute
    end
    object Undo: TAction
      Caption = 'Undo'
      OnExecute = UndoExecute
    end
    object Redo: TAction
      Caption = 'Redo'
      OnExecute = RedoExecute
    end
    object New: TAction
      Caption = 'New'
      OnExecute = NewExecute
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
  object Timer1: TTimer
    Enabled = False
    OnTimer = Timer1Timer
    Left = 32
    Top = 184
  end
  object MainMenu1: TMainMenu
    Left = 32
    Top = 248
    object File: TMenuItem
      Caption = 'File'
      object New1: TMenuItem
        Action = New
        ShortCut = 16462
      end
      object Open1: TMenuItem
        Action = Open
        ShortCut = 16463
      end
      object Save1: TMenuItem
        Action = Save
        ShortCut = 16467
      end
      object Load1: TMenuItem
        Action = Load
        ShortCut = 16460
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object Exit1: TMenuItem
        Action = Exit
        Caption = 'Exit | Stop calc'
        ShortCut = 27
      end
    end
    object Edit1: TMenuItem
      Caption = 'Edit'
      object Calculate1: TMenuItem
        Action = Recalc
        ShortCut = 32
      end
      object Undo1: TMenuItem
        Action = Undo
        ShortCut = 16474
      end
      object Redo1: TMenuItem
        Action = Redo
        ShortCut = 16473
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      ShortCut = 16456
      object Hotkeys1: TMenuItem
        Action = Help
      end
      object Cross1: TMenuItem
        Caption = 'Cross'
        ShortCut = 71
        OnClick = Cross1Click
      end
    end
  end
end
