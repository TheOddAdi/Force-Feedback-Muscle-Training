#include <windows.h>
#include <dinput.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <conio.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

LPDIRECTINPUT8 g_directInput = nullptr;
LPDIRECTINPUTDEVICE8 g_wheel = nullptr;
LPDIRECTINPUTEFFECT g_effect = nullptr;

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

    std::wcout << L"Found wheel: "
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
        return false;

    hr = g_directInput->EnumDevices(
        DI8DEVCLASS_GAMECTRL,
        EnumDevicesCallback,
        nullptr,
        DIEDFL_ATTACHEDONLY);

    if (FAILED(hr) || !g_wheel)
        return false;

    hr = g_wheel->SetDataFormat(&c_dfDIJoystick);

    if (FAILED(hr))
        return false;

    hr = g_wheel->SetCooperativeLevel(
        GetConsoleWindow(),
        DISCL_EXCLUSIVE | DISCL_BACKGROUND);

    if (FAILED(hr))
        return false;

    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = FALSE;

    g_wheel->SetProperty(
        DIPROP_AUTOCENTER,
        &dipdw.diph);

    hr = g_wheel->Acquire();

    if (FAILED(hr))
        return false;

    return true;
}

bool ApplyConstantForce(LONG magnitude)
{
    if (g_effect)
    {
        g_effect->Stop();
        g_effect->Release();
        g_effect = nullptr;
    }

    DICONSTANTFORCE cf;
    cf.lMagnitude = magnitude;

    DWORD axes[1] = { DIJOFS_X };
    LONG direction[1] = { 0 };

    DIEFFECT effect;
    ZeroMemory(&effect, sizeof(effect));

    effect.dwSize = sizeof(DIEFFECT);
    effect.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
    effect.dwDuration = INFINITE;
    effect.dwGain = DI_FFNOMINALMAX;
    effect.dwTriggerButton = DIEB_NOTRIGGER;
    effect.cAxes = 1;
    effect.rgdwAxes = axes;
    effect.rglDirection = direction;
    effect.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
    effect.lpvTypeSpecificParams = &cf;

    HRESULT hr = g_wheel->CreateEffect(
        GUID_ConstantForce,
        &effect,
        &g_effect,
        nullptr);

    if (FAILED(hr))
        return false;

    hr = g_effect->Start(1, 0);

    if (FAILED(hr))
        return false;

    return true;
}

void SmoothForceTransition(
    LONG startForce,
    LONG endForce,
    int durationMs,
    int steps)
{
    for (int i = 0; i <= steps; i++)
    {
        double t = static_cast<double>(i) / steps;

        // Smooth sine easing
        double eased =
            0.5 - 0.5 * cos(t * 3.141592653589793);

        LONG currentForce =
            static_cast<LONG>(
                startForce +
                (endForce - startForce) * eased);

        ApplyConstantForce(currentForce);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(durationMs / steps));
    }
}

void StopEffect()
{
    if (g_effect)
    {
        g_effect->Stop();
    }
}

void Cleanup()
{
    StopEffect();

    if (g_effect)
        g_effect->Release();

    if (g_wheel)
    {
        g_wheel->Unacquire();
        g_wheel->Release();
    }

    if (g_directInput)
        g_directInput->Release();
}

int main()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);

    std::cout << "MOZA Force Trainer\n";
    std::cout << "Press any key to stop.\n\n";

    if (!InitializeDirectInput(hInstance))
    {
        std::cout << "Initialization failed.\n";
        system("pause");
        return -1;
    }

    std::cout << "Wheel initialized successfully.\n";
    LONG FORCE_AMOUNT;
	int FORCE_TIME_MS;
	int PAUSE_TIME_MS;
	std::cout << "Enter force magnitude (0 to " << DI_FFNOMINALMAX << "): ";
    std::cin >> FORCE_AMOUNT;
	std::cout << "Enter force application time (ms): ";
	std::cin >> FORCE_TIME_MS;
	std::cout << "Enter pause time between forces (ms): ";
    std::cin >> PAUSE_TIME_MS;

	std::cout << "\nStarting training...\n";
    std::this_thread::sleep_for(
        std::chrono::milliseconds(1000));
	std::cout << "Applying forces. Press any key to stop.\n";

	int TRANSITION_TIME = 250; // Time for smooth transition in ms
	int TRANSITION_STEPS = 150; // Number of steps for smooth transition
    

    bool running = true;

    while (running)
    {
        if (_kbhit())
        {
            running = false;
            break;
        }

        std::cout << "LEFT FORCE\n";

        SmoothForceTransition(
            0,
            -FORCE_AMOUNT,
            TRANSITION_TIME,
            TRANSITION_STEPS);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(FORCE_TIME_MS));

        SmoothForceTransition(
            -FORCE_AMOUNT,
            0,
            TRANSITION_TIME,
            TRANSITION_STEPS);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(PAUSE_TIME_MS));

        if (_kbhit())
        {
            running = false;
            break;
        }

        std::cout << "RIGHT FORCE\n";

        SmoothForceTransition(
            0,
            FORCE_AMOUNT,
            TRANSITION_TIME,
            TRANSITION_STEPS);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(FORCE_TIME_MS));

        SmoothForceTransition(
            FORCE_AMOUNT,
            0,
            TRANSITION_TIME,
            TRANSITION_STEPS);

        std::this_thread::sleep_for(
            std::chrono::milliseconds(PAUSE_TIME_MS));
    }

    Cleanup();

    std::cout << "\nTraining stopped.\n";
    return 0;
}