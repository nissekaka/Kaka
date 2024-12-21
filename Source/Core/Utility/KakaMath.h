#pragma once
#include <math.h>

namespace Kaka
{
	constexpr float PI = 3.14159265f;
	constexpr double PI_D = 3.1415926535897932;

	template <typename T>
	constexpr auto Square(const T& aValue) noexcept
	{
		return aValue * aValue;
	}

	template <typename T>
	T WrapAngle(T aTheta) noexcept
	{
		constexpr T twoPi = static_cast<T>(2) * static_cast<T>(PI_D);
		const T mod = static_cast<T>(fmod(aTheta, twoPi));
		if (mod > static_cast<T>(PI_D))
		{
			return mod - twoPi;
		}
		else if (mod < -static_cast<T>(PI_D))
		{
			return mod + twoPi;
		}
		return mod;
	}

	template <typename T>
	constexpr T Interp(const T& aCurrent, const T& aTarget, float aAlpha) noexcept
	{
		return aCurrent + (aTarget - aCurrent) * aAlpha;
	}

	template <typename T>
	constexpr T DegToRad(T aDegrees) noexcept
	{
		return aDegrees * PI / static_cast<T>(180.0);
	}

	template <typename T>
	constexpr T RadToDeg(T aRad) noexcept
	{
		return aRad / PI * static_cast<T>(180.0);
	}

	template <typename T>
	constexpr T Gauss(T aX, T aSigma) noexcept
	{
		const auto ss = Square(aSigma);
		return (static_cast<T>(1.0) / sqrt(static_cast<T>(2.0) * static_cast<T>(PI_D) * ss)) * exp(
			-Square(aX) / (static_cast<T>(2.0) * ss));
	}
}
