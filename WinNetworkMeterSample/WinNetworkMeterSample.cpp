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
void                DrawAll(HWND hWnd, HDC hdc, PAINTSTRUCT ps, CWorker* pWorker, Graphics* offScreen, Bitmap* offScreenBitmap);
void                DrawMeter(Graphics& g, Gdiplus::RectF& rect, float percent, SolidBrush& mainBrush, const WCHAR* str);


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
	wcex.lpszMenuName   = NULL;// MAKEINTRESOURCEW(IDC_WINNETWORKMETERSAMPLE);
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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, 
//		WS_OVERLAPPEDWINDOW,
		WS_POPUP,
		CW_USEDEFAULT, 0, 700, 700, nullptr, nullptr, hInstance, nullptr);

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

			DrawAll(hWnd, hdc, ps, pMyWorker, offScreen, offScreenBitmap);

            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
		// 終了処理

		pMyWorker->Terminate();

		GdiplusShutdown(gdiToken);

        PostQuitMessage(0);
        break;

	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE:
		case VK_F12:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0L;
		}
		break;

	// ウィンドウのドラッグ
	case WM_LBUTTONDOWN:
		ReleaseCapture();
		SendMessage(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		return 0;

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

void DrawAll(HWND hWnd, HDC hdc, PAINTSTRUCT ps, CWorker* pWorker, Graphics* offScreen, Bitmap* offScreenBitmap)
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

	if (pWorker->traffics.size() >= 2) {
		CString str;

		const Traffic& t = pWorker->traffics[pWorker->traffics.size() - 1];	// 一番新しいもの
		const Traffic& t0 = pWorker->traffics[0];	// 一番古いもの

		// デバッグ表示、開始 Y 座標
		rect.Offset(0, 390);

		str.Format(L"Up=%lld(%lld), Down=%lld(%lld)",
			t.out, t.out - t0.out,
			t.in, t.in - t0.in);
		g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);

		DWORD duration = t.tick - t0.tick;

		// duration が ms なので *1000 してから除算
		float inb = (t.in - t0.in) * 1000.0f / duration;
		float outb = (t.out - t0.out) * 1000.0f / duration;

		str.Format(L"Up=%.0f[b/s], Down=%.0f[b/s], %ldms", outb, inb, duration);
		rect.Offset(0, 30);
		g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);

		str.Format(L"Up=%.1f[kb/s], Down=%.1f[kb/s]", outb / 1024, inb / 1024);
		rect.Offset(0, 30);
		g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);

		RECT rectWindow;
		GetClientRect(hWnd, &rectWindow);
		str.Format(L"i=%d, n=%d size=%dx%d", i, pWorker->traffics.size(), rectWindow.right, rectWindow.bottom);
		rect.Offset(0, 30);
		g.DrawString(str, str.GetLength(), &fontTahoma, rect, &format, &mainBrush);



		//--------------------------------------------------
		// タコメーター描画
		//--------------------------------------------------

		DWORD MB = 1024 * 1024;
		DWORD maxTrafficBytes = 100 * MB;
		float percent = 0.0f;
		int height = 200;

		// Up(byte単位)
		float xbase = 10.0f;
		float ybase = 30.0f;
		rect = Gdiplus::RectF(xbase, ybase, 200.0f, 200.0f);
		percent = outb == 0 ? 0.0f : (log10f((float)outb) / log10f((float)maxTrafficBytes))*100.0f;
		str.Format(L"▲ %.0f[b/s], %.0f%%", outb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);

		// Down(byte単位)
		rect = Gdiplus::RectF(xbase + 250.0f, ybase, 200.0f, 200.0f);
		percent = inb == 0 ? 0.0f : (log10f((float)inb) / log10f((float)maxTrafficBytes))*100.0f;
		str.Format(L"▼ %.0f[b/s], %.0f%%", inb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);

		// kB単位
		maxTrafficBytes /= 1024;
		inb /= 1024;
		outb /= 1024;

		// Up(kB単位)
		float y = ybase + height * 1;
		rect = Gdiplus::RectF(xbase, y, 200.0f, 200.0f);
		percent = outb == 0 ? 0.0f : (log10f((float)outb) / log10f((float)maxTrafficBytes))*100.0f;
		percent = percent < 0.0f ? 0.0f : percent;
		str.Format(L"▲ %.1f[kb/s], %.1f%%", outb, percent);
		DrawMeter(g, rect, percent, mainBrush, str);

		// Down(kB単位)
		rect = Gdiplus::RectF(xbase + 250.0f, y, 200.0f, 200.0f);
		percent = inb == 0 ? 0.0f : (log10f((float)inb) / log10f((float)maxTrafficBytes))*100.0f;
		percent = percent < 0.0f ? 0.0f : percent;
		str.Format(L"▼ %.1f[kb/s], %.1f%%", inb, percent);
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

	// 左下、右下の角度
	float PMIN = -30;
	float PMAX = 180 - PMIN;

	// 外枠
	Gdiplus::PointF center(rect.X + rect.Width / 2, rect.Y + rect.Height / 2);
	g.DrawArc(&p, rect, -180+PMIN, PMAX-PMIN);
//	g.DrawLine(&p, rect.X, center.Y, rect.GetRight(), center.Y);

	float length0 = rect.Width / 2;

	// 真ん中から左下へ。
	float r1 = PI * PMIN / 180;
	g.DrawLine(&p, center.X, center.Y, center.X - length0 * cosf(r1), center.Y - length0 * sinf(r1));

	// 真ん中から右下へ。
	float r2 = PI * PMAX / 180;
	g.DrawLine(&p, center.X, center.Y, center.X - length0 * cosf(r2), center.Y - length0 * sinf(r2));

	p.SetWidth(5);

	// 線を引く
	float length = rect.Width / 2 * 0.9f;
//	float rad = PI * percent / 100.0f;
	float rad = PI * (percent / 100.0f * (PMAX - PMIN) + PMIN) / 180;
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