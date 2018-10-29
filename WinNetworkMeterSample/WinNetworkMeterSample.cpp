// WinNetworkMeterSample.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"
#include "WinNetworkMeterSample.h"
#include "worker.h"

using namespace Gdiplus;

#define MAX_LOADSTRING 100

#define PI 3.14159265f

// グローバル変数:
DWORD threadId;    // スレッド ID 

int screenWidth = 800;
int screenHeight = 800;

HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                DrawAll(HDC hdc, PAINTSTRUCT ps, CWorker* pWorker, Graphics* offScreen, Bitmap* offScreenBitmap);
void DrawMeter(Graphics& g, Gdiplus::RectF& rect, float percent, SolidBrush& mainBrush, const WCHAR* str);


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
    LoadStringW(hInstance, IDC_WINNETWORKMETERSAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINNETWORKMETERSAMPLE));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINNETWORKMETERSAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINNETWORKMETERSAMPLE);
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
      CW_USEDEFAULT, 0, 800, 800, nullptr, nullptr, hInstance, nullptr);

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GdiplusStartupInput gdiSI;
	static ULONG_PTR gdiToken = NULL;
	static CWorker* pMyWorker = NULL;

	static Bitmap* offScreenBitmap = NULL;
	static Graphics* offScreen = NULL;


    switch (message)
    {
	case WM_CREATE:
	{
		// 初期化
		GdiplusStartup(&gdiToken, &gdiSI, NULL);

		// OffScreen
		offScreenBitmap = new Bitmap(screenWidth, screenHeight);
		offScreen = new Graphics(offScreenBitmap);

		// スレッド準備
		pMyWorker = new CWorker();
		pMyWorker->SetParams(hWnd);
		DWORD threadId;

		// スレッドの作成 
		HANDLE hThread = CreateThread(NULL, 0,
			CWorker::ThreadFunc, (LPVOID)pMyWorker,
			CREATE_SUSPENDED, &threadId);

		// スレッドの起動 
		ResumeThread(hThread);
	}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			DrawAll(hdc, ps, pMyWorker, offScreen, offScreenBitmap);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		// 終了処理

		pMyWorker->Terminate();

		GdiplusShutdown(gdiToken);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
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

void DrawAll(HDC hdc, PAINTSTRUCT ps, CWorker* pWorker, Graphics* offScreen, Bitmap* offScreenBitmap)
{
	if (pWorker == NULL) {
		return;
	}

	Graphics& g = *offScreen;

	static int i = 0;
	i++;

	//--------------------------------------------------
	// dump
	//--------------------------------------------------
	Font fontTahoma(L"Tahoma", 16);
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	SolidBrush mainBrush(Color(255, 192, 192, 192));
	SolidBrush backgroundBrush(Color(255, 10, 10, 10));
	Gdiplus::RectF rect = Gdiplus::RectF(0.0f, 0.0f, (float)screenWidth, (float)screenHeight);

	g.FillRectangle(&backgroundBrush, rect);

	CString str;
	str.Format(L"Up=%d, Down=%d", pWorker->trafficOut, pWorker->trafficIn);
	g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);

	if (pWorker->lastTrafficTick > 0) {
		DWORD duration = pWorker->trafficTick - pWorker->lastTrafficTick;

		// duration が ms なので *1000 してから除算
		DWORD inb = (pWorker->trafficIn - pWorker->lastTrafficIn) * 1000 / duration;
		DWORD outb = (pWorker->trafficOut - pWorker->lastTrafficOut) * 1000 / duration;

		str.Format(L"Up=%d[b/s], Down=%d[b/s]", outb, inb);
		rect.Offset(0, 30);
		g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);

		//str.Format(L"i=%d", i);
		//rect.Offset(0, 30);
		//g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);


		//--------------------------------------------------
		// タコメーター描画
		//--------------------------------------------------

		DWORD MB = 1000 * 1000;
		DWORD maxTrafficBytes = 100 * MB;
		float percent = 0.0f;
		int height = 160;

		// Up(byte単位)
		rect = Gdiplus::RectF(100.0f, 150.0f, 200.0f, 200.0f);
		percent = outb == 0 ? 0.0f : (log10f((float)outb) / log10f((float)maxTrafficBytes))*100.0f;
		str.Format(L"Up=%d[b/s], %.0f%%", outb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);

		// Down(byte単位)
		rect = Gdiplus::RectF(350.0f, 150.0f, 200.0f, 200.0f);
		percent = inb == 0 ? 0.0f : (log10f((float)inb) / log10f((float)maxTrafficBytes))*100.0f;
		str.Format(L"Down=%d[b/s], %.0f%%", outb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);

		// kB単位
		maxTrafficBytes /= 1000;
		inb /= 1000;
		outb /= 1000;

		// Up(kB単位)
		rect = Gdiplus::RectF(100.0f, 150.0f+ height *1, 200.0f, 200.0f);
		percent = outb == 0 ? 0.0f : (log10f((float)outb) / log10f((float)maxTrafficBytes))*100.0f;
		str.Format(L"Up=%d[kb/s], %.0f%%", outb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);

		// Down(kB単位)
		rect = Gdiplus::RectF(350.0f, 150.0f+ height *1, 200.0f, 200.0f);
		percent = inb == 0 ? 0.0f : (log10f((float)inb) / log10f((float)maxTrafficBytes))*100.0f;
		str.Format(L"Down=%d[kb/s], %.0f%%", outb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);


		// sample
		//rect = Gdiplus::RectF(100.0f, 150.0f+ height *2, 200.0f, 200.0f);
		//percent = (i % 10) * 10.0f;	// [0, 100]
		//str.Format(L"Sample (%.0f%%)", percent);
		//DrawMeter(g, rect, percent, mainBrush, str);
	}


	//--------------------------------------------------
	// 実画面に転送
	//--------------------------------------------------
	Graphics onScreen(hdc);
	onScreen.DrawImage(offScreenBitmap, 0, 0);
}

void DrawMeter(Graphics& g, Gdiplus::RectF& rect, float percent, SolidBrush& mainBrush, const WCHAR* str)
{
	// ペン
	Pen p(Color(255, 192, 192, 192), 1);

	// アンチエイリアス
//	g.SetSmoothingMode(SmoothingModeNone);
	g.SetSmoothingMode(SmoothingModeHighQuality);

	// 外枠
	Gdiplus::PointF center(rect.X + rect.Width / 2, rect.Y + rect.Height / 2);
	g.DrawArc(&p, rect, 0, -180);
	g.DrawLine(&p, rect.X, center.Y, rect.GetRight(), center.Y);

	p.SetWidth(5);

	// 線を引く
	float length = rect.Width / 2 * 0.9f;
	float rad = PI * percent / 100.0f;
	float x = center.X - length * cosf(rad);
	float y = center.Y - length * sinf(rad);
	g.DrawLine(&p, center.X, center.Y, x, y);

	// ラベル
	Font fontTahoma(L"Tahoma", 11);
	StringFormat format;
	format.SetAlignment(StringAlignmentNear);
	rect.Offset(0, -20);
	g.DrawString(str, wcslen(str), &fontTahoma, rect, &format, &mainBrush);
}