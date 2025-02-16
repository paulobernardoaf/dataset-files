
































#pragma once

#if defined(PCH)
#include PCH
#endif

#include <chrono>
#include <memory>
#include <wrl.h>


#include <EGL/eglplatform.h>

#include "raylib.h"
#include "utils.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Devices::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Gaming::Input;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Platform;

extern "C" { EGLNativeWindowType handle; };








#define MAX_GAMEPADS 4 
#define MAX_GAMEPAD_BUTTONS 32 
#define MAX_GAMEPAD_AXIS 8 


bool cursorLocked = false;
Vector2 mouseDelta = {0, 0};


CoreCursor ^regularCursor = ref new CoreCursor(CoreCursorType::Arrow, 0); 


ref class BaseApp : public Windows::ApplicationModel::Core::IFrameworkView
{
public:


virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView)
{


applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &BaseApp::OnActivated);





CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &BaseApp::OnResuming);
}

virtual void SetWindow(Windows::UI::Core::CoreWindow^ window)
{
window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &BaseApp::OnWindowSizeChanged);
window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &BaseApp::OnVisibilityChanged);
window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &BaseApp::OnWindowClosed);

window->PointerPressed += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &BaseApp::PointerPressed);
window->PointerWheelChanged += ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &BaseApp::PointerWheelChanged);
window->KeyDown += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &BaseApp::OnKeyDown);
window->KeyUp += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &BaseApp::OnKeyUp);

Windows::Devices::Input::MouseDevice::GetForCurrentView()->MouseMoved += ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &BaseApp::MouseMoved);

DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &BaseApp::OnDpiChanged);
currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &BaseApp::OnOrientationChanged);



handle = (EGLNativeWindowType)window;

InitWindow(width, height, NULL);
}

virtual void Load(Platform::String^ entryPoint) {}

void Setup(int width, int height)
{

this->width = width;
this->height = height;
}

virtual void Run()
{

DisplayInformation^ dInfo = DisplayInformation::GetForCurrentView();
Vector2 screenSize = { dInfo->ScreenWidthInRawPixels, dInfo->ScreenHeightInRawPixels };


UWPMessage *msg = CreateUWPMessage();
msg->type = UWP_MSG_SET_DISPLAY_DIMS;
msg->paramVector0 = screenSize;
UWPSendMessage(msg);


using clock = std::chrono::high_resolution_clock;
auto timeStart = clock::now();


if (GetFPS() <= 0) SetTargetFPS(60);

while (!mWindowClosed)
{
if (mWindowVisible)
{

auto delta = clock::now() - timeStart;

UWPMessage *timeMsg = CreateUWPMessage();
timeMsg->type = UWP_MSG_SET_GAME_TIME;
timeMsg->paramDouble0 = std::chrono::duration_cast<std::chrono::seconds>(delta).count();
UWPSendMessage(timeMsg);


Update();

PollInput();

CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
}
else
{
CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
}
}

CloseWindow();
}


virtual void Update() {}

virtual void Uninitialize() {}

protected:


