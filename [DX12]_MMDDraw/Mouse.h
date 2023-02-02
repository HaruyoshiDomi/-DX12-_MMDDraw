//--------------------------------------------------------------------------------------
// File: mouse.h
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
#ifndef HAL_YOUHEI_MOUSE_H
#define HAL_YOUHEI_MOUSE_H
#pragma once


#include <windows.h>
#include <memory>


// マウスモード
typedef enum Mouse_PositionMode_tag
{
    MOUSE_POSITION_MODE_ABSOLUTE, // 絶対座標モード
    MOUSE_POSITION_MODE_RELATIVE, // 相対座標モード
} Mouse_PositionMode;


// マウス状態構造体
typedef struct
{
    bool leftButton;
    bool middleButton;
    bool rightButton;
    bool xButton1;
    bool xButton2;
    float x;
    float y;
    int scrollWheelValue;
    Mouse_PositionMode positionMode;
} Mouse_State;

class Mouse
{
private:
    static HWND               m_Window ;
    static Mouse_PositionMode m_Mode;
    static HANDLE             m_ScrollWheelValue;
    static HANDLE             m_RelativeRead;
    static HANDLE             m_AbsoluteMode;
    static HANDLE             m_RelativeMode;
    static int                m_RelativeX;
    static int                m_RelativeY;
    static bool               m_InFocus;
    static Mouse_State        m_state;
    static int                m_LastX;
    static int                m_LastY;
    static Mouse* m_instance;

    Mouse(){}

public:
    Mouse* GetInstanse()
    {
        if (!m_instance)
            m_instance = new Mouse();

        return m_instance;
    }
    // マウスモジュールの初期化
    static void Mouse_Initialize(HWND window);

    // マウスモジュールの終了処理
    static  void Mouse_Finalize(void);

    // マウスの状態を取得する
    static  void Mouse_GetState(void);

    // 累積したマウススクロールホイール値をリセットする
    static  void Mouse_ResetScrollWheelValue(void);

    // マウスのポジションモードを設定する（デフォルトは絶対座標モード）
    static  void Mouse_SetMode(Mouse_PositionMode mode);

    // マウスの接続を検出する
    static  bool Mouse_IsConnected(void);

    // マウスカーソルが表示されているか確認する
    static  bool Mouse_IsVisible(void);

    // マウスカーソル表示を設定する
    static  void Mouse_SetVisible(bool visible);

    // マウス制御のためのウィンドウメッセージプロシージャフック関数
    static  void Mouse_ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

    static void clipToWindow(void);

    static Mouse_State* GetMouseState() { return &m_state; }
};




// 導入方法
//
// 対象のウィンドウが生成されたらそのウィンドウハンドルを引数に初期化関数を呼ぶ
//
// Mouse_Initialize(hwnd);
//
// ウィンドウメッセージプロシージャからマウス制御用フック関数を呼び出す
//
// LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
// {
//     switch (message)
//     {
//     case WM_ACTIVATEAPP:
//     case WM_INPUT:
//     case WM_MOUSEMOVE:
//     case WM_LBUTTONDOWN:
//     case WM_LBUTTONUP:
//     case WM_RBUTTONDOWN:
//     case WM_RBUTTONUP:
//     case WM_MBUTTONDOWN:
//     case WM_MBUTTONUP:
//     case WM_MOUSEWHEEL:
//     case WM_XBUTTONDOWN:
//     case WM_XBUTTONUP:
//     case WM_MOUSEHOVER:
//         Mouse_ProcessMessage(message, wParam, lParam);
//         break;
//
//     }
// }
//

#endif // HAL_YOUHEI_MOUSE_H
