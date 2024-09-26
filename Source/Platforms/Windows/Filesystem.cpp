// Copyright ❤️ 2023-2024, Sergei Belov

#include "Common/Encoding.h"
#include "fmt/xchar.h"

#include <Filesystem.h>
#include <Windows.h>

// clang-format off
#include <shellapi.h>
#include <shlobj_core.h>
// clang-format on

namespace Filesystem {
	namespace Detail {
		// Retrieves the name of a volume based on the specified volume path.
		std::string GetVolumeName(std::string_view volume) {
			const std::wstring rootPath = Encoding::MultiByteToWideChar(volume);

			WCHAR buffer[MAX_PATH + 1];
			::GetVolumeInformationW(rootPath.c_str(), buffer, MAX_PATH + 1, nullptr, nullptr, nullptr, nullptr, 0);

			return Encoding::WideCharToMultiByte(buffer);
		}

		// Retrieves the total and free capacity of a given volume.
		std::tuple<std::size_t, std::size_t> GetVolumeCapacity(std::string_view volume) {
			const std::wstring rootPath = Encoding::MultiByteToWideChar(volume);

			ULARGE_INTEGER bytesTotal, bytesFree;
			::GetDiskFreeSpaceExW(rootPath.data(), nullptr, &bytesTotal, &bytesFree);

			return { bytesTotal.QuadPart, bytesFree.QuadPart };
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
	} // namespace Detail

	std::vector<VolumeData> GetVolumesData() {
		std::vector<VolumeData> result;

		DWORD availableDrivesBitmask = ::GetLogicalDrives();
		for (char driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++) {
			if (availableDrivesBitmask & 1) {
				result.emplace_back(Detail::MakeVolumeData(driveLetter));
			}

			availableDrivesBitmask >>= 1;
		}

		return result;
	}

	void OpenSystemPath(const std::filesystem::path& value) {
		const std::wstring& nativeFormat = value.native();

		ShellExecuteW(nullptr, nullptr, fmt::format(L"\"{}\"", nativeFormat).c_str(), nullptr, nullptr, SW_NORMAL);
	}

	std::string GetLocalSettingsPath() {
		WCHAR pathBuffer[MAX_PATH + 1];
		SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, pathBuffer);

		std::wstring result(pathBuffer);
		result += L"\\Scan My Disk\\Settings.xml";

		return Encoding::WideCharToMultiByte(result);
	}

	bool Exists(std::string path) {
		return std::filesystem::exists(Encoding::MultiByteToWideChar(path));
	}

	std::vector<Node*> EnumerateDirectory(Node* pathNode, std::atomic<size_t>& progress) {
		thread_local std::vector<Node*> newTasks;
		newTasks.clear();

		std::error_code errorCode;
		std::wstring fullPath = Encoding::MultiByteToWideChar(pathNode->GetFullPath());
		auto iterator = std::filesystem::directory_iterator(fullPath, errorCode);

		size_t totalSize = 0;

		const auto end = std::filesystem::end(iterator);
		while (iterator != end) {
			if (!errorCode) {
				if (iterator->is_symlink(errorCode) || errorCode) {
					iterator.increment(errorCode);
					continue;
				}

				Node& newNode = pathNode->CreateChild();
				newNode.SetPath(Encoding::WideCharToMultiByte(iterator->path().filename().wstring()));

				if (iterator->is_directory(errorCode) && !errorCode) {
					newTasks.emplace_back(&newNode);
				} else if (iterator->file_size(errorCode) && !errorCode) {
					newNode.SetSize(iterator->file_size(errorCode));
					totalSize += newNode.GetSize();
				}
			}

			iterator.increment(errorCode);
		}

		progress += totalSize;

		return newTasks;
	}

	std::string OpenSelectFolderDialog() {
		std::string result;

		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		IFileDialog* pDialog;
		CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDialog));

		DWORD dwOptions;
		pDialog->GetOptions(&dwOptions);
		pDialog->SetOptions(dwOptions | FOS_PICKFOLDERS);
		pDialog->Show(NULL);

		IShellItem* pShellItem;
		if (SUCCEEDED(pDialog->GetResult(&pShellItem))) {
			PWSTR pwPath;
			if (SUCCEEDED(pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pwPath))) {
				result = Encoding::WideCharToMultiByte(pwPath);
				CoTaskMemFree(pwPath);
			}
			pShellItem->Release();
		}

		pDialog->Release();
		CoUninitialize();

		return result + "\\";
	}

	std::string BytesToString(size_t value) {
		constexpr std::array units = { "B", "KB", "MB", "GB", "TB", "PB" };

		auto size = static_cast<double>(value);

		size_t unit = 0;
		while (size >= 1024 && unit < units.size()) {
			size /= 1024;
			unit++;
		}

		return fmt::format("{:.2f} {}", size, units[unit]);
	}
} // namespace Filesystem
