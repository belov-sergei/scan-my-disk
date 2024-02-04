// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <fmt/format.h>
#include <Tree/Node.h>

namespace Filesystem {
	struct Entry {
		Entry() = default;

		Entry(size_t s, size_t d, const std::filesystem::path& p) {
			size = s;
			depth = d;
			path = p;
		}

		size_t size, depth;
		std::filesystem::path path;
	};

	// Represents information about a volume containing filesystem data.
	struct VolumeData {
		std::string name;
		std::string rootPath;
		size_t bytesTotal;
		size_t bytesFree;
	};

	std::vector<VolumeData> GetVolumesData();

	// Opens a file or URL in the default application based on the operating system.
	void OpenSystemPath(const std::filesystem::path& value);

	bool IsSymlink(const std::filesystem::directory_iterator& iterator, std::error_code& error);
	
	std::string GetLocalSettingsPath();

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);
	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);

	void CancelBuildTree();
} // namespace Filesystem

template <>
struct fmt::formatter<Filesystem::Entry> : fmt::formatter<std::string_view> {
	auto format(const Filesystem::Entry& value, fmt::format_context& context) const {
		std::string result;

		fmt::format_to(std::back_inserter(result), "Path= \"{}\" ", value.path.stem().string());
		fmt::format_to(std::back_inserter(result), "Size= \"{}\" ", value.size);
		fmt::format_to(std::back_inserter(result), "Depth=\"{}\" ", value.depth);

		return fmt::formatter<std::string_view>::format(result, context);
	}
};
