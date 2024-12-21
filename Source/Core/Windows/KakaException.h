#pragma once
#include <exception>
#include <string>


namespace Kaka
{
	class KakaException : public std::exception
	{
	public:
		KakaException(int aLine, const char* aFile);
		const char* what() const override;
		virtual const char* GetType() const;
		int GetLine() const;
		const std::string& GetFile() const;
		std::string GetOriginString() const;

	private:
		int line;
		std::string file;

	protected:
		mutable std::string whatBuffer;
	};
}
