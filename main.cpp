#include <windows.h>

#define WINDOW_CLASS_NAME "PowerPlayerDummy"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    WNDCLASSA windowClass = {0};
    windowClass.lpfnWndProc = &WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = WINDOW_CLASS_NAME;

    int result = RegisterClassA(&windowClass);
    if(!result) {
        MessageBoxA(NULL, "Ruh Roe Raggy", "Railed ro register rindow rass", MB_ICONERROR | MB_OK);
        return -1;
    }

    HWND dummyWindow = CreateWindowA(WINDOW_CLASS_NAME, "Nothing To See Here Officer", WS_DISABLED, 0, 0, 0, 0, NULL, NULL, hInstance, 0);
    if(!dummyWindow) {
        MessageBoxA(NULL, "Oops I Did It Again", "Window is no no", MB_ICONERROR | MB_OK);
        return -1;
    }

    NOTIFYICONDATA notifIcon = {0};
    notifIcon.cbSize = sizeof(NOTIFYICONDATA);
    notifIcon.hWnd = dummyWindow;


    MSG msg = {0};
    while(GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
