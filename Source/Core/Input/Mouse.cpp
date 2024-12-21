#include "stdafx.h"
#include "Mouse.h"
#include "Core/Windows/KakaWin.h"

namespace Kaka
{
	std::pair<int, int> Mouse::GetPos() const
	{
		return {x, y};
	}

	std::optional<Mouse::RawDelta> Mouse::ReadRawDelta()
	{
		if (rawDeltaBuffer.empty())
		{
			return std::nullopt;
		}
		const RawDelta d = rawDeltaBuffer.front();
		rawDeltaBuffer.pop();
		return d;
	}

	int Mouse::GetPosX() const
	{
		return x;
	}

	int Mouse::GetPosY() const
	{
		return y;
	}

	bool Mouse::IsInWindow() const
	{
		return isInWindow;
	}

	bool Mouse::LeftIsPressed() const
	{
		return leftIsPressed;
	}

	bool Mouse::RightIsPressed() const
	{
		return rightIsPressed;
	}

	bool Mouse::MiddleIsPressed() const
	{
		return middleIsPressed;
	}

	std::optional<Mouse::Event> Mouse::Read()
	{
		if (!buffer.empty())
		{
			const Event e = buffer.front();
			buffer.pop();
			return e;
		}
		return {};
	}

	void Mouse::Clear()
	{
		buffer = std::queue<Event>();
	}

	void Mouse::EnableRaw()
	{
		rawEnabled = true;
	}

	void Mouse::DisableRaw()
	{
		rawEnabled = false;
	}

	bool Mouse::RawEnabled() const
	{
		return rawEnabled;
	}

	void Mouse::OnMouseMove(const int aNewX, const int aNewY)
	{
		x = aNewX;
		y = aNewY;

		buffer.push(Event(Event::Type::Move, *this));
		TrimBuffer();
	}

	void Mouse::OnMouseLeave()
	{
		isInWindow = false;
		buffer.push(Event(Event::Type::Leave, *this));
		TrimBuffer();
	}

	void Mouse::OnMouseEnter()
	{
		isInWindow = true;
		buffer.push(Event(Event::Type::Enter, *this));
		TrimBuffer();
	}

	void Mouse::OnRawDelta(const int aDx, const int aDy)
	{
		rawDeltaBuffer.push({aDx, aDy});
		TrimBuffer();
	}

	void Mouse::OnLeftPressed(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		leftIsPressed = true;

		buffer.push(Event(Event::Type::LPressed, *this));
		TrimBuffer();
	}

	void Mouse::OnLeftReleased(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		leftIsPressed = false;

		buffer.push(Event(Event::Type::LReleased, *this));
		TrimBuffer();
	}

	void Mouse::OnRightPressed(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		rightIsPressed = true;

		buffer.push(Event(Event::Type::RPressed, *this));
		TrimBuffer();
	}

	void Mouse::OnRightReleased(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		rightIsPressed = false;

		buffer.push(Event(Event::Type::RReleased, *this));
		TrimBuffer();
	}

	void Mouse::OnMiddlePressed(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		middleIsPressed = true;

		buffer.push(Event(Event::Type::MPressed, *this));
		TrimBuffer();
	}

	void Mouse::OnMiddleReleased(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		middleIsPressed = false;

		buffer.push(Event(Event::Type::MReleased, *this));
		TrimBuffer();
	}

	void Mouse::OnWheelUp(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		buffer.push(Event(Event::Type::WheelUp, *this));
		TrimBuffer();
	}

	void Mouse::OnWheelDown(const int aX, const int aY)
	{
		UNREFERENCED_PARAMETER(aX);
		UNREFERENCED_PARAMETER(aY);
		buffer.push(Event(Event::Type::WheelDown, *this));
		TrimBuffer();
	}

	void Mouse::TrimBuffer()
	{
		{
			while (buffer.size() > BUFFER_SIZE)
			{
				buffer.pop();
			}
		}
	}

	void Mouse::TrimRawInputBuffer()
	{
		while (rawDeltaBuffer.size() > BUFFER_SIZE)
		{
			rawDeltaBuffer.pop();
		}
	}

	void Mouse::OnWheelDelta(const int aX, const int aY, const int aDelta)
	{
		wheelDeltaCarry += aDelta;
		// Generate events for every 120
		while (wheelDeltaCarry >= WHEEL_DELTA)
		{
			wheelDeltaCarry -= WHEEL_DELTA;
			OnWheelUp(aX, aY);
		}
		while (wheelDeltaCarry <= -WHEEL_DELTA)
		{
			wheelDeltaCarry += WHEEL_DELTA;
			OnWheelDown(aX, aY);
		}
	}
}
