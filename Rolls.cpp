#include <windows.h>
#include <cstring>
#include <string>
#include <iostream>

bool macroRunning = false;

void handleHotkey();
void runMacro();
void copyToClipboard(const std::wstring& text);
void paste();
void wait();

const std::wstring message1 = L"$tu";
const std::wstring message2 = L"$wa ";

const UINT hotKey = VK_F8;

std::wstring GetKeyName(UINT vk);

int main() {
    if (!RegisterHotKey(nullptr, 1, 0, hotKey)) {
        return 1;
    }
    std::wcout << L"Hello world! Mudae Auto Roll hotkey is now active.\n";
    std::wcout << L"Press " << GetKeyName(hotKey);
    std::wcout << L" in the mudae channel when you are ready.\n";
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY) {
            handleHotkey();
        }
    }
    UnregisterHotKey(nullptr, 1);
    return 0;
}

//converts virtual keyboard hotkey into user-readable key name text
std::wstring GetKeyName(UINT vk)
{
    wchar_t name[128] = {};
    UINT scanCode = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    LONG lParam = scanCode << 16;
    //moves the scan code binary into bits 16-23 where GetKeyNameText expects to find it
    if (GetKeyNameTextW(lParam, name, 128) > 0)
    {
        return name;
    }
    return L"'Unrecognizable Hotkey'";
}

//traffic controller
void handleHotkey() {
    if (!macroRunning) {
        runMacro();
    }
}

void runMacro() {
    macroRunning = true;
    //disables hotkey, prevents hotkey queue
    UnregisterHotKey(nullptr, 1);
    copyToClipboard(message1);
    paste();
    wait();
    copyToClipboard(message2);
    for (int i = 0; i < 17; i++) {
        wait();
        paste();
    }
    //reenables hotkey
    RegisterHotKey(nullptr, 1, 0, hotKey);
    macroRunning = false;
}

//clipboard manager
void copyToClipboard(const std::wstring& text) {
    //fail state check for clipboard access
    if (!OpenClipboard(nullptr)) {
        return;
    }
    EmptyClipboard();
    /*
    HGLOBAL GlobalAlloc(UINT uFlags, SIZE_T dwBytes);
    SIZE_T dwBytes = text.size() + 1 reserves space for null terminator \0, then
    multiply that by the size in bytes per character to find the necessary amount
    of bytes allocated to GlobalAlloc
    */
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.size() + 1) * sizeof(wchar_t));
    //fail state check for successful memory allocation
    if (hMem == nullptr) {
        CloseClipboard();
        return;
    }
    /*
    GlobalLock(hMem) obtains a address/pointer to the memory associated with the handle hMem
    static_cast<wchar_t*> tell the compiler that the memory pointed to should be treated as a sequence of wchar_t
    */
    wchar_t* ptr = static_cast<wchar_t*>(GlobalLock(hMem));
    //fail state check for whether GlobalLock successfully gave ptr access to the allocated memory
    if (ptr == nullptr) {
        GlobalFree(hMem);
        CloseClipboard();
        return;
    }
    //memcpy(destination, source, numBytes); copies the string bytes into the memory block associated with hMem
    memcpy(ptr, text.c_str(), (text.size() + 1) * sizeof(wchar_t));
    GlobalUnlock(hMem);
    //sends data to clipboard, and checks for fail state
    if (SetClipboardData(CF_UNICODETEXT, hMem) == nullptr) {
        GlobalFree(hMem);
    }
    CloseClipboard();
}

void paste() {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    // Press Ctrl
    input.ki.dwFlags = 0;
    input.ki.wVk = VK_CONTROL;
    SendInput(1, &input, sizeof(INPUT));
    // Press V
    input.ki.dwFlags = 0;
    input.ki.wVk = 'V';
    SendInput(1, &input, sizeof(INPUT));
    // Release V
    input.ki.wVk = 'V';
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    // Release Ctrl
    input.ki.wVk = VK_CONTROL;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    // Press Enter
    input.ki.wVk = VK_RETURN;
    input.ki.dwFlags = 0;
    SendInput(1, &input, sizeof(INPUT));
    // Release Enter
    input.ki.wVk = VK_RETURN;
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}

void wait() {
    Sleep(800);
}