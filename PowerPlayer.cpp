#include <stdio.h>
#include <string.h>

#include <windows.h>
#include <powrprof.h>

#define ICON_ID 1
#define WM_ICON_MESSAGE (WM_USER)

#define WINDOW_CLASS_NAME "PPParent"

#define IDM_ICONMENU_BASE        0x1000
#define IDM_ICONMENU_CTRLPANEL   (IDM_ICONMENU_BASE + 0x0)
#define IDM_ICONMENU_QUIT        (IDM_ICONMENU_BASE + 0x1)

#define IDM_ICONMENU_SCHEME_BASE (IDM_ICONMENU_BASE + 0x100)

typedef struct PowerScheme_ {
    GUID guid;
    wchar_t *name;
    bool isCurrent;
} PowerScheme;

typedef struct ListNode_ {
    PowerScheme data;
    ListNode_ *next;
} ListNode;

ListNode *listCreate(PowerScheme &data) {
    ListNode *newNode = (ListNode *) malloc(sizeof(ListNode));
    memcpy(&newNode->data, &data, sizeof(PowerScheme));
    newNode->next = NULL;

    return newNode;
}

void listAppendData(ListNode *list, PowerScheme &data) {
    ListNode *current = list;

    while(current->next != NULL) {
        current = current->next;
    }

    current->next = listCreate(data);
}

PowerScheme listGetData(ListNode *list, int i) {
    ListNode *current = list;
    int index = 0;
    
    while(index < i) {
        current = current->next;
        index++;
    }

    return current->data;
}

void listDestroy(ListNode *list) {
    if(list->next == NULL) {
        free(list->data.name);
        free(list);
    }
    else {
        listDestroy(list->next);
        free(list->data.name);
        free(list);
    }
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HMENU buildMenu(HWND parentWindow, ListNode *schemeList);
ListNode *getPowerSchemes();
void openPowerSettings();

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

    HICON hIcon = LoadIconA(hInstance, "MainIcon");
    if(!hIcon) {
        MessageBoxA(NULL, "Failed to load icon!", "PowerPlayer: Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    char *szTip = "PowerPlayer";

    NOTIFYICONDATAA iconData = {0};
    iconData.cbSize = sizeof(NOTIFYICONDATAA);
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

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
        case WM_ICON_MESSAGE:
            switch(lParam) {
                case WM_LBUTTONDBLCLK:
                    openPowerSettings();
                    break;

                case WM_MOUSEMOVE: {
                    GUID *currentScheme = NULL;
                    wchar_t *name = NULL;
                    DWORD nameSize = 0;
                    char szTip[128] = {0};
                    
                    PowerGetActiveScheme(NULL, &currentScheme);
                    PowerReadFriendlyName(NULL, currentScheme, NULL, NULL, NULL, &nameSize);
                    name = (wchar_t *) malloc(nameSize);
                    PowerReadFriendlyName(NULL, currentScheme, NULL, NULL, (UCHAR *) name, &nameSize);
                    snprintf(&szTip[0], 128, "PowerPlayer\n%S", name);
                    if(wcslen(name) + 12 > 127) {
                        // Add trailing "..." if the current scheme is too large for the tip
                        memset(&szTip[125], '.', 3);
                    }

                    NOTIFYICONDATAA iconData = {0}; 
                    iconData.cbSize = sizeof(NOTIFYICONDATAA);
                    iconData.hWnd = hWnd;
                    iconData.uID = ICON_ID;
                    iconData.uFlags = NIF_TIP;
                    memcpy(iconData.szTip, szTip, strlen(szTip));

                    Shell_NotifyIconA(NIM_MODIFY, &iconData);

                    free(name);
                    LocalFree(currentScheme);
                    
                    break;
                }

                case WM_RBUTTONDOWN: {
                    ListNode *schemeList = getPowerSchemes();
                    HMENU menu = buildMenu(hWnd, schemeList);
                    int menuChoice = 0;
                    POINT cursorPos;
                    GetCursorPos(&cursorPos);

                    SetForegroundWindow(hWnd);
                    menuChoice = TrackPopupMenu(menu, GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_RETURNCMD | TPM_NONOTIFY, cursorPos.x, cursorPos.y, 0, hWnd, NULL);
                    PostMessageA(hWnd, WM_NULL, 0, 0);

                    DestroyMenu(menu);

                    switch(menuChoice) {
                        case IDM_ICONMENU_CTRLPANEL: {
                            // User chose control panel
                            openPowerSettings();
                            break;
                        }

                        case IDM_ICONMENU_QUIT: {
                            // User chose to quit
                            PostQuitMessage(0);
                        }

                        default: {
                            // User chose a power scheme
                            int chosenSchemeIndex = menuChoice - IDM_ICONMENU_SCHEME_BASE;
                            PowerScheme chosenScheme = listGetData(schemeList, chosenSchemeIndex);
                            PowerSetActiveScheme(NULL, &chosenScheme.guid);

                            break;
                        }
                    }

                    listDestroy(schemeList);

                    break;
                }
            }
            return 0;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

HMENU buildMenu(HWND parentWindow, ListNode *schemeList) {
    HMENU menu = CreatePopupMenu();

    if(menu) {
        ListNode *current = schemeList;
        int listIndex = 0;
        while(current != NULL) {
            AppendMenuW(menu, MF_STRING | (current->data.isCurrent ? MF_CHECKED : 0), IDM_ICONMENU_SCHEME_BASE + listIndex, current->data.name);
            current = current->next;
            listIndex++;
        }

        AppendMenuA(menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(menu, MF_STRING, IDM_ICONMENU_CTRLPANEL, "Open Power Settings");
        AppendMenuA(menu, MF_SEPARATOR, 0, NULL);
        AppendMenuA(menu, MF_STRING, IDM_ICONMENU_QUIT, "Quit");
    }

    return menu;
}

ListNode *getPowerSchemes() {
    ListNode *list = NULL;
    int index = 0;

    GUID *activeScheme = NULL;
    PowerScheme currentScheme = {0};
    DWORD schemeGUIDSize = sizeof(GUID);
    DWORD nameBufferSize = 0;

    PowerGetActiveScheme(NULL, &activeScheme);

    while(PowerEnumerate(NULL, NULL, NULL, ACCESS_SCHEME, index, (UCHAR *) &currentScheme.guid, &schemeGUIDSize) != ERROR_NO_MORE_ITEMS) {
        PowerReadFriendlyName(NULL, &currentScheme.guid, NULL, NULL, NULL, &nameBufferSize);
        currentScheme.name = (wchar_t *) malloc(nameBufferSize);
        PowerReadFriendlyName(NULL, &currentScheme.guid, NULL, NULL, (UCHAR *) currentScheme.name, &nameBufferSize);

        currentScheme.isCurrent = false;
        if(IsEqualGUID(currentScheme.guid, *activeScheme)) {
            currentScheme.isCurrent = true;
        }

        if(list == NULL) {
            list = listCreate(currentScheme);
        } 
        else {
            listAppendData(list, currentScheme);
        }

        index++;
    }

    LocalFree(activeScheme);

    return list;
}

void openPowerSettings() {
    char command[MAX_PATH + 50 + 1]; // Max path length + length of control panel and options + byte for null terminator

    GetWindowsDirectoryA(&command[0], MAX_PATH);
    sprintf(&command[strlen(command)], "%s", "\\system32\\control.exe /name Microsoft.PowerOptions");
    WinExec(command, SW_SHOW);
}