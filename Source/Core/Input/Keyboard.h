#pragma once
#include <bitset>
#include <optional>
#include <queue>


namespace Kaka
{
	class Keyboard
	{
		friend class Window;

	public:
		class Event
		{
		public:
			enum class Type
			{
				Pressed,
				Released,
			};

		private:
			Type type;
			unsigned char keyCode;

		public:
			Event(const Type aType, const unsigned char aKeyCode)
				: type(aType),
				  keyCode(aKeyCode) { }

			~Event() = default;

			bool IsPressed() const
			{
				return type == Type::Pressed;
			}

			bool IsReleased() const
			{
				return type == Type::Released;
			}

			unsigned char GetKeyCode() const
			{
				return keyCode;
			}
		};

	public:
		Keyboard() = default;
		Keyboard(const Keyboard&) = delete;
		~Keyboard() = default;
		Keyboard& operator=(const Keyboard&) = delete;
		// Key event
		bool KeyIsPressed(unsigned char aKeyCode) const;
		std::optional<Event> ReadKey();
		bool KeyIsEmpty() const;
		void ClearKey();
		// Char event
		std::optional<char> ReadChar();
		bool CharIsEmpty() const;
		void ClearChar();
		void Clear();
		// Auto-repeat control
		void EnableAutorepeat();
		void DisableAutorepeat();
		bool AutorepeatIsEnabled() const;

	private:
		void OnKeyPressed(unsigned char aKeyCode);
		void OnKeyReleased(unsigned char aKeyCode);
		void OnChar(char aCharacter);
		void ClearKeyStates();
		template <typename T>
		static void TrimBuffer(std::queue<T>& aBuffer);

	private:
		static constexpr unsigned int NUM_KEYS = 256u;
		static constexpr unsigned int BUFFER_SIZE = 16u;
		bool autoRepeatEnabled = false;
		std::bitset<NUM_KEYS> keyStates;
		std::queue<Event> keyBuffer;
		std::queue<char> charBuffer;
	};
}
