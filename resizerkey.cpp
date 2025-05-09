#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <algorithm>
#include "resource.h"

using namespace std;

typedef unsigned long ulong;
const int PADDING = 20;
const int STEP = 10;

bool inResizeMode = false;
HWND targetWindow = NULL;
HHOOK hHook = NULL;
HWND hwnd = NULL;
HWND hOverlay = NULL;
HFONT hOverlayFont = NULL;

void CreateOverlay();
void ShowOverlay(bool show);
LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void ResizeActiveWindow() {
    HWND win = GetForegroundWindow();
    if (!win) return;
    HMONITOR mon = MonitorFromWindow(win, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi = { sizeof(mi) };
    if (!GetMonitorInfo(mon, &mi)) return;
    RECT &wa = mi.rcWork;
    MoveWindow(win,
        wa.left + PADDING,
        wa.top + PADDING,
        (wa.right - wa.left) - 2*PADDING,
        (wa.bottom - wa.top) - 2*PADDING,
        TRUE);
}

LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hWnd, &ps);
        RECT rc;
        GetClientRect(hWnd, &rc);
        
        HBRUSH br = CreateSolidBrush(RGB(51,51,51));
        FillRect(dc, &rc, br);
        DeleteObject(br);

        HFONT oldFont = (HFONT)SelectObject(dc, hOverlayFont);
        const wchar_t* text = L"RESIZE MODE\nESC to exit";

        RECT calc = rc;
        DrawTextW(dc, text, -1, &calc, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);
        int textW = calc.right - calc.left;
        int textH = calc.bottom - calc.top;

        int x = (rc.right - rc.left - textW) / 2;
        int y = (rc.bottom - rc.top - textH) / 2;
        RECT draw = { x, y, x + textW, y + textH };

        SetTextColor(dc, RGB(255,255,255));
        SetBkMode(dc, TRANSPARENT);
        DrawTextW(dc, text, -1, &draw, DT_CENTER | DT_WORDBREAK);

        SelectObject(dc, oldFont);
        EndPaint(hWnd, &ps);
        return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void CreateOverlay() {
    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = OverlayWndProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = L"OverlayWindowClass";
    RegisterClassW(&wc);
 
    hOverlayFont = CreateFontW(
        24,0,0,0,FW_BOLD, FALSE,FALSE,FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI"
    );

    DWORD ex = WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT;
    DWORD st = WS_POPUP;
    hOverlay = CreateWindowExW(ex, wc.lpszClassName, NULL, st,
        0,0,250,60, NULL, NULL, wc.hInstance, NULL);
    SetLayeredWindowAttributes(hOverlay, 0, 255, LWA_ALPHA);
    HRGN r = CreateRoundRectRgn(0,0,250,60,20,20);
    SetWindowRgn(hOverlay, r, TRUE);
}

