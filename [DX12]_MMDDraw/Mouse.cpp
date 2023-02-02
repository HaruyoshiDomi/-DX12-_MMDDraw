//--------------------------------------------------------------------------------------
// File: mouse.cpp
//
// 便利なマウスモジュール
//
//--------------------------------------------------------------------------------------
// 2020/02/11
//     DirectXTKより、なんちゃってC言語用にシェイプアップ改変
//
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------
#include "mouse.h"

#include <windowsx.h>
#include <assert.h>


#define SAFE_CLOSEHANDLE(h) if(h){CloseHandle(h); h = NULL;}

HWND Mouse::m_Window = NULL;
Mouse_PositionMode Mouse::m_Mode = MOUSE_POSITION_MODE_ABSOLUTE;
HANDLE             Mouse::m_ScrollWheelValue = NULL;
HANDLE             Mouse::m_RelativeRead = NULL;
HANDLE             Mouse::m_AbsoluteMode = NULL;
HANDLE             Mouse::m_RelativeMode = NULL;
int                Mouse::m_RelativeX = INT32_MAX;
int                Mouse::m_RelativeY = INT32_MAX;
bool               Mouse::m_InFocus = true;
int                Mouse::m_LastX = 0;
int                Mouse::m_LastY = 0;
Mouse*             Mouse::m_instance = nullptr;
Mouse_State        Mouse::m_state = {};


void Mouse::Mouse_Initialize(HWND window)
{
    m_instance = new Mouse();
    RtlZeroMemory(&m_state, sizeof(m_state));

    assert(window != NULL);

    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01 /* HID_USAGE_PAGE_GENERIC */;
    Rid.usUsage = 0x02     /* HID_USAGE_GENERIC_MOUSE */;
    Rid.dwFlags = RIDEV_INPUTSINK;
    Rid.hwndTarget = window;
    RegisterRawInputDevices(&Rid, 1, sizeof(RAWINPUTDEVICE));

    m_Window = window;
    m_Mode = MOUSE_POSITION_MODE_ABSOLUTE;

    if (!m_ScrollWheelValue) { m_ScrollWheelValue = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE); }
    if (!m_RelativeRead) { m_RelativeRead = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE); }
    if (!m_AbsoluteMode) { m_AbsoluteMode = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE); }
    if (!m_RelativeMode) { m_RelativeMode = CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE); }

    m_LastX = 0;
    m_LastY = 0;
    m_RelativeX = INT32_MAX;
    m_RelativeY = INT32_MAX;

    m_InFocus = true;
}

void Mouse::Mouse_Finalize(void)
{
    SAFE_CLOSEHANDLE(m_ScrollWheelValue);
    SAFE_CLOSEHANDLE(m_RelativeRead);
    SAFE_CLOSEHANDLE(m_AbsoluteMode);
    SAFE_CLOSEHANDLE(m_RelativeMode);
}

void Mouse::Mouse_GetState(void)
{
    memcpy(&m_state, &m_state, sizeof(m_state));
    m_state.positionMode = m_Mode;

    DWORD result = WaitForSingleObjectEx(m_ScrollWheelValue, 0, FALSE);
    if (result == WAIT_FAILED) { return; }

    if (result == WAIT_OBJECT_0) {

        m_state.scrollWheelValue = 0;
    }

    if (m_state.positionMode == MOUSE_POSITION_MODE_RELATIVE) {

        result = WaitForSingleObjectEx(m_RelativeRead, 0, FALSE);
        if (result == WAIT_FAILED) { return; }

        if (result == WAIT_OBJECT_0) {
            m_state.x = 0;
            m_state.y = 0;
        }
        else {
            SetEvent(m_RelativeRead);
        }
    }
}

void Mouse::Mouse_ResetScrollWheelValue(void)
{
    SetEvent(m_ScrollWheelValue);
}

void Mouse::Mouse_SetMode(Mouse_PositionMode mode)
{
    if (m_Mode == mode)
        return;

    SetEvent((mode == MOUSE_POSITION_MODE_ABSOLUTE) ? m_AbsoluteMode : m_RelativeMode);

    assert(m_Window != NULL);

    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_HOVER;
    tme.hwndTrack = m_Window;
    tme.dwHoverTime = 1;
    TrackMouseEvent(&tme);
}

bool Mouse::Mouse_IsConnected(void)
{
    return GetSystemMetrics(SM_MOUSEPRESENT) != 0;
}

bool Mouse::Mouse_IsVisible(void)
{
    if (m_Mode == MOUSE_POSITION_MODE_RELATIVE) {
        return false;
    }

    CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
    GetCursorInfo(&info);

    return (info.flags & CURSOR_SHOWING) != 0;
}

void Mouse::Mouse_SetVisible(bool visible)
{
    if (m_Mode == MOUSE_POSITION_MODE_RELATIVE) {
        return;
    }

    CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
    GetCursorInfo(&info);

    bool isVisible = (info.flags & CURSOR_SHOWING) != 0;

    if (isVisible != visible) {
        ShowCursor(visible);
    }
}

