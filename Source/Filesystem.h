#pragma once

#include <Windows.h>
#include <Tree/Node.h>

namespace Filesystem {
	struct Entry {
		size_t size, depth;
		std::filesystem::path path;
	};

	std::vector<std::string> GetLogicalDrives();
	std::pair<ULONGLONG, ULONGLONG> GetDriveSpace(std::string_view driveLetter);

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);
	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);
} // namespace Filesystem

template <>
struct std::formatter<Filesystem::Entry> : std::formatter<std::string_view> {
	auto format(const auto& value, auto& context) const {
		std::string result;

		std::format_to(std::back_inserter(result), "Path= \"{}\" ", value.path.stem().string());
		std::format_to(std::back_inserter(result), "Size= \"{}\" ", value.size);
		std::format_to(std::back_inserter(result), "Depth=\"{}\" ", value.depth);

		return std::formatter<std::string_view>::format(result, context);
	}
};
