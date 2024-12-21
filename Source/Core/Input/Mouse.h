#pragma once
#include <queue>
#include <optional>


namespace Kaka
{
	class Mouse
	{
		friend class Window;

	public:
		struct RawDelta
		{
			int x, y;
		};

		class Event
		{
		public:
			enum class Type
			{
				LPressed,
				LReleased,
				RPressed,
				RReleased,
				MPressed,
				MReleased,
				WheelUp,
				WheelDown,
				Move,
				Enter,
				Leave,
			};

		private:
			Type type;
			bool leftIsPressed;
			bool rightIsPressed;
			int x;
			int y;

		public:
			Event(const Type aType, const Mouse& aParent)
				:
				type(aType),
				leftIsPressed(aParent.leftIsPressed),
				rightIsPressed(aParent.rightIsPressed),
				x(aParent.x),
				y(aParent.y) { }

			Type GetType() const
			{
				return type;
			}

			std::pair<int, int> GetPos() const
			{
				return {x, y};
			}

			int GetPosX() const
			{
				return x;
			}

			int GetPosY() const
			{
				return y;
			}

			bool LeftIsPressed() const
			{
				return leftIsPressed;
			}

			bool RightIsPressed() const
			{
				return rightIsPressed;
			}
		};

	public:
		Mouse() = default;
		Mouse(const Mouse&) = delete;
		~Mouse() = default;
		Mouse& operator=(const Mouse&) = delete;
		std::pair<int, int> GetPos() const;
		std::optional<RawDelta> ReadRawDelta();
		int GetPosX() const;
		int GetPosY() const;
		bool IsInWindow() const;
		bool LeftIsPressed() const;
		bool RightIsPressed() const;
		bool MiddleIsPressed() const;
		std::optional<Event> Read();

		bool IsEmpty() const
		{
			return buffer.empty();
		}

		void Clear();
		void EnableRaw();
		void DisableRaw();
		bool RawEnabled() const;

	private:
		void OnMouseMove(int aNewX, int aNewY);
		void OnMouseLeave();
		void OnMouseEnter();
		void OnRawDelta(int aDx, int aDy);
		void OnLeftPressed(int aX, int aY);
		void OnLeftReleased(int aX, int aY);
		void OnRightPressed(int aX, int aY);
		void OnRightReleased(int aX, int aY);
		void OnMiddlePressed(int aX, int aY);
		void OnMiddleReleased(int aX, int aY);
		void OnWheelUp(int aX, int aY);
		void OnWheelDown(int aX, int aY);
		void TrimBuffer();
		void TrimRawInputBuffer();
		void OnWheelDelta(int aX, int aY, int aDelta);

	private:
		static constexpr unsigned int BUFFER_SIZE = 16u;
		int x;
		int y;
		bool leftIsPressed = false;
		bool rightIsPressed = false;
		bool middleIsPressed = false;
		bool isInWindow = false;
		int wheelDeltaCarry = 0;
		bool rawEnabled = false;
		std::queue<Event> buffer;
		std::queue<RawDelta> rawDeltaBuffer;
	};
}
