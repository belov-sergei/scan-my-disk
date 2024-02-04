// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include "fmt/xchar.h"

#include <Windows.h>
#include <shlobj_core.h>

namespace Filesystem {
	namespace Detail {
		// Converts a multi-byte string to a wide character string.
		std::wstring MultiByteToWideChar(std::string_view value) {
			const int bufferSize = ::MultiByteToWideChar(CP_UTF8, 0, value.data(),
				static_cast<int>(value.size()),
				nullptr,
				0
			);

			std::wstring result(bufferSize, 0);

			::MultiByteToWideChar(CP_UTF8, 0, value.data(),
				static_cast<int>(value.size()),
				result.data(),
				static_cast<int>(result.size())
			);

			return result;
		}

		// Converts a wide character string to a multibyte string using UTF-8 encoding.
		std::string WideCharToMultiByte(std::wstring_view value) {
			const int bufferSize = ::WideCharToMultiByte(CP_UTF8, 0, value.data(),
				static_cast<int>(value.size()),
				nullptr,
				0,
				nullptr,
				nullptr
			);

			std::string result(bufferSize, 0);

			::WideCharToMultiByte(CP_UTF8, 0, value.data(),
				static_cast<int>(value.size()),
				result.data(),
				static_cast<int>(result.size()),
				nullptr,
				nullptr
			);

			return result;
		}

		// Retrieves the name of a volume based on the specified volume path.
		std::string GetVolumeName(std::string_view volume) {
			const std::wstring rootPath = MultiByteToWideChar(volume);

			WCHAR buffer[MAX_PATH + 1];
			::GetVolumeInformationW(rootPath.c_str(), buffer, MAX_PATH + 1,
				nullptr,\
				nullptr,
				nullptr,
				nullptr, 0);

			return WideCharToMultiByte(buffer);
		}

		// Retrieves the total and free capacity of a given volume.
		std::tuple<std::size_t, std::size_t> GetVolumeCapacity(std::string_view volume) {
			const std::wstring rootPath = MultiByteToWideChar(volume);

			ULARGE_INTEGER bytesTotal, bytesFree;
			::GetDiskFreeSpaceExW(rootPath.data(), nullptr, &bytesTotal, &bytesFree);

			return {bytesTotal.QuadPart, bytesFree.QuadPart};
		}

		// Creates a VolumeData object for the specified drive letter.
		VolumeData MakeVolumeData(char driveLetter) {
			VolumeData result;
			result.rootPath = fmt::format("{}:\\", driveLetter);
			result.name = GetVolumeName(result.rootPath);

			const auto& [bytesTotal, bytesFree] = GetVolumeCapacity(result.rootPath);
			result.bytesTotal = bytesTotal;
			result.bytesFree = bytesFree;

			return result;
		}
	}

	// Retrieves information about the available volumes on the system.
	std::vector<VolumeData> GetVolumesData() {
		std::vector<VolumeData> result;

		DWORD availableDrivesBitmask = ::GetLogicalDrives();
		for (char driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++) {
			if (availableDrivesBitmask & 1) {
				result.emplace_back(
					Detail::MakeVolumeData(driveLetter)
				);
			}

			availableDrivesBitmask >>= 1;
		}

		return result;
	}

	// Opens the system path specified by the given value.
	void OpenSystemPath(const std::filesystem::path& value) {
		const std::wstring& nativeFormat = value.native();
		
		ShellExecuteW(nullptr, nullptr,
			fmt::format(L"\"{}\"", nativeFormat).c_str(),
			nullptr,
			nullptr,
			SW_NORMAL
		);
	}

	// Retrieves the local settings path.
	std::string GetLocalSettingsPath() { 
		WCHAR pathBuffer[MAX_PATH + 1];
		SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, pathBuffer);

		std::wstring result(pathBuffer);
		result += L"\\Scan My Disk\\Settings.xml";
		
		return Detail::WideCharToMultiByte(result);
	}

	bool IsSymlink(const std::filesystem::directory_iterator& iterator, std::error_code& error) {
		return iterator->is_symlink(error);
	}
}
