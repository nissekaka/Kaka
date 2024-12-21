#include "stdafx.h"
#include "Window.h"
#include "Game/resource.h"
#include <External/include/imgui/imgui_impl_win32.h>
#include <cassert>
#include <sstream>

namespace Kaka
{
	// Window Class
	Window::WindowClass Window::WindowClass::wndClass;

	const wchar_t* Window::WindowClass::GetName()
	{
		return WND_CLASS_NAME;
	}

	HINSTANCE Window::WindowClass::GetInstance()
	{
		return wndClass.hInst;
	}

	Window::WindowClass::WindowClass() : hInst(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wc = {0};
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 32, 32, 0));
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();
		wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0));
		RegisterClassEx(&wc);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClass(WND_CLASS_NAME, GetInstance());
	}

	Window::Window(const int aWidth, const int aHeight, const wchar_t* aName)
	{
		width = aWidth;
		height = aHeight;

		// Calculate window size based on desired client region size
		RECT wr = {};
		wr.left = 0;
		wr.right = aWidth + wr.left;
		wr.top = 0;
		wr.bottom = aHeight + wr.top;
		AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

		hWnd = CreateWindow(
			WindowClass::GetName(),
			aName,
			WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			wr.right - wr.left,
			wr.bottom - wr.top,
			nullptr,
			nullptr,
			WindowClass::GetInstance(),
			this
		);
		// Check error
		assert(hWnd && "hWnd is null");

		// Show window
		ShowWindow(hWnd, SW_SHOWDEFAULT);

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		width = clientRect.right - clientRect.left;
		height = clientRect.bottom - clientRect.top;

		// Create graphics object
		pGfx = std::make_unique<Graphics>(hWnd, width, height);

		// Initialise ImGui Win32 Impl
		ImGui_ImplWin32_Init(hWnd);

		// Register mouse raw input device
		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = 0x01; // Mouse page
		rid.usUsage = 0x02; // Mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		//MoveWindow(hWnd, 1920, 0, aWidth, aHeight, TRUE);
		//ShowWindow(hWnd, SW_MAXIMIZE);
	}

	Window::~Window()
	{
		ImGui_ImplWin32_Shutdown();
		DestroyWindow(hWnd);
	}

	void Window::EnableCursor()
	{
		cursorEnabled = true;
		ShowCursor();
		EnableImGuiMouse();
		FreeCursor();
	}

	void Window::DisableCursor()
	{
		cursorEnabled = false;
		HideCursor();
		DisableImGuiMouse();
		ConfineCursor();
	}

	bool Window::CursorEnabled() const
	{
		return cursorEnabled;
	}

	std::optional<int> Window::ProcessMessages()
	{
		MSG msg = {};
		// While queue has messages, remove and dispatch them
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// Check for quit because PeekMessage does not signal this via return
			if (msg.message == WM_QUIT)
			{
				// wParam here is the value passed to PostQuitMessage
				return static_cast<int>(msg.wParam);
			}
			TranslateMessage(&msg); // Posts auxilliary WM_CHAR messages from key msg
			DispatchMessage(&msg);
		}

		// Return empty optional
		return {};
	}

	Graphics& Window::Gfx() const
	{
		assert(pGfx && "pGfx is nullptr");
		return *pGfx;
	}

	void Window::ConfineCursor() const
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
	}

	void Window::FreeCursor()
	{
		ClipCursor(nullptr);
	}

	void Window::HideCursor()
	{
		while (::ShowCursor(FALSE) >= 0)
		{
			;
		}
	}

	void Window::ShowCursor()
	{
		while (::ShowCursor(TRUE) < 0)
		{
			;
		}
	}

	void Window::EnableImGuiMouse()
	{
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	void Window::DisableImGuiMouse()
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	LRESULT WINAPI Window::HandleMsgSetup(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam)
	{
		// Use create parameter passed in from CreateWindow() to store window class pointer at WinAPI
		if (aUMsg == WM_NCCREATE)
		{
			// Extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(aLParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
			// Set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(aHWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			// Sett kessage proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr(aHWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
			// Forward message to window class handler
			return pWnd->HandleMsg(aHWnd, aUMsg, aWParam, aLParam);
		}
		// If we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(aHWnd, aUMsg, aWParam, aLParam);
	}

	LRESULT WINAPI Window::HandleMsgThunk(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam)
	{
		// Retrieve ptr to window class
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(aHWnd, GWLP_USERDATA));
		// Forward message to window class handler
		return pWnd->HandleMsg(aHWnd, aUMsg, aWParam, aLParam);
	}

	LRESULT Window::HandleMsg(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(aHWnd, aUMsg, aWParam, aLParam))
		{
			return true;
		}

		switch (aUMsg)
		{
			// We don't want the DefProc to handle this message because
			// we want our destructor to destroy the window, so return 0 instead of break
			case WM_CLOSE:
			{
				PostQuitMessage(0);
				return 0;
			}
			// Clear keystates when window loses focus to prevent input getting stuck
			case WM_KILLFOCUS:
			{
				keyboard.ClearKeyStates();
				break;
			}
			case WM_ACTIVATE:
			{
				// Confine/free cursor on window to foreground/background if cursor disabled
				if (!cursorEnabled)
				{
					if (aWParam & WA_ACTIVE)
					{
						ConfineCursor();
						HideCursor();
					}
					else
					{
						FreeCursor();
						ShowCursor();
					}
				}
				break;
			}
			case WM_SYSCOMMAND:
			{
				//if (aWParam == 61490 ||
				//	aWParam == 61488 ||
				//	aWParam == 61728 ||
				//	aWParam == 61730 ||
				//	aWParam == 61458)
				//{
				//	// Get the client area size of your Win32 window
				//	RECT clientRect;
				//	GetClientRect(aHWnd, &clientRect);
				//	const int clientWidth = clientRect.right - clientRect.left;
				//	const int clientHeight = clientRect.bottom - clientRect.top;

				//	// Update ImGui's window size settings
				//	ImGuiIO& io = ImGui::GetIO();
				//	io.DisplaySize.x = static_cast<float>(clientWidth);
				//	io.DisplaySize.y = static_cast<float>(clientHeight);
				//}
			}
			/********** KEYBOARD MESSAGES **********/
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				// Stifle this keyboard message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureKeyboard)
				{
					break;
				}
				if (!(aLParam & 0x40000000) || keyboard.AutorepeatIsEnabled())
				{
					keyboard.OnKeyPressed(static_cast<unsigned char>(aWParam));
				}
				break;
			}
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				// Stifle this keyboard message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureKeyboard)
				{
					break;
				}
				keyboard.OnKeyReleased(static_cast<unsigned char>(aWParam));
				break;
			}
			case WM_CHAR:
			{
				// Stifle this keyboard message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureKeyboard)
				{
					break;
				}
				keyboard.OnChar(static_cast<char>(aWParam));
				break;
			}
			/********** END KEYBOARD MESSAGES **********/

			/********** MOUSE MESSAGES **********/
			case WM_MOUSEMOVE:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				// In client region --> log move, and log enter + capture mouse
				if (x >= 0 && x < width && y >= 0 && y < height)
				{
					mouse.OnMouseMove(x, y);
					if (!mouse.IsInWindow())
					{
						SetCapture(aHWnd);
						mouse.OnMouseEnter();
					}
				}
				// Not in client --> log move / maintain capture if button down
				else
				{
					if (mouse.LeftIsPressed() || mouse.RightIsPressed() || mouse.MiddleIsPressed())
					{
						mouse.OnMouseMove(x, y);
					}
					// Button up --> release capture / log event for leaving
					else
					{
						ReleaseCapture();
						mouse.OnMouseLeave();
					}
				}
				break;
			}
			case WM_LBUTTONDOWN:
			{
				// Bring window to foreground on left click in client region
				SetForegroundWindow(hWnd);
				if (!cursorEnabled)
				{
					ConfineCursor();
					HideCursor();
				}
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				mouse.OnLeftPressed(x, y);

				break;
			}
			case WM_LBUTTONUP:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				mouse.OnLeftReleased(x, y);
				break;
			}
			case WM_RBUTTONDOWN:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				mouse.OnRightPressed(x, y);
				break;
			}
			case WM_RBUTTONUP:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				mouse.OnRightReleased(x, y);
				break;
			}
			case WM_MBUTTONDOWN:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				mouse.OnMiddlePressed(x, y);
				break;
			}
			case WM_MBUTTONUP:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				mouse.OnMiddleReleased(x, y);
				break;
			}
			case WM_MOUSEWHEEL:
			{
				// Stifle this mouse message if ImGui wants to capture
				if (ImGui::GetIO().WantCaptureMouse)
				{
					break;
				}
				const auto [x, y] = MAKEPOINTS(aLParam);
				const int delta = GET_WHEEL_DELTA_WPARAM(aWParam);
				mouse.OnWheelDelta(x, y, delta);
				break;
			}
			/********** END MOUSE MESSAGES **********/

			/********** RAW MOUSE MESSAGES **********/
			case WM_INPUT:
			{
				if (!mouse.RawEnabled())
				{
					break;
				}
				UINT size = {};
				// First get the size of the input data
				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(aLParam),
					RID_INPUT,
					nullptr,
					&size,
					sizeof(RAWINPUTHEADER)) == -1)
				{
					// Bail msg processing if error
					break;
				}
				rawBuffer.resize(size);
				// Read in the input data
				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(aLParam),
					RID_INPUT,
					rawBuffer.data(),
					&size,
					sizeof(RAWINPUTHEADER)) != size)
				{
					// Bail msg processing if error
					break;
				}
				// Process the raw input data
				auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
				if (ri.header.dwType == RIM_TYPEMOUSE &&
					(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
				{
					mouse.OnRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
				}
				break;
			}
			/********** END RAW MOUSE MESSAGES **********/
		}
		return DefWindowProc(aHWnd, aUMsg, aWParam, aLParam);
	}
}