void Mouse::Mouse_ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    HANDLE evts[3] = {
        m_ScrollWheelValue,
        m_AbsoluteMode,
        m_RelativeMode
    };

    switch (WaitForMultipleObjectsEx(_countof(evts), evts, FALSE, 0, FALSE))
    {
    case WAIT_OBJECT_0:
        m_state.scrollWheelValue = 0;
        ResetEvent(evts[0]);
        break;

    case (WAIT_OBJECT_0 + 1):
    {
        m_Mode = MOUSE_POSITION_MODE_ABSOLUTE;
        ClipCursor(nullptr);

        POINT point;
        point.x = m_LastX;
        point.y = m_LastY;

        // リモートディスクトップに対応するために移動前にカーソルを表示する
        ShowCursor(TRUE);

        if (MapWindowPoints(m_Window, nullptr, &point, 1)) {
            SetCursorPos(point.x, point.y);
        }

        m_state.x = m_LastX;
        m_state.y = m_LastY;
    }
    break;

    case (WAIT_OBJECT_0 + 2):
    {
        ResetEvent(m_RelativeRead);

        m_Mode = MOUSE_POSITION_MODE_RELATIVE;
        m_state.x = m_state.y = 0;
        m_RelativeX = INT32_MAX;
        m_RelativeY = INT32_MAX;

        ShowCursor(FALSE);

        clipToWindow();
    }
    break;

    case WAIT_FAILED:
        return;
    }

    switch (message)
    {
    case WM_ACTIVATEAPP:
        if (wParam) {

            m_InFocus = true;

            if (m_Mode == MOUSE_POSITION_MODE_RELATIVE) {

                m_state.x = m_state.y = 0;
                ShowCursor(FALSE);
                clipToWindow();
            }
        }
        else {
            int scrollWheel = m_state.scrollWheelValue;
            memset(&m_state, 0, sizeof(m_state));
            m_state.scrollWheelValue = scrollWheel;
            m_InFocus = false;
        }
        return;

    case WM_INPUT:
        if (m_InFocus && m_Mode == MOUSE_POSITION_MODE_RELATIVE) {

            RAWINPUT raw;
            UINT rawSize = sizeof(raw);

            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &rawSize, sizeof(RAWINPUTHEADER));

            if (raw.header.dwType == RIM_TYPEMOUSE) {

                if (!(raw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {

                    m_state.x = raw.data.mouse.lLastX;
                    m_state.y = raw.data.mouse.lLastY;

                    ResetEvent(m_RelativeRead);
                }
                else if (raw.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) {

                    // リモートディスクトップなどに対応
                    const int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                    const int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

                    int x = (int)((raw.data.mouse.lLastX / 65535.0f) * width);
                    int y = (int)((raw.data.mouse.lLastY / 65535.0f) * height);

                    if (m_RelativeX == INT32_MAX) {
                        m_state.x = m_state.y = 0;
                    }
                    else {
                        m_state.x = x - m_RelativeX;
                        m_state.y = y - m_RelativeY;
                    }

                    m_RelativeX = x;
                    m_RelativeY = y;

                    ResetEvent(m_RelativeRead);
                }
            }
        }
        return;


    case WM_MOUSEMOVE:
        break;

    case WM_LBUTTONDOWN:
        m_state.leftButton = true;
        break;

    case WM_LBUTTONUP:
        m_state.leftButton = false;
        break;

    case WM_RBUTTONDOWN:
        m_state.rightButton = true;
        break;

    case WM_RBUTTONUP:
        m_state.rightButton = false;
        break;

    case WM_MBUTTONDOWN:
        m_state.middleButton = true;
        break;

    case WM_MBUTTONUP:
        m_state.middleButton = false;
        break;

    case WM_MOUSEWHEEL:
        m_state.scrollWheelValue += GET_WHEEL_DELTA_WPARAM(wParam);
        return;

    case WM_XBUTTONDOWN:
        switch (GET_XBUTTON_WPARAM(wParam))
        {
        case XBUTTON1:
            m_state.xButton1 = true;
            break;

        case XBUTTON2:
            m_state.xButton2 = true;
            break;
        }
        break;

    case WM_XBUTTONUP:
        switch (GET_XBUTTON_WPARAM(wParam))
        {
        case XBUTTON1:
            m_state.xButton1 = false;
            break;

        case XBUTTON2:
            m_state.xButton2 = false;
            break;
        }
        break;

    case WM_MOUSEHOVER:
        break;

    default:
        // マウスに対するメッセージは無かった…
        return;
    }

    if (m_Mode == MOUSE_POSITION_MODE_ABSOLUTE) {

        // すべてのマウスメッセージに対して新しい座標を取得する
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);

        m_state.x = m_LastX = xPos;
        m_state.y = m_LastY = yPos;
    }
}

void Mouse::clipToWindow(void)
{
    assert(m_Window != NULL);

    RECT rect;
    GetClientRect(m_Window, &rect);

    POINT ul;
    ul.x = rect.left;
    ul.y = rect.top;

    POINT lr;
    lr.x = rect.right;
    lr.y = rect.bottom;

    MapWindowPoints(m_Window, NULL, &ul, 1);
    MapWindowPoints(m_Window, NULL, &lr, 1);

    rect.left = ul.x;
    rect.top = ul.y;

    rect.right = lr.x;
    rect.bottom = lr.y;

    ClipCursor(&rect);
}

