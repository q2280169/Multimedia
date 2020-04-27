// Multimedia.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "Multimedia.h"
#include "audio.h"
#include "image.h"
#include "vedio.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
Audio* audio;
Image* image;
Vedio* vedio;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void CALLBACK CallIMGWater(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);
void CALLBACK CallBoFang(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);
void CALLBACK CallText(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);
void CALLBACK CallDanRuDanChu(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);
void CALLBACK CalLPinJie(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);
void CALLBACK CallVEDWater(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime);
void killTimers(HWND hwnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MULTIMEDIA, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MULTIMEDIA));
    MSG msg;

    // 主消息循环:
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
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MULTIMEDIA));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MULTIMEDIA);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   audio = new Audio();
   audio->CreateMediaSession(hWnd, L"C:\\Users\\GaoWei\\Music\\1.wav");
   image = new Image(hInstance);
   vedio = new Vedio();
   //disturb();
   //SetTimer(hWnd, ID_TIMER, 25, NULL);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case ID_AU_PLAY:
                audio->play();
                break;
            case ID_AU_PAUSE:
                audio->pause();
                break;
            case ID_AU_STOP:
                audio->stop();
                break;
            case ID_TRANS_CODE:
                audio->TransCode();
                break;
            case ID_IMG_UP:
            {
                killTimers(hWnd);
                hdc = GetDC(hWnd);
                image->UpDisplay(hdc);
                ReleaseDC(hWnd, hdc);
                break;
            }
            case ID_IMG_DOWN:
            {
                killTimers(hWnd);
                hdc = GetDC(hWnd);
                image->DownDisplay(hdc);
                ReleaseDC(hWnd, hdc);
                break;
            }
            case ID_IMG_GRAY:
            {
                killTimers(hWnd);
                hdc = GetDC(hWnd);
                image->GrayDisplay(hdc);
                ReleaseDC(hWnd, hdc);
                break;
            }
            case IM_IMG_FUDIAO:
            {
                killTimers(hWnd);
                hdc = GetDC(hWnd);
                image->FudiaoDisPlay(hdc);
                ReleaseDC(hWnd, hdc);
                break;
            }
            case ID_IMG_PLUS:
            {
                killTimers(hWnd);
                hdc = GetDC(hWnd);
                image->PlusDisplay(hdc);
                ReleaseDC(hWnd, hdc);
                break;
            }
            case ID_IMG_WATER:
            {
                killTimers(hWnd);
                image->disturb();
                SetTimer(hWnd, ID_TIMER, 25, CallIMGWater);
                break;
            }
            case ID_VED_PLAY:
            {
                killTimers(hWnd);
                SetTimer(hWnd, ID_TIMER_BOFANG, 25, CallBoFang);
                break;
            }
            case ID_VED_TEXT:
            {
                killTimers(hWnd);
                SetTimer(hWnd, ID_TIMER_TEXT, 25, CallText);
                break;
            }
            case ID_VED_DANRUDANCHU:
            {
                killTimers(hWnd);
                SetTimer(hWnd, ID_TIMER_DANRUDANCHU, 25, CallDanRuDanChu);
                break;
            }
            case ID_VED_PINJIE:
            {
                killTimers(hWnd);
                SetTimer(hWnd, ID_TIMER_PINJIE, 25, CalLPinJie);
                break;
            }
            case ID_VED_WATER:
            {
                killTimers(hWnd);
                vedio->disturb();
                SetTimer(hWnd, ID_TIMER_WATER, 25, CallVEDWater);
                break;
            }
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_TIMER:
    {
        HDC hdc = GetDC(hWnd); 
        ReleaseDC(hWnd, hdc);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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

void CALLBACK CallIMGWater(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
    image->WaterDisplay(hWnd);
}

void CALLBACK CallBoFang(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
    vedio->bofang(hWnd);
}

void CALLBACK CallText(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
    vedio->zimu(hWnd);
}

void CALLBACK CallDanRuDanChu(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
    vedio->danrudanchu(hWnd);
}

void CALLBACK CalLPinJie(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
    vedio->pinjie(hWnd);
}

void CALLBACK CallVEDWater(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
    vedio->water(hWnd);
}

void killTimers(HWND hwnd)
{
    KillTimer(hwnd, ID_TIMER);
    KillTimer(hwnd ,ID_TIMER_BOFANG);
    KillTimer(hwnd, ID_TIMER_TEXT);
    KillTimer(hwnd, ID_TIMER_DANRUDANCHU);
    KillTimer(hwnd, ID_TIMER_PINJIE);
    KillTimer(hwnd, ID_TIMER_WATER);
}
