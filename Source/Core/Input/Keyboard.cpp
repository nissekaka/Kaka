#include "stdafx.h"
#include "Keyboard.h"

namespace Kaka
{
	bool Keyboard::KeyIsPressed(const unsigned char aKeyCode) const
	{
		return keyStates[aKeyCode];
	}

	std::optional<Keyboard::Event> Keyboard::ReadKey()
	{
		if (!keyBuffer.empty())
		{
			const Event e = keyBuffer.front();
			keyBuffer.pop();
			return e;
		}
		return {};
	}

	bool Keyboard::KeyIsEmpty() const
	{
		return keyBuffer.empty();
	}

	void Keyboard::ClearKey()
	{
		keyBuffer = std::queue<Event>();
	}

	std::optional<char> Keyboard::ReadChar()
	{
		if (!charBuffer.empty())
		{
			const unsigned char charCode = charBuffer.front();
			charBuffer.pop();
			return charCode;
		}
		return {};
	}

	bool Keyboard::CharIsEmpty() const
	{
		return charBuffer.empty();
	}

	void Keyboard::ClearChar()
	{
		charBuffer = std::queue<char>();
	}

	void Keyboard::Clear()
	{
		ClearKey();
		ClearChar();
	}

	void Keyboard::EnableAutorepeat()
	{
		autoRepeatEnabled = true;
	}

	void Keyboard::DisableAutorepeat()
	{
		autoRepeatEnabled = false;
	}

	bool Keyboard::AutorepeatIsEnabled() const
	{
		return autoRepeatEnabled;
	}

	void Keyboard::OnKeyPressed(const unsigned char aKeyCode)
	{
		keyStates[aKeyCode] = true;
		keyBuffer.push(Event(Event::Type::Pressed, aKeyCode));
		TrimBuffer(keyBuffer);
	}

	void Keyboard::OnKeyReleased(const unsigned char aKeyCode)
	{
		keyStates[aKeyCode] = false;
		keyBuffer.push(Event(Event::Type::Released, aKeyCode));
	}

	void Keyboard::OnChar(const char aCharacter)
	{
		charBuffer.push(aCharacter);
		TrimBuffer(charBuffer);
	}

	void Keyboard::ClearKeyStates()
	{
		keyStates.reset();
	}

	template <typename T>
	void Keyboard::TrimBuffer(std::queue<T>& aBuffer)
	{
		while (aBuffer.size() > BUFFER_SIZE)
		{
			aBuffer.pop();
		}
	}
}
