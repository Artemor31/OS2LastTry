#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <tchar.h>

#include "resource.h"
#include <string>
#include <sstream>
//#include "afxres.h"

#define IDC_TREE             2001
#define WM_ADDITEM           WM_USER + 1

HWND hWnd = NULL;
HACCEL hAccel = NULL;
HWND hDlg = NULL;
HWND hFindDlg = NULL;
HWND hwndTimer = NULL;
HMENU  hMenu = NULL;
HFONT hFont = NULL;

TCHAR szBuffer[100] = TEXT("");
BOOL fInsertAddItemToRoot = FALSE; // флаг - добавлять новые элементы в корень дерева

FINDREPLACE findDlg;
UINT uFindMsgString = 0;

POINT ptDrag;

HBRUSH brushes[3];
int brush_index = 0;

LPCWSTR scoreText;
int score = 0;

HWND edit1 = NULL;

int CUrrentColorOfTree = 0;   
COLORREF ColPref[5]
{
	0x0000FF00,
	0x000000FF,
	0x00FF0000,
	0x00FFFFFF,
	0x0000E4FF
};
LPCTSTR KeyVal;
HTREEITEM Syshitem;

LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL PreTranslateMessage(LPMSG lpMsg);
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
void OnDestroy(HWND hwnd);
void OnSize(HWND hwnd, UINT state, int cx, int cy);
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
void OnAddItem(HWND hwnd);
void OnFindMsgString(HWND hwnd, LPFINDREPLACE lpFindReplace);
void OnNotify(HWND hwnd, LPNMHDR lpnmhdr);
void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags);
void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags);
void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags);
void OnSysKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags);

INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL Dialog_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam);
void Dialog_OnClose(HWND hwnd);
void Dialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
HTREEITEM TreeView_FindItem(HWND hwnd, HTREEITEM hParent, LPCTSTR lpText, BOOL fIgnoreCase);
void TreeView_DeleteAllCheckedItems(HWND hwnd, HTREEITEM hParent);
std::string convertInt(int number);
std::wstring s2ws(const std::string& s);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpszCmdLine, int nCmdShow)
{
    brushes[0] = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    brushes[1] = (HBRUSH)GetStockObject(GRAY_BRUSH);
    brushes[2] = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
    MSG  Massage;
    BOOL IsError;

    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = MyWindowProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = brushes[brush_index];
    wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
    wcex.lpszClassName = TEXT("MyWindowClass");
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


    if (0 == RegisterClassEx(&wcex))
    {
        return -1;
    }
    LoadLibrary(TEXT("ComCtl32.dll"));
    hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

    hWnd = CreateWindowEx(0, TEXT("MyWindowClass"), TEXT("SampleWin32"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

    if (NULL == hWnd)
    {
        return -1;
    }
    ShowWindow(hWnd, nCmdShow);

    while (true)
    {
        if (PeekMessage(&Massage, NULL, 0, 0, PM_NOREMOVE) == TRUE)
        {
            IsError = GetMessage(&Massage, NULL, 0, 0);
            if (IsError == -1 || IsError == FALSE)
            {
                return FALSE;
            }
            else
            {
                TranslateMessage(&Massage);
                DispatchMessage(&Massage);
            }
        }
    }
    return static_cast<int>(Massage.wParam);
}
LRESULT CALLBACK MyWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndCtl = GetDlgItem(hWnd, IDC_TREE);
    hMenu = GetMenu(hWnd);
    switch (uMsg)
    {
        HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
        HANDLE_MSG(hWnd, WM_SIZE, OnSize);
        HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
        HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, OnLButtonDown);
        HANDLE_MSG(hWnd, WM_LBUTTONUP, OnLButtonUp);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE, OnMouseMove);
        HANDLE_MSG(hWnd, WM_SYSKEYDOWN, OnSysKey);
        HANDLE_MSG(hWnd, WM_KEYDOWN, OnSysKey);

    case WM_SYSCHAR:

		KeyVal = (LPCTSTR)(char)wParam;
		Syshitem = TreeView_FindItem(hwndCtl, NULL, KeyVal, true);

        if (Syshitem != FALSE)
        {
            TreeView_SetItemState(hwndCtl, Syshitem, 0, TVIS_SELECTED);
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(ERROR_SUCCESS);
        return 0;
    case WM_ADDITEM:
        OnAddItem(hWnd);
        return 0;
    case WM_NOTIFY:
        OnNotify(hWnd, (LPNMHDR)lParam);
        return 0;
    case WM_PAINT:
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        FillRect(ps.hdc, &ps.rcPaint, brushes[brush_index]);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_TIMER:
    {
        score++;
        std::wstring tmp = s2ws(convertInt(score));
        scoreText = tmp.c_str();        
        Edit_SetText(edit1, scoreText);


       /* if (brush_index == 2) brush_index = 0;
        else brush_index++;
        InvalidateRect(hWnd, NULL, FALSE);*/
    }
    return 0;
    }

    if (uFindMsgString == uMsg)
    {
        OnFindMsgString(hWnd, (LPFINDREPLACE)lParam);
        return 0;
    }
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
BOOL PreTranslateMessage(LPMSG lpMsg)
{
    BOOL bRet = TRUE;

    if (!TranslateAccelerator(hWnd, hAccel, lpMsg))
    {
        bRet = IsDialogMessage(hDlg, lpMsg);

        if (FALSE == bRet)
            bRet = IsDialogMessage(hFindDlg, lpMsg);
    }
    return bRet;
}
BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)
{
    // создаём дерево просмотра 
    CreateWindowEx(0, TEXT("SysTreeView32"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | TVS_LINESATROOT | TVS_HASLINES | TVS_HASBUTTONS | TVS_SHOWSELALWAYS | TVS_CHECKBOXES,
        10, 30, 250, 410, hwnd, (HMENU)IDC_TREE, lpCreateStruct->hInstance, NULL);

    // создаём кнопку "Добавить запись"
    CreateWindowEx(0, TEXT("Button"), TEXT("Добавить запись"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 10, 200, 40, hwnd, (HMENU)ID_NEW_RECORD, lpCreateStruct->hInstance, NULL);

    // создаём кнопку "Добавить неск. записей"
    CreateWindowEx(0, TEXT("Button"), TEXT("Добавить неск. записей"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 55, 200, 40, hwnd, (HMENU)ID_NEW_RECORD2, lpCreateStruct->hInstance, NULL);

    // создаём кнопку "Удалить запись"
    CreateWindowEx(0, TEXT("Button"), TEXT("Удалить отмеч. записи"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 100, 200, 40, hwnd, (HMENU)ID_DEL_RECORD, lpCreateStruct->hInstance, NULL);

    // создаём кнопку "Найти запись"
    CreateWindowEx(0, TEXT("Button"), TEXT("Найти запись"),
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 370, 145, 200, 40, hwnd, (HMENU)ID_FIND_RECORD, lpCreateStruct->hInstance, NULL);

    // создаём гиперссылку
    CreateWindowEx(0, TEXT("SysLink"), TEXT("<a>Изменить шрифт</a>"),
        WS_CHILD | WS_VISIBLE | LWS_TRANSPARENT, 10, 440, 250, 30, hwnd, (HMENU)ID_FORMAT_FONT, lpCreateStruct->hInstance, NULL);

    // создаём дату
    CreateWindowEx(0, DATETIMEPICK_CLASS, TEXT("DateTime"),
        WS_BORDER | WS_CHILD | WS_VISIBLE | DTS_SHOWNONE, 370, 260, 220, 20, hwnd, NULL, lpCreateStruct->hInstance, NULL);

    // Создание радиокнопок
    CreateWindowEx(0, TEXT("Button"), TEXT("Показать кнопки"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        370, 200, 200, 20, hwnd, (HMENU)ID_YES, lpCreateStruct->hInstance, NULL);
    CreateWindowEx(0, TEXT("Button"), TEXT("Скрыть кнопки"),
        WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        370, 230, 200, 20, hwnd, (HMENU)ID_NO, lpCreateStruct->hInstance, NULL);

    // EditBox
    edit1 = CreateWindowEx(0, TEXT("Edit"), TEXT("0.0"),
        WS_CHILD | WS_TABSTOP | WS_VISIBLE | WS_BORDER,
        580, 10, 150, 20, hwnd, NULL, lpCreateStruct->hInstance, NULL);

    return TRUE;
}
void OnSize(HWND hwnd, UINT state, int cx, int cy)
{
    if (state != SIZE_MINIMIZED)
    {
        HWND hwndCtl = GetDlgItem(hwnd, IDC_TREE);
        MoveWindow(hwndCtl, 10, 10, 350, cy - 50, TRUE);
        hwndCtl = GetDlgItem(hwnd, ID_FORMAT_FONT);
        MoveWindow(hwndCtl, 10, cy - 30, 350, 30, TRUE);
    }
}
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    HINSTANCE hInstance = GetWindowInstance(hwnd);
    switch (id)
    {
    case ID_NO:
        hwndCtl = GetDlgItem(hwnd, ID_NEW_RECORD);
        ShowWindow(hwndCtl, SW_HIDE);
        hwndCtl = GetDlgItem(hwnd, ID_NEW_RECORD2);
        ShowWindow(hwndCtl, SW_HIDE);
        hwndCtl = GetDlgItem(hwnd, ID_DEL_RECORD);
        ShowWindow(hwndCtl, SW_HIDE);
        hwndCtl = GetDlgItem(hwnd, ID_FIND_RECORD);
        ShowWindow(hwndCtl, SW_HIDE);
        CheckMenuItem(hMenu, ID_NO, MF_BYCOMMAND | MF_CHECKED);
        CheckMenuItem(hMenu, ID_YES, MF_BYCOMMAND | MF_UNCHECKED);
        DrawMenuBar(hwnd);
        break;

    case ID_YES:
	{
		hwndCtl = GetDlgItem(hwnd, ID_NEW_RECORD);
		ShowWindow(hwndCtl, SW_SHOW);
		hwndCtl = GetDlgItem(hwnd, ID_NEW_RECORD2);
		ShowWindow(hwndCtl, SW_SHOW);
		hwndCtl = GetDlgItem(hwnd, ID_DEL_RECORD);
		ShowWindow(hwndCtl, SW_SHOW);
		hwndCtl = GetDlgItem(hwnd, ID_FIND_RECORD);
		ShowWindow(hwndCtl, SW_SHOW);
		CheckMenuItem(hMenu, ID_YES, MF_BYCOMMAND | MF_CHECKED);
		CheckMenuItem(hMenu, ID_NO, MF_BYCOMMAND | MF_UNCHECKED);
		DrawMenuBar(hwnd);
	}
        break;

    case ID_NEW_RECORD:
    {
        int nDlgResult = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DialogProc);

        if (IDOK == nDlgResult)
        {
            SendMessage(hwnd, WM_ADDITEM, 0, 0);
        }
    }
    break;

    case ID_NEW_RECORD2:
	{
		if (IsWindow(hDlg) == FALSE)
		{
			hDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hwnd, DialogProc);
			ShowWindow(hDlg, SW_SHOW);
		}
	}
	break;

    case ID_DEL_RECORD:
    {
        int mbResult = MessageBox(hwnd, TEXT("Удалить отмеченные элементы?"), TEXT("SampleWin32"), MB_YESNO | MB_ICONQUESTION);

        if (mbResult == IDYES)
        {
            hwndCtl = GetDlgItem(hwnd, IDC_TREE);
            TreeView_DeleteAllCheckedItems(hwndCtl, NULL);
        }
    }
    break;

    case ID_FIND_RECORD:
	{
		if (0 == uFindMsgString)
		{
			//код сообщения FINDMSGSTRING
			uFindMsgString = RegisterWindowMessage(FINDMSGSTRING);
		}

		if (IsWindow(hFindDlg) == FALSE)
		{
			findDlg.lStructSize = sizeof(FINDREPLACE);
			findDlg.Flags = FR_HIDEUPDOWN | FR_HIDEWHOLEWORD;
			findDlg.hInstance = hInstance;
			findDlg.hwndOwner = hwnd;
			findDlg.lpstrFindWhat = szBuffer;
			findDlg.wFindWhatLen = _countof(szBuffer);

			hFindDlg = FindText(&findDlg);
		}
	}
	break;

    case ID_FORMAT_COLOR:
    {
        hwndCtl = GetDlgItem(hwnd, IDC_TREE);

        static COLORREF colors[16];

        CHOOSECOLOR cс = { sizeof(CHOOSECOLOR) };
        cс.hInstance = (HWND)hInstance;
        cс.hwndOwner = hwnd;
        cс.rgbResult = TreeView_GetTextColor(hwndCtl);
        cс.lpCustColors = colors;
        cс.Flags = CC_FULLOPEN | CC_RGBINIT;

        BOOL bRet = ChooseColor(&cс);

        if (FALSE != bRet)
        {
            TreeView_SetTextColor(hwndCtl, cс.rgbResult);
            UpdateWindow(hwndCtl);
        }
    }
    break;
    case ID_FORMAT_FONT:
    {
        HWND hwndCtl = GetDlgItem(hwnd, IDC_TREE);
        CHOOSEFONT cf = { sizeof(CHOOSEFONT) };
        cf.hInstance = hInstance;
        cf.hwndOwner = hwnd;
        LOGFONT lf;
        ZeroMemory(&lf, sizeof(lf));
        cf.lpLogFont = &lf;
        BOOL bRet = ChooseFont(&cf);
        if (FALSE != bRet)
        {
            HFONT hNewFont = CreateFontIndirect(cf.lpLogFont);
            if (NULL != hNewFont)
            {
                if (NULL != hFont) DeleteObject(hFont);
                hFont = hNewFont;
                SendDlgItemMessage(hwnd, IDC_TREE, WM_SETFONT, (WPARAM)hFont, (LPARAM)TRUE);
            }
        }
    }
    }
}
void OnAddItem(HWND hWnd)
{
    HWND hWndCtl = GetDlgItem(hWnd, IDC_TREE);
    TVINSERTSTRUCT tvis;

    if (FALSE != fInsertAddItemToRoot)
    {
        tvis.hParent = TVI_ROOT;
    }
    else
    {
        tvis.hParent = TreeView_GetNextSelected(hWndCtl, NULL);
    }

    tvis.hInsertAfter = TVI_LAST;
    tvis.item.mask = TVIF_TEXT;
    tvis.item.pszText = szBuffer;
    HTREEITEM hitem = TreeView_InsertItem(hWndCtl, &tvis);
    TreeView_SelectSetFirstVisible(hWndCtl, hitem);
}
void OnFindMsgString(HWND hwnd, LPFINDREPLACE lpFindReplace)
{
    if (lpFindReplace->Flags & FR_FINDNEXT)
    {
        HWND hwndCtl = GetDlgItem(hwnd, IDC_TREE);

        // "С учетом регистра"
        BOOL fIgnorCase = (lpFindReplace->Flags & FR_MATCHCASE) ? FALSE : TRUE;
        HTREEITEM hitem = TreeView_FindItem(hwndCtl, NULL, szBuffer, fIgnorCase);

        if (NULL != hitem)
        {
            TreeView_SetItemState(hwndCtl, TreeView_GetNextSelected(hwndCtl, NULL), 0, TVIS_SELECTED);
            TreeView_SetItemState(hwndCtl, hitem, TVIS_SELECTED, TVIS_SELECTED);
            TreeView_SelectSetFirstVisible(hwndCtl, hitem);

            SetFocus(hwndCtl);
        }
        else
        {
            MessageBox(hwnd, TEXT("Элемент не найден"), TEXT("SampleWin32"), MB_OK | MB_ICONINFORMATION);
        }
    }
}
void OnNotify(HWND hwnd, LPNMHDR lpnmhdr)
{
    switch (lpnmhdr->code)
    {
    case NM_CLICK:
        if (lpnmhdr->idFrom == ID_FORMAT_FONT)
        {
            OnCommand(hwnd, lpnmhdr->idFrom, lpnmhdr->hwndFrom, 0);
        }
        break;
    case TVN_ITEMCHANGED:
        if (lpnmhdr->idFrom == IDC_TREE)
        {
            NMTVITEMCHANGE* pnm = (NMTVITEMCHANGE*)lpnmhdr;
            UINT uStateOld = (pnm->uStateOld & TVIS_STATEIMAGEMASK);
            UINT uStateNew = (pnm->uStateNew & TVIS_STATEIMAGEMASK);
            if (uStateOld != uStateNew)
            {
                HTREEITEM hitem = TreeView_GetChild(lpnmhdr->hwndFrom, pnm->hItem);
                while (NULL != hitem)
                {
                    TreeView_SetItemState(lpnmhdr->hwndFrom, hitem, uStateNew, TVIS_STATEIMAGEMASK);
                    hitem = TreeView_GetNextSibling(lpnmhdr->hwndFrom, hitem);
                }
            }
        }
        break;
    }
}
void OnLButtonDown(HWND hwnd, BOOL fDoubleClick, int x, int y, UINT keyFlags)
{
    SetFocus(hwnd);

    if (fDoubleClick != FALSE)
    {
        if (IsMaximized(hwnd)) ShowWindow(hwnd, SW_RESTORE);
        else  ShowWindow(hwnd, SW_MAXIMIZE);
    }
    else
    {
        SetCapture(hwnd);
        ptDrag.x = x;
        ptDrag.y = y;
        score = 0;
        SetTimer(hwnd, 1, 100, NULL);
    }
}
void OnLButtonUp(HWND hwnd, int x, int y, UINT keyFlags)
{
    ReleaseCapture();
    KillTimer(hwnd, 1);
}
void OnMouseMove(HWND hwnd, int x, int y, UINT keyFlags)
{
    if (GetCapture() == hwnd)
    {
        RECT rect;
        GetWindowRect(hwnd, &rect);

        rect.left += x - ptDrag.x;
        rect.top += y - ptDrag.y;
        SetWindowPos(hwnd, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE);
    }
}
void OnSysKey(HWND hwnd, UINT vk, BOOL fDown, int cRepeat, UINT flags)
{
    RECT rect;
    GetWindowRect(hwnd, &rect);

    HWND hwndCtl = GetDlgItem(hwnd, IDC_TREE);

    switch (vk)
    {
    case VK_LEFT:
        SetWindowPos(hwnd, NULL, rect.left - 5, rect.top, 0, 0, SWP_NOSIZE);
        if (CUrrentColorOfTree > 0)
            CUrrentColorOfTree--;
        else
            CUrrentColorOfTree = 4;
        TreeView_SetBkColor(hwndCtl, ColPref[CUrrentColorOfTree]);
        break;

    case VK_RIGHT:
        SetWindowPos(hwnd, NULL, rect.left + 5, rect.top, 0, 0, SWP_NOSIZE);
        if (CUrrentColorOfTree < 5)
            CUrrentColorOfTree++;
        else
            CUrrentColorOfTree = 0;
        TreeView_SetBkColor(hwndCtl, ColPref[CUrrentColorOfTree]);
        break;

    case VK_UP:
        SetWindowPos(hwnd, NULL, rect.left, rect.top - 5, 0, 0, SWP_NOSIZE);
        break;

    case VK_DOWN:
        SetWindowPos(hwnd, NULL, rect.left, rect.top + 5, 0, 0, SWP_NOSIZE);
        break;
    }
}
INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        BOOL bRet = HANDLE_WM_INITDIALOG(hwndDlg, wParam, lParam, Dialog_OnInitDialog);
        return SetDlgMsgResult(hwndDlg, uMsg, bRet);
    }
    case WM_CLOSE:
        HANDLE_WM_CLOSE(hwndDlg, wParam, lParam, Dialog_OnClose);
        return TRUE;
    case WM_COMMAND:
        HANDLE_WM_COMMAND(hwndDlg, wParam, lParam, Dialog_OnCommand);
        return TRUE;
    }

    return FALSE;
}
BOOL Dialog_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT1);
    Edit_LimitText(hwndEdit, _countof(szBuffer) - 1);
    Edit_SetCueBannerText(hwndEdit, L"Название новой записи");
    CheckDlgButton(hwnd, IDC_CHECK_ROOT, BST_CHECKED);
    return TRUE;
} 
void Dialog_OnClose(HWND hwnd)
{
    if (hwnd == hDlg)
        DestroyWindow(hwnd);
    else
        EndDialog(hwnd, IDCLOSE);
} 
void Dialog_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK: 
    {
        fInsertAddItemToRoot = (IsDlgButtonChecked(hwnd, IDC_CHECK_ROOT) == BST_CHECKED) ? TRUE : FALSE;
        int cch = GetDlgItemText(hwnd, IDC_EDIT1, szBuffer, _countof(szBuffer));

        if (0 == cch) 
        {
            HWND hwndEdit = GetDlgItem(hwnd, IDC_EDIT1);

            EDITBALLOONTIP ebt = { sizeof(EDITBALLOONTIP) };

            ebt.pszTitle = L"SampleWin32";
            ebt.pszText = L"Укажите название новой записи";
            ebt.ttiIcon = TTI_WARNING;

            Edit_ShowBalloonTip(hwndEdit, &ebt);
        }
        else if (hwnd == hDlg)
        {
            SetDlgItemText(hwnd, IDC_EDIT1, NULL);
            SendMessage(GetParent(hwnd), WM_ADDITEM, 0, 0);
        } 
        else
            EndDialog(hwnd, IDOK);
    } 
    break;

    case IDCANCEL: 
        if (hwnd == hDlg)
            DestroyWindow(hwnd);
        else
            EndDialog(hwnd, IDCANCEL);
        break;
    } 
} 
HTREEITEM TreeView_FindItem(HWND hwnd, HTREEITEM hParent, LPCTSTR lpText, BOOL fIgnoreCase)
{
    TCHAR szItemText[100];

    TVITEM item;
    item.mask = TVIF_TEXT;

    item.cchTextMax = _countof(szItemText);
    item.pszText = szItemText;

    item.hItem = TreeView_GetChild(hwnd, hParent);

    while (NULL != item.hItem)
    {
        BOOL bRet = TreeView_GetItem(hwnd, &item);

        if (FALSE != bRet)
        {
            DWORD dwCmpFlags = (FALSE != fIgnoreCase) ? NORM_IGNORECASE : 0;

            HRESULT hr = CompareString(LOCALE_USER_DEFAULT, dwCmpFlags, lpText, -1, item.pszText, -1);

            if (CSTR_EQUAL == hr)
            {
                return item.hItem;
            }
        }

        HTREEITEM hitem = TreeView_FindItem(hwnd, item.hItem, lpText, fIgnoreCase);

        if (NULL != hitem)
        {
            return hitem;
        }

        item.hItem = TreeView_GetNextSibling(hwnd, item.hItem);
    }

    return NULL;
}
void TreeView_DeleteAllCheckedItems(HWND hwnd, HTREEITEM hParent)
{
    HTREEITEM hitem = TreeView_GetChild(hwnd, hParent);

    while (NULL != hitem)
    {
        HTREEITEM hti = TreeView_GetNextSibling(hwnd, hitem);

        if (TreeView_GetCheckState(hwnd, hitem))
        {
            TreeView_DeleteItem(hwnd, hitem);
        }
        else
        {
            TreeView_DeleteAllCheckedItems(hwnd, hitem);
        }
        hitem = hti;
    }
}

std:: string convertInt(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

std::wstring StrToWstr(const std::string& text) {
    return std::wstring(text.begin(), text.end());
}

std::wstring s2ws(const std::string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}