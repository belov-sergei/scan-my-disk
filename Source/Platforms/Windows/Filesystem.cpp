// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include "fmt/xchar.h"

#include <Windows.h>
#include <shlobj_core.h>

namespace Filesystem {
	std::vector<std::string> GetLogicalDrives() {
		std::vector<std::string> logicalDrives;

		DWORD availableDrivesBitmask = ::GetLogicalDrives();
		for (auto driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++) {
			if (availableDrivesBitmask & 1) {
				logicalDrives.emplace_back(fmt::format("{}:\\", driveLetter));
			}

			availableDrivesBitmask >>= 1;
		}

		return logicalDrives;
	}

	std::pair<size_t, size_t> GetDriveSpace(std::string_view driveLetter) {
		ULARGE_INTEGER bytesTotal, bytesFree;
		::GetDiskFreeSpaceEx(driveLetter.data(), nullptr, &bytesTotal, &bytesFree);

		return std::make_pair(bytesTotal.QuadPart, bytesFree.QuadPart);
	}

	void OpenPath(const std::filesystem::path& value) {
		const std::wstring string = fmt::format(L"\"{}\"", value.c_str());
		ShellExecuteW(nullptr, nullptr, string.c_str(), nullptr, nullptr, SW_NORMAL);
	}

	std::string GetLocalSettingsPath() {
		char path[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

		return fmt::format("{}\\Scan My Disk\\Settings.xml", path);
	}

	bool IsSymlink(const std::filesystem::directory_iterator& iterator, std::error_code& error) {
		return iterator->is_symlink(error);
	}
}
