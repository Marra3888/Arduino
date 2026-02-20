
#Include <GUIConstants.au3>

#include <ButtonConstants.au3>
#include <EditConstants.au3>
#include <GUIConstantsEx.au3>
#include <WindowsConstants.au3>

$Form1 = GUICreate("Text Generator", 293, 212, 192, 124)
$Input1 = GUICtrlCreateInput("", 8, 8, 281, 21)
GUICtrlSetLimit(-1, 40)
$Edit1 = GUICtrlCreateEdit("", 8, 80, 209, 113, $ES_AUTOVSCROLL+$WS_VSCROLL)
GUICtrlSetData(-1, "")
$Label1 = GUICtrlCreateLabel("", 64, 48, 200, 24)
GUICtrlSetFont(-1, 12, 800, 0, "MS Sans Serif")
$Button1 = GUICtrlCreateButton("Copy", 224, 112, 59, 49, $WS_GROUP)
GUIRegisterMsg($WM_COMMAND, "_WM_COMMAND")
GUISetState()

Do
Until GUIGetMsg() = $GUI_EVENT_CLOSE

Func _WM_COMMAND($hWnd, $iMsg, $wParam, $lParam)
    Switch BitAND($wParam, 0xFFFF)
        Case $Input1
            Switch BitShift($wParam, 16)
                Case $EN_CHANGE
                    Local $Data = GUICtrlRead($Input1)
					$Data = StringReplace($Data, "А", "A")
					$Data = StringReplace($Data, "Б", "\260")
					$Data = StringReplace($Data, "В", "B")
					$Data = StringReplace($Data, "Г", "\261")
					$Data = StringReplace($Data, "Д", "\262")
					$Data = StringReplace($Data, "Е", "E")
					$Data = StringReplace($Data, "Ё", "\323")
					$Data = StringReplace($Data, "Ж", "\263")
					$Data = StringReplace($Data, "З", "\264")
					$Data = StringReplace($Data, "И", "\270")
					$Data = StringReplace($Data, "Й", "\271")
					$Data = StringReplace($Data, "К", "K")
					$Data = StringReplace($Data, "Л", "\272")
					$Data = StringReplace($Data, "М", "M")
					$Data = StringReplace($Data, "Н", "H")
					$Data = StringReplace($Data, "О", "O")
					$Data = StringReplace($Data, "П", "\273")
					$Data = StringReplace($Data, "Р", "P")
					$Data = StringReplace($Data, "С", "C")
					$Data = StringReplace($Data, "Т", "T")
					$Data = StringReplace($Data, "У", "\274")
					$Data = StringReplace($Data, "Ф", "\277")
					$Data = StringReplace($Data, "Х", "X")
					$Data = StringReplace($Data, "Ц", "\300")
					$Data = StringReplace($Data, "Ч", "\301")
					$Data = StringReplace($Data, "Ш", "\302")
					$Data = StringReplace($Data, "Щ", "\303")
					$Data = StringReplace($Data, "Ъ", "\304")
					$Data = StringReplace($Data, "Ы", "\305")
					$Data = StringReplace($Data, "Ь", "b")
					$Data = StringReplace($Data, "Э", "\310")
					$Data = StringReplace($Data, "Ю", "\311")
					$Data = StringReplace($Data, "Я", "\312")
					$Data = StringReplace($Data, "Ї", "\330")

					$Data = StringReplace($Data, "а", "A")
					$Data = StringReplace($Data, "б", "\260")
					$Data = StringReplace($Data, "в", "B")
					$Data = StringReplace($Data, "г", "\261")
					$Data = StringReplace($Data, "д", "\262")
					$Data = StringReplace($Data, "е", "E")
					$Data = StringReplace($Data, "ё", "\323")
					$Data = StringReplace($Data, "ж", "\263")
					$Data = StringReplace($Data, "з", "\264")
					$Data = StringReplace($Data, "и", "\270")
					$Data = StringReplace($Data, "й", "\271")
					$Data = StringReplace($Data, "к", "K")
					$Data = StringReplace($Data, "л", "\272")
					$Data = StringReplace($Data, "м", "M")
					$Data = StringReplace($Data, "н", "H")
					$Data = StringReplace($Data, "о", "O")
					$Data = StringReplace($Data, "п", "\273")
					$Data = StringReplace($Data, "р", "P")
					$Data = StringReplace($Data, "с", "C")
					$Data = StringReplace($Data, "т", "T")
					$Data = StringReplace($Data, "у", "\274")
					$Data = StringReplace($Data, "ф", "\277")
					$Data = StringReplace($Data, "х", "X")
					$Data = StringReplace($Data, "ц", "\300")
					$Data = StringReplace($Data, "ч", "\301")
					$Data = StringReplace($Data, "ш", "\302")
					$Data = StringReplace($Data, "щ", "\303")
					$Data = StringReplace($Data, "ъ", "\304")
					$Data = StringReplace($Data, "ы", "\305")
					$Data = StringReplace($Data, "ь", "b")
					$Data = StringReplace($Data, "э", "\310")
					$Data = StringReplace($Data, "ю", "\311")
					$Data = StringReplace($Data, "я", "\312")
					$Data = StringReplace($Data, "ї", "\330")

					GUICtrlSetData($Edit1, $Data)
					GUICtrlSetData($Label1, "Макс. 40 символов")
            EndSwitch

	  Case $Button1
			ClipPut (  GUICtrlRead($Edit1) )
			GUICtrlSetData($Label1, "Скопировано!")
	EndSwitch
    Return $GUI_RUNDEFMSG
EndFunc