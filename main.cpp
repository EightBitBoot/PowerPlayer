#include <string.h>

#include <windows.h>

#define ICON_ID 1
#define WM_ICON_MESSAGE (WM_USER)

#define WINDOW_CLASS_NAME "PPParent"

#define IDM_BASE 0x1000
#define IDM_NAME (IDM_BASE + 0x0)
#define IDM_QUIT (IDM_BASE + 0x1)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HMENU buildMenu(HWND parentWindow);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WNDCLASSA windowClass = {0};
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    int result = RegisterClassA(&windowClass);
    if(!result) {
        MessageBoxA(NULL, "Failed to register window class!\nGo yell this at the developer.", "PowerPlayer: Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    HWND parentWindow = CreateWindowA(WINDOW_CLASS_NAME, "ParentWindow", WS_DISABLED, 0, 0, 0, 0, NULL, NULL, hInstance, 0);
    if(!parentWindow) {
        MessageBoxA(NULL, "Failed to create parent window!\nGo yell this at the developer.", "PowerPlayer: Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    HICON hIcon = (HICON) LoadImageA(NULL, "PowerPlayer.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if(!hIcon) {
        MessageBoxA(NULL, "Failed to load icon!", "PowerPlayer: Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    char *szTip = "PowerPlayer";

    NOTIFYICONDATAA iconData = {0};
    iconData.cbSize = sizeof(iconData);
    iconData.hWnd = parentWindow;
    iconData.uID = ICON_ID;
    iconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    iconData.uCallbackMessage = WM_ICON_MESSAGE;
    iconData.hIcon = hIcon;
    memcpy(iconData.szTip, szTip, strlen(szTip));

    Shell_NotifyIconA(NIM_ADD, &iconData);

    MSG msg = {0};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIconA(NIM_DELETE, &iconData);

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_ICON_MESSAGE:
            switch(lParam) {
                case WM_MOUSEMOVE:
                    // TODO(Adin): Update icon tooltip to have the current power profile
                    break;

                case WM_RBUTTONDOWN:
                    HMENU menu = buildMenu(hwnd);
                    POINT cursorPos;
                    GetCursorPos(&cursorPos);

                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(menu, GetSystemMetrics(SM_MENUDROPALIGNMENT), cursorPos.x, cursorPos.y, 0, hwnd, NULL);
                    PostMessageA(hwnd, WM_NULL, 0, 0);

                    DestroyMenu(menu);
                    break;
            }
            return 0;

        case WM_COMMAND:
            if(HIWORD(wParam) == 0) {
                // Message came from a menu
                switch(LOWORD(wParam)) {
                    case IDM_QUIT:
                        PostQuitMessage(0);
                        break;

                    // TODO(Adin): Handle other menu items
                }
                return 0;
            }

            return 1; // Didn't process the message

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

HMENU buildMenu(HWND parentWindow) {
    HMENU menu = CreatePopupMenu();

    if(menu) {
        AppendMenuA(menu, MF_STRING | MF_DISABLED, IDM_NAME, "Power Player");
        AppendMenuA(menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(menu, MF_STRING, IDM_QUIT, "Quit");
    }

    return menu;
}