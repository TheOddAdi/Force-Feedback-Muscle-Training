#include <windows.h>
#include <dinput.h>
#include <iostream>
#include <string>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECTINPUT8 g_directInput = nullptr;
LPDIRECTINPUTDEVICE8 g_wheel = nullptr;

BOOL CALLBACK EnumDevicesCallback(
    const DIDEVICEINSTANCE* pdidInstance,
    VOID* pContext)
{
    HRESULT hr;

    hr = g_directInput->CreateDevice(
        pdidInstance->guidInstance,
        &g_wheel,
        nullptr);

    if (FAILED(hr))
        return DIENUM_CONTINUE;

    std::wcout << "Found wheel: "
        << pdidInstance->tszProductName
        << std::endl;

    return DIENUM_STOP;
}

bool InitializeDirectInput(HINSTANCE hInstance)
{
    HRESULT hr;

    hr = DirectInput8Create(
        hInstance,
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (VOID**)&g_directInput,
        nullptr);

    if (FAILED(hr))
    {
        std::cout << "Failed to initialize DirectInput\n";
        return false;
    }

    hr = g_directInput->EnumDevices(
        DI8DEVCLASS_GAMECTRL,
        EnumDevicesCallback,
        nullptr,
        DIEDFL_ATTACHEDONLY);

    if (FAILED(hr) || !g_wheel)
    {
        std::cout << "No wheel detected\n";
        return false;
    }

    return true;
}

int main()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    std::cout << "MOZA Force Training App\n";
    std::cout << "=======================\n\n";

    if (!InitializeDirectInput(hInstance))
    {
        std::cout << "Initialization failed\n";
        system("pause");
        return -1;
    }

    std::cout << "Wheel initialized successfully\n";

    system("pause");
    return 0;
}