void PollInput()
{

{

while (UWPHasMessages())
{

auto msg = UWPGetMessage();


switch(msg->type)
{
case UWP_MSG_SHOW_MOUSE: 
case UWP_MSG_UNLOCK_MOUSE:
{
CoreWindow::GetForCurrentThread()->PointerCursor = regularCursor;
cursorLocked = false;
MoveMouse(GetMousePosition());
break;
}
case UWP_MSG_HIDE_MOUSE: 
case UWP_MSG_LOCK_MOUSE:
{
CoreWindow::GetForCurrentThread()->PointerCursor = nullptr;
cursorLocked = true;
break;
}
case UWP_MSG_SET_MOUSE_LOCATION:
{
MoveMouse(msg->paramVector0);
break;
}
}


DeleteUWPMessage(msg);
}
}


{
for (int k = 0x08; k < 0xA6; k++) {
auto state = CoreWindow::GetForCurrentThread()->GetKeyState((Windows::System::VirtualKey) k);

#if defined(HOLDHACK)


if (KeyboardStateHack[k] == 2)
{
if ((state & CoreVirtualKeyStates::None) == CoreVirtualKeyStates::None)
{
KeyboardStateHack[k] = 3;
}
}
else if (KeyboardStateHack[k] == 3)
{
if ((state & CoreVirtualKeyStates::None) == CoreVirtualKeyStates::None)
{
KeyboardStateHack[k] = 4;
}
}
else if (KeyboardStateHack[k] == 4)
{
if ((state & CoreVirtualKeyStates::None) == CoreVirtualKeyStates::None)
{

KeyboardStateHack[k] = 0;


RegisterKey(k, 0);
}
}
#endif





if (k == 0xA4 || k == 0xA5)
{
if ((state & CoreVirtualKeyStates::Down) == CoreVirtualKeyStates::Down)
{
RegisterKey(k, 1);
}
else
{
RegisterKey(k, 0);
}
}
}
}


{

if (CurrentPointerID > -1)
{
auto point = PointerPoint::GetCurrentPoint(CurrentPointerID);
auto props = point->Properties;

if (props->IsLeftButtonPressed)
{
RegisterClick(MOUSE_LEFT_BUTTON, 1);
}
else
{
RegisterClick(MOUSE_LEFT_BUTTON, 0);
}

if (props->IsRightButtonPressed)
{
RegisterClick(MOUSE_RIGHT_BUTTON, 1);
}
else
{
RegisterClick(MOUSE_RIGHT_BUTTON, 0);
}

if (props->IsMiddleButtonPressed)
{
RegisterClick(MOUSE_MIDDLE_BUTTON, 1);
}
else
{
RegisterClick(MOUSE_MIDDLE_BUTTON, 0);
}
}

CoreWindow ^window = CoreWindow::GetForCurrentThread();

if (cursorLocked)
{

auto curMousePos = GetMousePosition();

auto x = curMousePos.x + mouseDelta.x;
auto y = curMousePos.y + mouseDelta.y;

UpdateMousePosition({ x, y });




Vector2 centerClient = { (float)(GetScreenWidth() / 2), (float)(GetScreenHeight() / 2) };
window->PointerPosition = Point(centerClient.x + window->Bounds.X, centerClient.y + window->Bounds.Y);
}
else
{

auto x = window->PointerPosition.X - window->Bounds.X;
auto y = window->PointerPosition.Y - window->Bounds.Y;

UpdateMousePosition({ x, y });
}

mouseDelta = { 0 ,0 };
}


{

for (int i = 0; i < MAX_GAMEPADS; i++)
{




UWPMessage* msg = CreateUWPMessage();
msg->type = UWP_MSG_SET_GAMEPAD_ACTIVE;
msg->paramInt0 = i;
msg->paramBool0 = i < Gamepad::Gamepads->Size;
UWPSendMessage(msg);
}


for (int i = 0; i < MAX_GAMEPADS; i++)
{
if (IsGamepadAvailable(i))
{

auto gamepad = Gamepad::Gamepads->GetAt(i);
GamepadReading reading = gamepad->GetCurrentReading();


RegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_DOWN, ((reading.Buttons & GamepadButtons::A) == GamepadButtons::A));
RegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, ((reading.Buttons & GamepadButtons::B) == GamepadButtons::B));
RegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_LEFT, ((reading.Buttons & GamepadButtons::X) == GamepadButtons::X));
RegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_FACE_UP, ((reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y));

RegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_TRIGGER_1, ((reading.Buttons & GamepadButtons::LeftShoulder) == GamepadButtons::LeftShoulder));
RegisterGamepadButton(i, GAMEPAD_BUTTON_RIGHT_TRIGGER_1, ((reading.Buttons & GamepadButtons::RightShoulder) == GamepadButtons::RightShoulder));

RegisterGamepadButton(i, GAMEPAD_BUTTON_MIDDLE_LEFT, ((reading.Buttons & GamepadButtons::View) == GamepadButtons::View)); 
RegisterGamepadButton(i, GAMEPAD_BUTTON_MIDDLE_RIGHT, ((reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu)); 

RegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_UP, ((reading.Buttons & GamepadButtons::DPadUp) == GamepadButtons::DPadUp));
RegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_RIGHT, ((reading.Buttons & GamepadButtons::DPadRight) == GamepadButtons::DPadRight));
RegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_DOWN, ((reading.Buttons & GamepadButtons::DPadDown) == GamepadButtons::DPadDown));
RegisterGamepadButton(i, GAMEPAD_BUTTON_LEFT_FACE_LEFT, ((reading.Buttons & GamepadButtons::DPadLeft) == GamepadButtons::DPadLeft));
RegisterGamepadButton(i, GAMEPAD_BUTTON_MIDDLE, false); 


