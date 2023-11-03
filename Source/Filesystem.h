#pragma once

#include <Windows.h>
#include <Tree/Node.h>

namespace Filesystem {
	struct Entry {
		size_t size, depth;
		std::filesystem::path path;
	};

	std::string ToString(const Entry& entry);

	std::vector<std::string> GetLogicalDrives();
	std::pair<ULONGLONG, ULONGLONG> GetDriveSpace(std::string_view driveLetter);

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);
	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);
} // namespace Filesystem
