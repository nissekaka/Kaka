#include "stdafx.h"
#include "KakaException.h"
#include <sstream>

namespace Kaka
{
	KakaException::KakaException(const int aLine, const char* aFile)
		:
		line(aLine),
		file(aFile) {}

	const char* KakaException::what() const
	{
		std::ostringstream oss;
		oss << GetType() << std::endl
			<< GetOriginString();
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* KakaException::GetType() const
	{
		return "Kaka Exception";
	}

	int KakaException::GetLine() const
	{
		return line;
	}

	const std::string& KakaException::GetFile() const
	{
		return file;
	}

	std::string KakaException::GetOriginString() const
	{
		std::ostringstream oss;
		oss << "[File] " << file << std::endl
			<< "[Line] " << line;
		return oss.str();
	}
}
