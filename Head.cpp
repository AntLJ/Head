// Head.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "Head.h"

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#define ID_LIST 1
#define ID_TEXT 2
#define MAXREAD 8192
#define DIRATTR (DDL_READWRITE | DDL_READONLY | DDL_HIDDEN | DDL_SYSTEM | DDL_DIRECTORY | DDL_ARCHIVE | DDL_DRIVES)
#define DTFLAGS (DT_WORDBREAK | DT_EXPANDTABS | DT_NOCLIP |DT_NOPREFIX)
LRESULT CALLBACK ListProc(HWND, UINT, WPARAM, LPARAM);
WNDPROC OldList;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_HEAD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HEAD));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HEAD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_HEAD);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bValidFile;
	static BYTE buffer[MAXREAD];
	static HWND hwndList, hwndText;
	static RECT rect;
	static TCHAR szFile[MAX_PATH + 1];
	HANDLE hFile;
	HDC hdc;
	int i, cxChar, cyChar;
	PAINTSTRUCT ps;
	TCHAR szBuffer[MAX_PATH + 1];

    switch (message)
    {
	case WM_CREATE:
		cxChar = LOWORD(GetDialogBaseUnits());
		cyChar = HIWORD(GetDialogBaseUnits());

		rect.left = 20 * cxChar;
		rect.top = 3 * cyChar;

		hwndList = CreateWindow(TEXT("listbox"), NULL,
			WS_CHILDWINDOW | WS_VISIBLE | LBS_STANDARD,
			cxChar, cyChar * 3,
			cxChar * 13 + GetSystemMetrics(SM_CXVSCROLL),
			cyChar * 10,
			hwnd, (HMENU)ID_LIST,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL);
		GetCurrentDirectory(MAX_PATH + 1, szBuffer);

		hwndText = CreateWindow(TEXT("static"), szBuffer,
			WS_CHILDWINDOW | WS_VISIBLE | SS_LEFT,
			cxChar, cyChar, cxChar * MAX_PATH, cyChar,
			hwnd, (HMENU)ID_TEXT,
			(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE),
			NULL);

		OldList = (WNDPROC)SetWindowLong(hwndList, GWL_WNDPROC, (LPARAM)ListProc);

		SendMessage(hwndList, LB_DIR, DIRATTR, (LPARAM)TEXT("*.*"));
		return 0;

	case WM_SIZE:
		rect.right = LOWORD(lParam);
		rect.bottom = HIWORD(lParam);
		return 0;
	case WM_SETFOCUS:
		SetFocus(hwndList);
		return 0;
    case WM_COMMAND:
		if (LOWORD(wParam) == ID_LIST && HIWORD(wParam) == LBN_DBLCLK)
		{
			if (LB_ERR == (i = SendMessage(hwndList, LB_GETCURSEL, 0, 0)))
				break;

			SendMessage(hwndList, LB_GETTEXT, i, (LPARAM)szBuffer);
			if (INVALID_HANDLE_VALUE != (hFile = CreateFile(szBuffer,
				GENERIC_READ, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, 0, NULL)))
			{
				CloseHandle(hFile);
				bValidFile = TRUE;
				lstrcpy(szFile, szBuffer);
				GetCurrentDirectory(MAX_PATH + 1, szBuffer);
				if (szBuffer[lstrlen(szBuffer) - 1] != '\\')
					lstrcat(szBuffer, TEXT("\\"));
				SetWindowText(hwndText, lstrcat(szBuffer, szFile));
			}
			else
			{
				bValidFile = FALSE;
				szBuffer[lstrlen(szBuffer) - 1] = '\0';
				// If setting the directory doesn't work, maybe it's
				// a drive change, so try that.
				if (!SetCurrentDirectory(szBuffer + 1))
				{
					szBuffer[3] = ':';
					szBuffer[4] = '\0';
					SetCurrentDirectory(szBuffer + 2);
				}
				// Get the new directory name and fill the list box.
				GetCurrentDirectory(MAX_PATH + 1, szBuffer);
				SetWindowText(hwndText, szBuffer);
				SendMessage(hwndList, LB_RESETCONTENT, 0, 0);
				SendMessage(hwndList, LB_DIR, DIRATTR, (LPARAM)TEXT("*.*"));
			}
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;
    case WM_PAINT:
		if (!bValidFile)
			break;
		if (INVALID_HANDLE_VALUE == (hFile = CreateFile(szFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)))
		{
			bValidFile = FALSE;
			break;
		}
		ReadFile(hFile, buffer, MAXREAD, (LPDWORD)&i, NULL);
		CloseHandle(hFile);
		// i now equals the number of bytes in buffer.
		// Commence getting a device context for displaying text.
		hdc = BeginPaint(hwnd, &ps);
		SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));
		SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
		SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
		// Assume the file is ASCII
		DrawTextA(hdc, (LPCSTR)buffer, i, &rect, DTFLAGS);
		EndPaint(hwnd, &ps);
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
		return 0;
    }
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK ListProc(HWND hwnd, UINT message,
	WPARAM wParam, LPARAM lParam)
{
	if (message == WM_KEYDOWN && wParam == VK_RETURN)
		SendMessage(GetParent(hwnd), WM_COMMAND, MAKELONG(1, LBN_DBLCLK), (LPARAM)hwnd);
	return CallWindowProc(OldList, hwnd, message, wParam, lParam);
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
