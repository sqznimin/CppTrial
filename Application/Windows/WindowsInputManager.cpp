#include "Application/Windows/WindowsInputManager.h"
#include "Application/Application.h"

WindowsInputManager inputManager;
InputManager *gInputManager = &inputManager;

void WindowsInputManager::OnLoad()
{
    std::memset(keycodeTable, CT_KEY_UNKNOWN, sizeof(keycodeTable));

    keycodeTable[0x00B] = CT_KEY_0;
    keycodeTable[0x002] = CT_KEY_1;
    keycodeTable[0x003] = CT_KEY_2;
    keycodeTable[0x004] = CT_KEY_3;
    keycodeTable[0x005] = CT_KEY_4;
    keycodeTable[0x006] = CT_KEY_5;
    keycodeTable[0x007] = CT_KEY_6;
    keycodeTable[0x008] = CT_KEY_7;
    keycodeTable[0x009] = CT_KEY_8;
    keycodeTable[0x00A] = CT_KEY_9;
    keycodeTable[0x01E] = CT_KEY_A;
    keycodeTable[0x030] = CT_KEY_B;
    keycodeTable[0x02E] = CT_KEY_C;
    keycodeTable[0x020] = CT_KEY_D;
    keycodeTable[0x012] = CT_KEY_E;
    keycodeTable[0x021] = CT_KEY_F;
    keycodeTable[0x022] = CT_KEY_G;
    keycodeTable[0x023] = CT_KEY_H;
    keycodeTable[0x017] = CT_KEY_I;
    keycodeTable[0x024] = CT_KEY_J;
    keycodeTable[0x025] = CT_KEY_K;
    keycodeTable[0x026] = CT_KEY_L;
    keycodeTable[0x032] = CT_KEY_M;
    keycodeTable[0x031] = CT_KEY_N;
    keycodeTable[0x018] = CT_KEY_O;
    keycodeTable[0x019] = CT_KEY_P;
    keycodeTable[0x010] = CT_KEY_Q;
    keycodeTable[0x013] = CT_KEY_R;
    keycodeTable[0x01F] = CT_KEY_S;
    keycodeTable[0x014] = CT_KEY_T;
    keycodeTable[0x016] = CT_KEY_U;
    keycodeTable[0x02F] = CT_KEY_V;
    keycodeTable[0x011] = CT_KEY_W;
    keycodeTable[0x02D] = CT_KEY_X;
    keycodeTable[0x015] = CT_KEY_Y;
    keycodeTable[0x02C] = CT_KEY_Z;

    keycodeTable[0x028] = CT_KEY_APOSTROPHE;
    keycodeTable[0x02B] = CT_KEY_BACKSLASH;
    keycodeTable[0x033] = CT_KEY_COMMA;
    keycodeTable[0x00D] = CT_KEY_EQUAL;
    keycodeTable[0x029] = CT_KEY_GRAVE_ACCENT;
    keycodeTable[0x01A] = CT_KEY_LEFT_BRACKET;
    keycodeTable[0x00C] = CT_KEY_MINUS;
    keycodeTable[0x034] = CT_KEY_PERIOD;
    keycodeTable[0x01B] = CT_KEY_RIGHT_BRACKET;
    keycodeTable[0x027] = CT_KEY_SEMICOLON;
    keycodeTable[0x035] = CT_KEY_SLASH;
    keycodeTable[0x056] = CT_KEY_WORLD_2;

    keycodeTable[0x00E] = CT_KEY_BACKSPACE;
    keycodeTable[0x153] = CT_KEY_DELETE;
    keycodeTable[0x14F] = CT_KEY_END;
    keycodeTable[0x01C] = CT_KEY_ENTER;
    keycodeTable[0x001] = CT_KEY_ESCAPE;
    keycodeTable[0x147] = CT_KEY_HOME;
    keycodeTable[0x152] = CT_KEY_INSERT;
    keycodeTable[0x15D] = CT_KEY_MENU;
    keycodeTable[0x151] = CT_KEY_PAGE_DOWN;
    keycodeTable[0x149] = CT_KEY_PAGE_UP;
    keycodeTable[0x045] = CT_KEY_PAUSE;
    keycodeTable[0x146] = CT_KEY_PAUSE;
    keycodeTable[0x039] = CT_KEY_SPACE;
    keycodeTable[0x00F] = CT_KEY_TAB;
    keycodeTable[0x03A] = CT_KEY_CAPS_LOCK;
    keycodeTable[0x145] = CT_KEY_NUM_LOCK;
    keycodeTable[0x046] = CT_KEY_SCROLL_LOCK;
    keycodeTable[0x03B] = CT_KEY_F1;
    keycodeTable[0x03C] = CT_KEY_F2;
    keycodeTable[0x03D] = CT_KEY_F3;
    keycodeTable[0x03E] = CT_KEY_F4;
    keycodeTable[0x03F] = CT_KEY_F5;
    keycodeTable[0x040] = CT_KEY_F6;
    keycodeTable[0x041] = CT_KEY_F7;
    keycodeTable[0x042] = CT_KEY_F8;
    keycodeTable[0x043] = CT_KEY_F9;
    keycodeTable[0x044] = CT_KEY_F10;
    keycodeTable[0x057] = CT_KEY_F11;
    keycodeTable[0x058] = CT_KEY_F12;
    keycodeTable[0x064] = CT_KEY_F13;
    keycodeTable[0x065] = CT_KEY_F14;
    keycodeTable[0x066] = CT_KEY_F15;
    keycodeTable[0x067] = CT_KEY_F16;
    keycodeTable[0x068] = CT_KEY_F17;
    keycodeTable[0x069] = CT_KEY_F18;
    keycodeTable[0x06A] = CT_KEY_F19;
    keycodeTable[0x06B] = CT_KEY_F20;
    keycodeTable[0x06C] = CT_KEY_F21;
    keycodeTable[0x06D] = CT_KEY_F22;
    keycodeTable[0x06E] = CT_KEY_F23;
    keycodeTable[0x076] = CT_KEY_F24;
    keycodeTable[0x038] = CT_KEY_LEFT_ALT;
    keycodeTable[0x01D] = CT_KEY_LEFT_CONTROL;
    keycodeTable[0x02A] = CT_KEY_LEFT_SHIFT;
    keycodeTable[0x15B] = CT_KEY_LEFT_SUPER;
    keycodeTable[0x137] = CT_KEY_PRINT_SCREEN;
    keycodeTable[0x138] = CT_KEY_RIGHT_ALT;
    keycodeTable[0x11D] = CT_KEY_RIGHT_CONTROL;
    keycodeTable[0x036] = CT_KEY_RIGHT_SHIFT;
    keycodeTable[0x15C] = CT_KEY_RIGHT_SUPER;
    keycodeTable[0x150] = CT_KEY_DOWN;
    keycodeTable[0x14B] = CT_KEY_LEFT;
    keycodeTable[0x14D] = CT_KEY_RIGHT;
    keycodeTable[0x148] = CT_KEY_UP;

    keycodeTable[0x052] = CT_KEY_KP_0;
    keycodeTable[0x04F] = CT_KEY_KP_1;
    keycodeTable[0x050] = CT_KEY_KP_2;
    keycodeTable[0x051] = CT_KEY_KP_3;
    keycodeTable[0x04B] = CT_KEY_KP_4;
    keycodeTable[0x04C] = CT_KEY_KP_5;
    keycodeTable[0x04D] = CT_KEY_KP_6;
    keycodeTable[0x047] = CT_KEY_KP_7;
    keycodeTable[0x048] = CT_KEY_KP_8;
    keycodeTable[0x049] = CT_KEY_KP_9;
    keycodeTable[0x04E] = CT_KEY_KP_ADD;
    keycodeTable[0x053] = CT_KEY_KP_DECIMAL;
    keycodeTable[0x135] = CT_KEY_KP_DIVIDE;
    keycodeTable[0x11C] = CT_KEY_KP_ENTER;
    keycodeTable[0x059] = CT_KEY_KP_EQUAL;
    keycodeTable[0x037] = CT_KEY_KP_MULTIPLY;
    keycodeTable[0x04A] = CT_KEY_KP_SUBTRACT;

    InputManager::OnLoad();
}

Input &WindowsInputManager::GetInput()
{
    return gApp->GetWindow().GetInput();
}