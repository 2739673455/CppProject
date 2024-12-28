#include <windows.h>
#include <tchar.h>
#include <atlimage.h>
#include <imm.h>
#pragma comment(lib, "Imm32.lib")

// 全局变量
#define WM_TRAYICON (WM_USER + 1) // 自定义消息，用于托盘图标事件
#define ID_TRAY_EXIT 1001         // 菜单项 ID：退出

NOTIFYICONDATA nid;              // 通知区域图标结构体

struct WindowProcessParam { CImage* image; int ttl; HWND hwnd; int inputMode; };
const WCHAR* zhongImagePath = L"C:\\Users\\27396\\Downloads\\InputModePicture\\zhong.png"; // 0b011
const WCHAR* yingImagePath = L"C:\\Users\\27396\\Downloads\\InputModePicture\\ying.png"; // 0b010
const WCHAR* enImagePath = L"C:\\Users\\27396\\Downloads\\InputModePicture\\en.png"; // 0b000
const WCHAR* A_zhImagePath = L"C:\\Users\\27396\\Downloads\\InputModePicture\\a_zh.png"; // 0b111,0b110
const WCHAR* A_enImagePath = L"C:\\Users\\27396\\Downloads\\InputModePicture\\a_en.png"; // 0b100
const HKL PIN_CODE = reinterpret_cast<HKL>(0x8040804);
const HKL EN_CODE = reinterpret_cast<HKL>(0x4090409);


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void createWindow(HINSTANCE hInstance, WNDCLASS wc, LPCWSTR windowName, WindowProcessParam winProcessP);
WNDCLASS getWindowClass(HINSTANCE hInstance, LPCWSTR className);
void setImage(CImage* image, int currentInputMode);
int getInputMode(HWND hwnd);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    CImage image;
    setImage(&image, 0);
    WNDCLASS wc = getWindowClass(hInstance, L"ImageShowWindowClass");
    createWindow(hInstance, wc, L"ImageShowWindow", { &image, 1000, 0, 0 });
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}

// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {

    case WM_NCCREATE: {
        // 提取结构体引用并存储到窗口数据中
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        WindowProcessParam* pParam = (WindowProcessParam*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pParam);
        // 设置定时器，周期检测活动窗口的输入法状态
        SetTimer(hwnd, 2, 100, NULL);
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    case WM_CREATE: {
        // 初始化托盘图标
        ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uID = 1; // 图标 ID
        nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON; // 托盘图标的回调消息
        nid.hIcon = LoadIcon(NULL, IDI_INFORMATION); // 使用系统图标
        lstrcpy(nid.szTip, L"My Tray Icon"); // 提示文字
        Shell_NotifyIcon(NIM_ADD, &nid);    // 添加到托盘
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        WindowProcessParam* pParam = (WindowProcessParam*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        // 设置定时器，ttl毫秒之后销毁窗口
        SetTimer(hwnd, 1, pParam->ttl, NULL);
        HDC hdc = BeginPaint(hwnd, &ps);
        pParam->image->Draw(hdc, CRect(0, 0, pParam->image->GetWidth(), pParam->image->GetHeight()));
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_TIMER: {
        switch (wParam) {
        case 1: {
            KillTimer(hwnd, 1);
            ShowWindow(hwnd, SW_HIDE);
            break;
        }
        case 2: {
            WindowProcessParam* pParam = (WindowProcessParam*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            HWND currentHwnd = GetForegroundWindow();
            int currentInputMode = getInputMode(currentHwnd);
            if (currentHwnd == hwnd) break;
            if (currentHwnd != pParam->hwnd || currentInputMode != pParam->inputMode) {
                ShowWindow(hwnd, SW_HIDE);
                setImage(pParam->image, currentInputMode);
                ShowWindow(hwnd, SW_SHOW);
                pParam->hwnd = currentHwnd;
                pParam->inputMode = currentInputMode;
            }
            break;
        }
        }
        return 0;
    }
    case WM_TRAYICON: {
        if (lParam == WM_RBUTTONDOWN) {
            // 右键点击托盘图标
            POINT pt;
            GetCursorPos(&pt); // 获取鼠标位置

            // 创建菜单
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
            SetForegroundWindow(hwnd); // 确保菜单在前台显示

            // 显示菜单
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu); // 销毁菜单
        }
        return 0;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_TRAY_EXIT) {
            SendMessage(hwnd, WM_DESTROY, 0, 0);
        }
        return 0;
    }
    case WM_CLOSE: {
        DestroyWindow(hwnd);
        return 0;
    }
    case WM_DESTROY: {
        Shell_NotifyIcon(NIM_DELETE, &nid); // 删除托盘图标
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

// 创建窗口
void createWindow(HINSTANCE hInstance, WNDCLASS wc, LPCWSTR windowName, WindowProcessParam winProcessP) {
    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
        wc.lpszClassName,
        windowName,
        WS_VISIBLE | WS_POPUP,
        (GetSystemMetrics(SM_CXSCREEN) - winProcessP.image->GetWidth()) / 2, 150,
        winProcessP.image->GetWidth(), winProcessP.image->GetHeight(),
        NULL, NULL, hInstance, &winProcessP
    );
    SetLayeredWindowAttributes(hwnd, RGB(255, 255, 255), (BYTE)150, LWA_COLORKEY | LWA_ALPHA);
}

// 注册窗口类
WNDCLASS getWindowClass(HINSTANCE hInstance, LPCWSTR className) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // 默认光标样式
    RegisterClass(&wc);
    return wc;
}

// 图片load
void setImage(CImage* image, int currentInputMode) {
    switch (currentInputMode) {
    case 0b111: {
        image->Load(A_zhImagePath);
        break;
    }
    case 0b110: {
        image->Load(A_enImagePath);
        break;
    }
    case 0b100: {
        image->Load(A_enImagePath);
        break;
    }
    case 0b011: {
        image->Load(zhongImagePath);
        break;
    }
    case 0b010: {
        image->Load(yingImagePath);
        break;
    }
    case 0b000: {
        image->Load(enImagePath);
        break;
    }
    default: {
        image->Load(enImagePath);
    }
    }
}

// 输入法模式识别
int getInputMode(HWND hwnd) {
    // 判断大小写: 大写 1 | 小写 0
    int capsMode = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    // 判断输入法: 微软拼音 1 | En 0
    DWORD threadId = GetWindowThreadProcessId(hwnd, nullptr);
    HKL hkl = GetKeyboardLayout(threadId);
    int inputMode = (hkl == PIN_CODE);

    // 判断微软拼音: 中文 1 | 英文 0
    // IME 类的默认窗口句柄
    HWND imeHwnd = ImmGetDefaultIMEWnd(hwnd);
    int imeMode = 0;
    LRESULT inputState = SendMessage(imeHwnd, WM_IME_CONTROL, 0x005, 0);
    if (inputState == 1) {
        LRESULT setState = SendMessage(imeHwnd, WM_IME_CONTROL, 0x001, 0);
        imeMode = setState & 1;
    }
    return (capsMode << 2) + (inputMode << 1) + imeMode;
}