RegisterGamepadAxis(i, GAMEPAD_AXIS_LEFT_X, (float)reading.LeftThumbstickX);
RegisterGamepadAxis(i, GAMEPAD_AXIS_LEFT_Y, (float)reading.LeftThumbstickY);
RegisterGamepadAxis(i, GAMEPAD_AXIS_RIGHT_X, (float)reading.RightThumbstickX);
RegisterGamepadAxis(i, GAMEPAD_AXIS_RIGHT_Y, (float)reading.RightThumbstickY);
RegisterGamepadAxis(i, GAMEPAD_AXIS_LEFT_TRIGGER, (float)reading.LeftTrigger);
RegisterGamepadAxis(i, GAMEPAD_AXIS_RIGHT_TRIGGER, (float)reading.RightTrigger);
}
}
}
}


void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
{

CoreWindow::GetForCurrentThread()->Activate();
}

void OnResuming(Platform::Object^ sender, Platform::Object^ args) {}


void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWP_MSG_HANDLE_RESIZE;
UWPSendMessage(msg);
}

void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
{
mWindowVisible = args->Visible;
}

void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args)
{
mWindowClosed = true;
}


void OnDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args) {}
void OnOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args) {}


void PointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{

CurrentPointerID = args->CurrentPoint->PointerId;
args->Handled = true;
}

void PointerWheelChanged(Windows::UI::Core::CoreWindow ^sender, Windows::UI::Core::PointerEventArgs^ args)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWP_MSG_SCROLL_WHEEL_UPDATE;
msg->paramFloat0 = args->CurrentPoint->Properties->MouseWheelDelta;
UWPSendMessage(msg);
}

void MouseMoved(Windows::Devices::Input::MouseDevice^ mouseDevice, Windows::Devices::Input::MouseEventArgs^ args)
{
mouseDelta.x += args->MouseDelta.X;
mouseDelta.y += args->MouseDelta.Y;
}

void OnKeyDown(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args)
{
#if defined(HOLDHACK)

KeyboardStateHack[(int)args->VirtualKey] = 1;
#endif

RegisterKey((int)args->VirtualKey, 1);
}

void OnKeyUp(Windows::UI::Core::CoreWindow ^ sender, Windows::UI::Core::KeyEventArgs ^ args)
{
#if defined(HOLDHACK)

if (KeyboardStateHack[(int)args->VirtualKey] == 1)
{
KeyboardStateHack[(int)args->VirtualKey] = 2;
}
else if (KeyboardStateHack[(int)args->VirtualKey] == 2)
{
KeyboardStateHack[(int)args->VirtualKey] = 3;
}
else if (KeyboardStateHack[(int)args->VirtualKey] == 3)
{
KeyboardStateHack[(int)args->VirtualKey] = 4;
}
else if (KeyboardStateHack[(int)args->VirtualKey] == 4)
{
RegisterKey((int)args->VirtualKey, 0);
KeyboardStateHack[(int)args->VirtualKey] = 0;
}
#else

RegisterKey((int)args->VirtualKey, 0);
#endif
}

private:

void RegisterKey(int key, char status)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWPMessageType::UWP_MSG_REGISTER_KEY;
msg->paramInt0 = key;
msg->paramChar0 = status;
UWPSendMessage(msg);
}

void MoveMouse(Vector2 pos)
{
CoreWindow ^window = CoreWindow::GetForCurrentThread();
Point mousePosScreen = Point(pos.x + window->Bounds.X, pos.y + window->Bounds.Y);
window->PointerPosition = mousePosScreen;
}

void RegisterGamepadButton(int gamepad, int button, char status)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWP_MSG_SET_GAMEPAD_BUTTON;
msg->paramInt0 = gamepad;
msg->paramInt1 = button;
msg->paramChar0 = status;
UWPSendMessage(msg);
}

void RegisterGamepadAxis(int gamepad, int axis, float value)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWP_MSG_SET_GAMEPAD_AXIS;
msg->paramInt0 = gamepad;
msg->paramInt1 = axis;
msg->paramFloat0 = value;
UWPSendMessage(msg);
}

void UpdateMousePosition(Vector2 pos)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWP_MSG_UPDATE_MOUSE_LOCATION;
msg->paramVector0 = pos;
UWPSendMessage(msg);
}

void RegisterClick(int button, char status)
{
UWPMessage* msg = CreateUWPMessage();
msg->type = UWPMessageType::UWP_MSG_REGISTER_CLICK;
msg->paramInt0 = button;
msg->paramChar0 = status;
UWPSendMessage(msg);
}

bool mWindowClosed = false;
bool mWindowVisible = true;

int width = 640;
int height = 480;

int CurrentPointerID = -1;

#if defined(HOLDHACK)
char KeyboardStateHack[0xA6]; 
#endif
};


template<typename AppType>
ref class ApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
{
return ref new AppType();
}
};