void ShowOverlay(bool show) {
    if (!hOverlay) return;
    if (show) {
        RECT wa;
        SystemParametersInfoW(SPI_GETWORKAREA,0,&wa,0);
        int w = 250, h = 60;
        int x = wa.left + (wa.right - wa.left - w) / 2;
        int y = wa.bottom - (wa.bottom - wa.top) / 4 - h / 2;
        SetWindowPos(hOverlay, HWND_TOPMOST, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
        if (targetWindow) SetForegroundWindow(targetWindow);
    } else {
        ShowWindow(hOverlay, SW_HIDE);
        if (targetWindow) SetForegroundWindow(targetWindow);
    }
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
        if (!inResizeMode) {
            if (p->vkCode == 'B' &&
                (GetKeyState(VK_LWIN)&0x8000 || GetKeyState(VK_RWIN)&0x8000) &&
                !(GetKeyState(VK_SHIFT)&0x8000)) {
                ResizeActiveWindow();
                return 1;
            }
            if (p->vkCode == 'B' &&
                (GetKeyState(VK_LWIN)&0x8000 || GetKeyState(VK_RWIN)&0x8000) &&
                (GetKeyState(VK_SHIFT)&0x8000)) {
                targetWindow = GetForegroundWindow();
                if (targetWindow) {
                    inResizeMode = true;
                    ShowOverlay(true);
                }
                return 1;
            }
        } else {
            if (p->vkCode == VK_ESCAPE) {
                inResizeMode = false;
                ShowOverlay(false);
                return 1;
            }
            if (targetWindow) {
                HMONITOR hMon = MonitorFromWindow(targetWindow, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi = { sizeof(mi) };
                GetMonitorInfo(hMon, &mi);
                RECT wa = mi.rcWork;

                RECT wr;
                GetWindowRect(targetWindow, &wr);
                long nl = wr.left;
                long nt = wr.top;
                long nw = wr.right - wr.left;
                long nh = wr.bottom - wr.top;

                if (p->vkCode == VK_LEFT)      nl -= STEP;
                else if (p->vkCode == VK_RIGHT) nl += STEP;
                else if (p->vkCode == VK_UP)    nt -= STEP;
                else if (p->vkCode == VK_DOWN)  nt += STEP;
                else {
                    BYTE ks[256];
                    WCHAR buf[5];
                    HKL layout = GetKeyboardLayout(0);
                    GetKeyboardState(ks);
                    int res = ToUnicodeEx(p->vkCode, p->scanCode, ks, buf, _countof(buf), 0, layout);
                    if (res > 0) {
                        WCHAR ch = buf[0];
                        if (ch == L'j' || ch == L'J') { nl -= STEP; nw += STEP; }
                        else if (ch == L'ñ' || ch == L'Ñ') nw += STEP;
                        else if (ch == L'k' || ch == L'K') { nt -= STEP; nh += STEP; }
                        else if (ch == L'l' || ch == L'L') nh += STEP;

                        else if (ch == L'u' || ch == L'U') { nl += STEP; nw -= STEP; }
                        else if (ch == L'i' || ch == L'I') { nt += STEP; nh -= STEP; }
                        else if (ch == L'o' || ch == L'O') nh -= STEP;
                        else if (ch == L'p' || ch == L'P') nw -= STEP;
                    }
                }

                nw = max(100L, nw);
                nh = max(100L, nh);
                if (nl < wa.left) nl = wa.left;
                if (nt < wa.top)  nt = wa.top;
                if (nl + nw > wa.right) nw = wa.right - nl;
                if (nt + nh > wa.bottom) nh = wa.bottom - nt;

                MoveWindow(targetWindow, nl, nt, nw, nh, TRUE);
                return 1;
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_USER + 1 && lp == WM_LBUTTONDOWN) PostQuitMessage(0);
    return DefWindowProcW(hWnd, msg, wp, lp);
}

int main() {
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSW wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = GetModuleHandle(NULL);
    wc.lpszClassName = L"HiddenClass";
    RegisterClassW(&wc);
    hwnd = CreateWindowW(wc.lpszClassName, L"", 0,0,0,0,0, NULL, NULL, wc.hInstance, NULL);
    if (!hwnd) return 1;

    CreateOverlay();
    NOTIFYICONDATAW nid = {0};
    nid.cbSize = sizeof(nid);
    nid.hWnd   = hwnd;
    nid.uID    = 1;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = WM_USER + 1;
    // nid.hIcon = (HICON)LoadImageW(NULL, L"resizekey.ico", IMAGE_ICON, 16,16, LR_LOADFROMFILE);
    nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY_ICON));
    if (!nid.hIcon) std::cerr << "Error loading resizekey.ico" << std::endl;
    wcscpy_s(nid.szTip, L"Click to exit");
    Shell_NotifyIconW(NIM_ADD, &nid);

    hHook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnhookWindowsHookEx(hHook);
    Shell_NotifyIconW(NIM_DELETE, &nid);
    DestroyWindow(hwnd);
    if (hOverlay) DestroyWindow(hOverlay);
    if (hOverlayFont) DeleteObject(hOverlayFont);
    return 0;
}
