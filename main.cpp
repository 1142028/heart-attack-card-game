#include <windows.h>
#include <tchar.h>

#define ID_SINGLE  1001
#define ID_DOUBLE 1002
#define ID_MUTIPLE 1003
#define ID_HELP_ABOUT 1004

void DrawSuit(HDC hdc, int x, int y, LPCTSTR suit, bool flip = false)
{
    if (!flip)
    {
        TextOut(hdc, x, y, suit, lstrlen(suit));
    }
    else
    {
        SetGraphicsMode(hdc, GM_ADVANCED);

        XFORM oldForm;
        GetWorldTransform(hdc, &oldForm);

        XFORM form;
        form.eM11 = -1.0f;
        form.eM12 = 0.0f;
        form.eM21 = 0.0f;
        form.eM22 = -1.0f;
        form.eDx = (FLOAT)(2 * x + 20);
        form.eDy = (FLOAT)(2 * y + 20);

        SetWorldTransform(hdc, &form);
        TextOut(hdc, x, y, suit, lstrlen(suit));
        SetWorldTransform(hdc, &oldForm);
    }
}

void DrawCornerFlip(HDC hdc, int x, int y, int w, int h, LPCTSTR value, LPCTSTR suit)
{
    SetGraphicsMode(hdc, GM_ADVANCED);

    XFORM oldForm;
    GetWorldTransform(hdc, &oldForm);

    XFORM form;
    form.eM11 = -1.0f;
    form.eM12 = 0.0f;
    form.eM21 = 0.0f;
    form.eM22 = -1.0f;
    form.eDx = (FLOAT)(2 * x + w);
    form.eDy = (FLOAT)(2 * y + h);

    SetWorldTransform(hdc, &form);

    TextOut(hdc, x + 6, y + 5, value, lstrlen(value));
    TextOut(hdc, x + 6, y + 22, suit, lstrlen(suit));

    SetWorldTransform(hdc, &oldForm);
}

void DrawCard(
    HDC hdc,
    int x,
    int y,
    int w,
    int h,
    LPCTSTR value,
    LPCTSTR suit,
    COLORREF color)
{
    HBRUSH hWhite = CreateSolidBrush(RGB(255, 255, 255));
    RECT rc = { x, y, x + w, y + h };
    FillRect(hdc, &rc, hWhite);
    DeleteObject(hWhite);

    Rectangle(hdc, x, y, x + w, y + h);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, color);

    HFONT hSmall = CreateFont(
        18, 0, 0, 0,
        FW_BOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        VARIABLE_PITCH,
        _T("Times New Roman")
    );

    HFONT hMid = CreateFont(
        24, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        VARIABLE_PITCH,
        _T("Times New Roman")
    );

    HFONT hBig = CreateFont(
        52, 0, 0, 0,
        FW_NORMAL,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY,
        VARIABLE_PITCH,
        _T("Times New Roman")
    );

    SelectObject(hdc, hSmall);

    TextOut(hdc, x + 6, y + 5, value, lstrlen(value));
    TextOut(hdc, x + 6, y + 22, suit, lstrlen(suit));

    DrawCornerFlip(hdc, x, y, w, h, value, suit);

    int num = 0;

    if (lstrcmp(value, _T("A")) == 0) num = 1;
    else if (lstrcmp(value, _T("2")) == 0) num = 2;
    else if (lstrcmp(value, _T("3")) == 0) num = 3;
    else if (lstrcmp(value, _T("4")) == 0) num = 4;
    else if (lstrcmp(value, _T("5")) == 0) num = 5;
    else if (lstrcmp(value, _T("6")) == 0) num = 6;
    else if (lstrcmp(value, _T("7")) == 0) num = 7;
    else if (lstrcmp(value, _T("8")) == 0) num = 8;
    else if (lstrcmp(value, _T("9")) == 0) num = 9;
    else if (lstrcmp(value, _T("10")) == 0) num = 10;

    if (num == 0)
    {
        SelectObject(hdc, hBig);
        TextOut(hdc, x + w / 2 - 18, y + h / 2 - 30, value, lstrlen(value));
    }
    else
    {
        SelectObject(hdc, hMid);

        if (num == 1)
        {
            DrawSuit(hdc, x + 38, y + 58, suit);
        }
        else if (num == 2)
        {
            DrawSuit(hdc, x + 38, y + 22, suit);
            DrawSuit(hdc, x + 31, y + 92, suit, true);
        }
        else if (num == 3)
        {
            DrawSuit(hdc, x + 38, y + 18, suit);
            DrawSuit(hdc, x + 38, y + 58, suit);
            DrawSuit(hdc, x + 30, y + 98, suit, true);
        }
        else if (num == 4)
        {
            DrawSuit(hdc, x + 22, y + 20, suit);
            DrawSuit(hdc, x + 55, y + 20, suit);

            DrawSuit(hdc, x + 14, y + 92, suit, true);
            DrawSuit(hdc, x + 47, y + 92, suit, true);
        }
        else if (num == 5)
        {
            DrawSuit(hdc, x + 22, y + 18, suit);
            DrawSuit(hdc, x + 55, y + 18, suit);

            DrawSuit(hdc, x + 38, y + 58, suit);

            DrawSuit(hdc, x + 14, y + 96, suit, true);
            DrawSuit(hdc, x + 47, y + 96, suit, true);
        }
        else if (num == 6)
        {
            DrawSuit(hdc, x + 22, y + 14, suit);
            DrawSuit(hdc, x + 55, y + 14, suit);

            DrawSuit(hdc, x + 22, y + 54, suit);
            DrawSuit(hdc, x + 55, y + 54, suit);

            DrawSuit(hdc, x + 14, y + 94, suit, true);
            DrawSuit(hdc, x + 47, y + 94, suit, true);
        }
        else if (num == 7)
        {
            DrawSuit(hdc, x + 22, y + 12, suit);
            DrawSuit(hdc, x + 55, y + 12, suit);

            DrawSuit(hdc, x + 38, y + 34, suit);

            DrawSuit(hdc, x + 22, y + 54, suit);
            DrawSuit(hdc, x + 55, y + 54, suit);

            DrawSuit(hdc, x + 14, y + 94, suit, true);
            DrawSuit(hdc, x + 47, y + 94, suit, true);
        }
        else if (num == 8)
        {
            DrawSuit(hdc, x + 22, y + 16, suit);
            DrawSuit(hdc, x + 55, y + 16, suit);

            DrawSuit(hdc, x + 38, y + 40, suit);

            DrawSuit(hdc, x + 22, y + 63, suit);
            DrawSuit(hdc, x + 55, y + 63, suit);

            DrawSuit(hdc, x + 30, y + 90, suit, true);

            DrawSuit(hdc, x + 14, y + 114, suit, true);
            DrawSuit(hdc, x + 47, y + 114, suit, true);
        }
        else if (num == 9)
        {
            DrawSuit(hdc, x + 22, y + 8, suit);
            DrawSuit(hdc, x + 55, y + 8, suit);

            DrawSuit(hdc, x + 38, y + 28, suit);

            DrawSuit(hdc, x + 22, y + 48, suit);
            DrawSuit(hdc, x + 55, y + 48, suit);

            DrawSuit(hdc, x + 14, y + 76, suit, true);
            DrawSuit(hdc, x + 47, y + 76, suit, true);

            DrawSuit(hdc, x + 14, y + 108, suit, true);
            DrawSuit(hdc, x + 47, y + 108, suit, true);
        }
        else if (num == 10)
        {
            DrawSuit(hdc, x + 22, y + 14, suit);
            DrawSuit(hdc, x + 55, y + 14, suit);

            DrawSuit(hdc, x + 38, y + 34, suit);

            DrawSuit(hdc, x + 22, y + 52, suit);
            DrawSuit(hdc, x + 55, y + 52, suit);

            DrawSuit(hdc, x + 14, y + 76, suit, true);
            DrawSuit(hdc, x + 47, y + 76, suit, true);

            DrawSuit(hdc, x + 30, y + 98, suit, true);

            DrawSuit(hdc, x + 14, y + 118, suit, true);
            DrawSuit(hdc, x + 47, y + 118, suit, true);
        }
    }

    DeleteObject(hSmall);
    DeleteObject(hMid);
    DeleteObject(hBig);
}

