#pragma once
#include "Core/Windows/KakaWin.h"
#include "Core/Graphics/Graphics.h"
#include "Core/Input/Keyboard.h"
#include "Core/Input/Mouse.h"
#include <memory>
#include <optional>

namespace Kaka
{
	class Window
	{
	private:
		// Singleton manages registration/cleanup of Window class
		class WindowClass
		{
		public:
			static const wchar_t* GetName();
			static HINSTANCE GetInstance();

		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			static constexpr const wchar_t* WND_CLASS_NAME = L"Kaka";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};

	public:
		Window(int aWidth, int aHeight, const wchar_t* aName);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const WindowClass&) = delete;
		void EnableCursor();
		void DisableCursor();
		bool CursorEnabled() const;
		static std::optional<int> ProcessMessages();
		Graphics& Gfx() const;

	private:
		void ConfineCursor() const;
		void FreeCursor();
		void HideCursor();
		void ShowCursor();
		void EnableImGuiMouse();
		void DisableImGuiMouse();
		static LRESULT WINAPI HandleMsgSetup(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam);
		static LRESULT WINAPI HandleMsgThunk(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam);
		LRESULT HandleMsg(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam);

	public:
		Keyboard keyboard;
		Mouse mouse;

	private:
		bool cursorEnabled = true;
		int width;
		int height;
		HWND hWnd;
		std::vector<BYTE> rawBuffer;
		std::unique_ptr<Graphics> pGfx;
	};
}
