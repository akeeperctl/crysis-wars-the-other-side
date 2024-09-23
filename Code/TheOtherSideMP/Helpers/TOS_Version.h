/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once
#include <windows.h>
#include <string>
#include <memory>
#include "platform.h"

namespace TOS_Version
{
	inline static string GetDLLVersion(const char* dllFileName)
	{
		std::string version;

		DWORD verHandle = 0;
		UINT size = 0;
		LPBYTE lpBuffer = nullptr;
		LPCSTR lptstrFilename = dllFileName;

		DWORD verSize = GetFileVersionInfoSize(lptstrFilename, &verHandle);
		if (verSize != 0)
		{
			std::unique_ptr<char[]> verData(new char[verSize]);

			if (GetFileVersionInfo(lptstrFilename, verHandle, verSize, verData.get()))
			{
				if (VerQueryValue(verData.get(), "\\", reinterpret_cast<void**>(&lpBuffer), &size))
				{
					if (size)
					{
						VS_FIXEDFILEINFO* verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
						if (verInfo->dwSignature == 0xfeef04bd)
						{
							// Используем std::to_string для форматирования версии
							version = std::to_string((verInfo->dwFileVersionMS >> 16) & 0xffff) + "." +
								std::to_string((verInfo->dwFileVersionMS >> 0) & 0xffff) + "." +
								std::to_string((verInfo->dwFileVersionLS >> 16) & 0xffff) + "." +
								std::to_string((verInfo->dwFileVersionLS >> 0) & 0xffff);
						}
					}
				}
			}
		}

		return string(version.c_str());
	}

}