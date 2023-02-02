//--------------------------------------------------------------------------------------
// File: mouse.h
//
// �֗��ȃ}�E�X���W���[��
//
//--------------------------------------------------------------------------------------
// 2020/02/11
//     DirectXTK���A�Ȃ񂿂����C����p�ɃV�F�C�v�A�b�v����
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


// �}�E�X���[�h
typedef enum Mouse_PositionMode_tag
{
    MOUSE_POSITION_MODE_ABSOLUTE, // ��΍��W���[�h
    MOUSE_POSITION_MODE_RELATIVE, // ���΍��W���[�h
} Mouse_PositionMode;


// �}�E�X��ԍ\����
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
    // �}�E�X���W���[���̏�����
    static void Mouse_Initialize(HWND window);

    // �}�E�X���W���[���̏I������
    static  void Mouse_Finalize(void);

    // �}�E�X�̏�Ԃ��擾����
    static  void Mouse_GetState(void);

    // �ݐς����}�E�X�X�N���[���z�C�[���l�����Z�b�g����
    static  void Mouse_ResetScrollWheelValue(void);

    // �}�E�X�̃|�W�V�������[�h��ݒ肷��i�f�t�H���g�͐�΍��W���[�h�j
    static  void Mouse_SetMode(Mouse_PositionMode mode);

    // �}�E�X�̐ڑ������o����
    static  bool Mouse_IsConnected(void);

    // �}�E�X�J�[�\�����\������Ă��邩�m�F����
    static  bool Mouse_IsVisible(void);

    // �}�E�X�J�[�\���\����ݒ肷��
    static  void Mouse_SetVisible(bool visible);

    // �}�E�X����̂��߂̃E�B���h�E���b�Z�[�W�v���V�[�W���t�b�N�֐�
    static  void Mouse_ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam);

    static void clipToWindow(void);

    static Mouse_State* GetMouseState() { return &m_state; }
};




// �������@
//
// �Ώۂ̃E�B���h�E���������ꂽ�炻�̃E�B���h�E�n���h���������ɏ������֐����Ă�
//
// Mouse_Initialize(hwnd);
//
// �E�B���h�E���b�Z�[�W�v���V�[�W������}�E�X����p�t�b�N�֐����Ăяo��
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
