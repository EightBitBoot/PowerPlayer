#include <string.h>

#include <windows.h>

#define ICON_ID 1
#define WM_ICON_MESSAGE (WM_USER)

#define WINDOW_CLASS_NAME "PowerPlayerDummy"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

    HWND dummyWindow = CreateWindowA(WINDOW_CLASS_NAME, "DummyWindow", WS_DISABLED, 0, 0, 0, 0, NULL, NULL, hInstance, 0);
    if(!dummyWindow) {
        MessageBoxA(NULL, "Failed to create dummy window!\nGo yell this at the developer.", "PowerPlayer: Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    char *szTip = "PowerPlayer";
    HICON hIcon = (HICON) LoadImageA(NULL, "PowerPlayer.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
    if(!hIcon) {
        MessageBoxA(NULL, "Failed to load icon!", "PowerPlayer: Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    NOTIFYICONDATAA iconData = {0};
    iconData.cbSize = sizeof(iconData);
    iconData.hWnd = dummyWindow;
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
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}