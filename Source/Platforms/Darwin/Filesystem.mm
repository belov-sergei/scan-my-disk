// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include "fmt/xchar.h"

#include <dirent.h>
#include <sys/param.h>
#include <sys/mount.h>

namespace Filesystem {
	namespace Detail {
		std::unordered_set<std::filesystem::path::string_type> Firmlinks = [](){
			std::unordered_set<std::filesystem::path::string_type> links;

			std::ifstream stream("/usr/share/firmlinks");
			if (stream.is_open()) {
				std::string line;
				while (std::getline(stream, line)) {
					if (const size_t delimiter = line.find('\t'); delimiter != std::string::npos) {
						std::filesystem::path::string_type value = {line.begin(), line.begin() + delimiter};
						links.emplace(value);
					}
				}
			}

			return links;
		}();

		bool IsSymlink(const std::filesystem::directory_iterator& iterator, std::error_code& error) {
			auto& links = Detail::Firmlinks;
			return iterator->is_symlink(error) || links.count(iterator->path().native());
		}
	}

	std::vector<std::string> GetLogicalDrives() {
		std::vector<std::string> logicalDrives;

		DIR* volumes = opendir("/Volumes");
		dirent* entry;

		while ((entry = readdir(volumes)) != nullptr) {
			if (entry->d_name[0] == '.') {
				continue;
			}

			logicalDrives.emplace_back(entry->d_name);
		}

		closedir(volumes);

		return logicalDrives;
	}

	std::pair<size_t, size_t> GetDriveSpace(std::string_view driveLetter) {
		struct statfs stats;
		size_t totalBytes = 0;
		size_t freeBytes = 0;

		if (statfs(("/Volumes/" + std::string(driveLetter)).c_str(), &stats) == 0) {
			totalBytes = stats.f_blocks * stats.f_bsize;
			freeBytes = stats.f_bfree * stats.f_bsize;
		}

		return std::make_pair(totalBytes, freeBytes);
	}

	void OpenPath(const std::filesystem::path& value) {
		system(fmt::format("open \"{}\"", value.native()).c_str());
	}

	std::string GetLocalSettingsPath() {
		return fmt::format("{}/{}", getenv("HOME"), "Library/Application Support/Scan My Disk/Settings.xml");
	}	

	std::queue<NodeWrapper> EnumerateDirectory(Tree::Node<Entry>& node, std::atomic<size_t>& progress) {
		std::error_code error;
		std::queue<NodeWrapper> result;

		auto iterator = std::filesystem::directory_iterator(node->path, error);
		const auto end = std::filesystem::end(iterator);

		const auto depth = node->depth + 1;

		size_t total = 0;
		while (iterator != end) {
			if (!error) {
				if (iterator->is_symlink(error) || error) {
					iterator.increment(error);
					continue;
				}

				auto& child = node.emplace(0, depth, iterator->path());

				if (iterator->is_directory(error) && !error) {
					result.emplace(std::ref(child));
				}
				else if (iterator->file_size(error) && !error) {
					child->size = iterator->file_size(error);
					total += child->size;
				}
			}

			iterator.increment(error);
		}

		progress += total;
		
		return result;
	}
}
