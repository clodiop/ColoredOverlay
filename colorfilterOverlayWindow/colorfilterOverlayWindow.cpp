#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <regex>
#pragma warning(disable : 4996)

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
TCHAR szClassName[] = _T("ColorOverlayApp");

RECT getAllScreens()
{
    RECT maxScreen;
    maxScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    maxScreen.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    maxScreen.right = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    maxScreen.bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    return maxScreen;
}


bool HexColorToRGB(const std::wstring& hexColor, COLORREF* color) {
    if (hexColor.size() != 7 || hexColor[0] != '#') {
        // Invalid format (e.g., #RRGGBB expected)
        *color =  RGB(0, 0, 0);  // Default to black
        return false;
    }

    int red, green, blue;
    if (swscanf(hexColor.c_str(), L"#%2x%2x%2x", &red, &green, &blue) != 3) {
        // Conversion failed
        *color =  RGB(0, 0, 0); 
        return false;
    }

    *color =  RGB(red, green, blue);
    return true;
}
BOOL isAppRunning()
//The system closes the handle automatically when the process terminates.
{
    if (CreateMutex(NULL, TRUE, szClassName) && GetLastError() == ERROR_ALREADY_EXISTS) {
        return TRUE; //application is running
    }

    return FALSE;
}

struct windowData
{
    COLORREF color = RGB(0xCC, 0x35, 0xD9);
};
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nCmdShow)
{
    RECT maxScreen = getAllScreens();
    int amount_of_args;
    LPWSTR* str = CommandLineToArgvW(GetCommandLineW(), &amount_of_args);
    
    int alphaval = 60;
    windowData wd;
    if (amount_of_args == 3){
         bool worked = HexColorToRGB(std::wstring(str[1]), &wd.color);
         alphaval = std::stoi(str[2]);
         if (!worked) {
            MessageBoxA(NULL, "RGB SETTING DIDNT WORK | COLOROVERLAY", "ERROR | COLOROVERLAY", S_OK);
            return 0;
         }
    }
    else {
        MessageBoxA(NULL, "ERROR, YOU NEED TO SPECIFY THE CORRECT ARGUMENTS   exe <hex> <alpha>  | COLOROVERLAY", "ERROR | COLOROVERLAY", S_OK);
        return 0;
    }

    if (isAppRunning()) {
        MessageBoxA(NULL,"COLOR OVERLAY IS ALREADY RUNNING | COLOROVERLAY", "Error | COLOROVERLAY",S_OK);
       return 0;
    }

    HWND hwnd;               
    MSG messages;         
    WNDCLASSEX wc ={};        

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WindowProcedure;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szClassName;
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);


    if (!RegisterClassEx(&wc))
    {
        MessageBoxA(NULL, "REGISTER THE CLASS PROPERLY | COLOROVERLAY", "Error | COLOROVERLAY", S_OK);
        return 0;
    }

    hwnd = CreateWindowEx(
        WS_EX_NOACTIVATE | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        szClassName,                                                         
        szClassName,                             
        WS_POPUP,                                                            
        maxScreen.left	,
        maxScreen.top,                                                                 
        maxScreen.right ,                                                             
        maxScreen.bottom ,                                                           
        HWND_DESKTOP,                                                        
        NULL,                                                                
        hInstance,                                                       
        &wd                                                                 
    );

    if(hwnd == NULL){

        MessageBoxA(NULL, "FAILED TO CREATE CLASS | COLOROVERLAY", "Error | COLOROVERLAY", S_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    SetLayeredWindowAttributes(hwnd, 0, alphaval, LWA_ALPHA);


    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    windowData* wd = ((windowData*)GetWindowLongPtr(hwnd, GWLP_USERDATA));

    switch (message)                  
    {
        case WM_CREATE:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lParam)->lpCreateParams);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);      
            break;
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            HBRUSH hBrush = CreateSolidBrush(wd->color); 
            FillRect(hdc, &ps.rcPaint, hBrush);
            DeleteObject(hBrush);

            EndPaint(hwnd, &ps);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); // just make sure
        }
        break;
        case WM_WINDOWPOSCHANGED:
        {
            SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
        }
        break;
        default:                      
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