HMENU CreateAppMenu()
{
    HMENU hMenuBar = CreateMenu();

    AppendMenu(hMenuBar, MF_STRING, ID_SINGLE, _T("單人計時賽"));
    AppendMenu(hMenuBar, MF_STRING, ID_DOUBLE, _T("雙人鍵盤競速"));
    AppendMenu(hMenuBar, MF_STRING, ID_MUTIPLE, _T("多人視覺辨識賽"));
    AppendMenu(hMenuBar, MF_STRING, ID_HELP_ABOUT, _T("說明"));

    return hMenuBar;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_SINGLE:
            MessageBox(hwnd, _T("單人模式"), _T("Menu"), MB_OK);
            break;

        case ID_DOUBLE:
            MessageBox(hwnd, _T("雙人模式"), _T("Menu"), MB_OK);
            break;

        case ID_MUTIPLE:
            MessageBox(hwnd, _T("多人模式"), _T("Menu"), MB_OK);
            break;

        case ID_HELP_ABOUT:
            MessageBox(hwnd, _T("心臟病"), _T("About"), MB_OK);
            break;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        HFONT hTitle = CreateFont(
            30, 0, 0, 0,
            FW_BOLD,
            FALSE, FALSE, FALSE,
            DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY,
            VARIABLE_PITCH,
            _T("微軟正黑體")
        );

        SelectObject(hdc, hTitle);
        TextOut(hdc, 10, 10, _T("撲克牌畫面"), 6);
        DeleteObject(hTitle);

        LPCTSTR values[] =
        {
            _T("A"), _T("2"), _T("3"), _T("4"),
            _T("5"), _T("6"), _T("7"), _T("8"),
            _T("9"), _T("10"), _T("J"), _T("Q"), _T("K")
        };

        LPCTSTR suits[] =
        {
            _T("♠"),
            _T("♥"),
            _T("♦"),
            _T("♣")
        };

        COLORREF colors[] =
        {
            RGB(0, 0, 0),
            RGB(255, 0, 0),
            RGB(255, 0, 0),
            RGB(0, 0, 0)
        };

        int cardW = 95;
        int cardH = 145;

        int startX = 10;
        int startY = 55;

        int gapX = 5;
        int gapY = 8;

        for (int s = 0; s < 4; s++)
        {
            for (int v = 0; v < 13; v++)
            {
                int x = startX + v * (cardW + gapX);
                int y = startY + s * (cardH + gapY);

                DrawCard(
                    hdc,
                    x,
                    y,
                    cardW,
                    cardH,
                    values[v],
                    suits[s],
                    colors[s]
                );
            }
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int nCmdShow)
{
    const TCHAR CLASS_NAME[] = _T("PokerWindow");

    WNDCLASS wc = {};

    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, _T("Window Registration Failed!"), _T("Error"), MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindow(
        CLASS_NAME,
        _T("心臟病(第12組1142018 1142028 1142058)"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1450,
        760,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd)
    {
        MessageBox(NULL, _T("Window Creation Failed!"), _T("Error"), MB_ICONERROR);
        return 0;
    }

    SetMenu(hwnd, CreateAppMenu());

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}