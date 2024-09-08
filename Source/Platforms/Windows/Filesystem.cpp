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

	std::vector<NodeWrapper> EnumerateDirectory(Tree::Node<Entry>& directoryNode, std::atomic<size_t>& scanProgress) {
		thread_local std::vector<NodeWrapper> newTasks;
		newTasks.clear();

		std::error_code errorCode;
		auto iterator = std::filesystem::directory_iterator(directoryNode->pathFull, errorCode);

		size_t totalSize = 0;
		const size_t currentDepth = directoryNode->depth + 1;

		const auto end = std::filesystem::end(iterator);
		while (iterator != end) {
			if (!errorCode) {
				if (iterator->is_symlink(errorCode) || errorCode) {
					iterator.increment(errorCode);
					continue;
				}

				auto& childNode = directoryNode.emplace(0, currentDepth, iterator->path());

				if (iterator->is_directory(errorCode) && !errorCode) {
					newTasks.emplace_back(std::ref(childNode));
				} else if (iterator->file_size(errorCode) && !errorCode) {
					childNode->size = iterator->file_size(errorCode);
					totalSize += childNode->size;
				}
			}

			iterator.increment(errorCode);
		}

		scanProgress += totalSize;

		return newTasks;
	}

	std::filesystem::path OpenSelectFolderDialog() {
		std::filesystem::path result;

		HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		if (SUCCEEDED(hr)) {
			IFileDialog* pfd;
			if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
				DWORD dwOptions;
				if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
					if (SUCCEEDED(pfd->SetOptions(dwOptions | FOS_PICKFOLDERS))) {
						if (SUCCEEDED(pfd->Show(NULL))) {
							IShellItem* psi;
							if (SUCCEEDED(pfd->GetResult(&psi))) {
								PWSTR pszPath;
								if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
									result = pszPath;
									CoTaskMemFree(pszPath);
								}
								psi->Release();
							}
						}
					}
				}
				pfd->Release();
			}
			CoUninitialize();
		}

		return result;
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
