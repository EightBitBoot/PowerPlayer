#include <string.h>

#include <windows.h>
#include <powrprof.h>

#define ICON_ID 1
#define WM_ICON_MESSAGE (WM_USER)

#define WINDOW_CLASS_NAME "PPParent"

#define IDM_ICONMENU_BASE 0x1000
#define IDM_ICONMENU_QUIT (IDM_ICONMENU_BASE + 0x0)

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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HMENU buildMenu(HWND parentWindow, ListNode *schemeList);
ListNode *getPowerSchemes();

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

                case WM_RBUTTONDOWN: {
                    ListNode *schemeList = getPowerSchemes();
                    HMENU menu = buildMenu(hwnd, schemeList);
                    int menuChoice = 0;
                    POINT cursorPos;
                    GetCursorPos(&cursorPos);

                    SetForegroundWindow(hwnd);
                    menuChoice = TrackPopupMenu(menu, GetSystemMetrics(SM_MENUDROPALIGNMENT) | TPM_RETURNCMD | TPM_NONOTIFY, cursorPos.x, cursorPos.y, 0, hwnd, NULL);
                    PostMessageA(hwnd, WM_NULL, 0, 0);

                    DestroyMenu(menu);

                    if(menuChoice == IDM_ICONMENU_QUIT) {
                        // User chose to quit
                        PostQuitMessage(0);
                    } 
                    else {
                        // User chose a power scheme
                        int chosenSchemeIndex = menuChoice - IDM_ICONMENU_SCHEME_BASE;
                        PowerScheme chosenScheme = listGetData(schemeList, chosenSchemeIndex);
                        PowerSetActiveScheme(NULL, &chosenScheme.guid);
                    }

                    listDestroy(schemeList);

                    break;
                }
            }
            return 0;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
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