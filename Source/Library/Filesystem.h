// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <fmt/format.h>
#include <Tree/Node.h>

namespace Filesystem {
	// Struct representing an entry in the filesystem.
	struct Entry {
		Entry() = default;

		Entry(size_t size, size_t depth, const std::filesystem::path& path)
		    : size(size)
		    , depth(depth)
		    , path(path) {}

		size_t size { 0 };
		size_t depth { 0 };

		std::filesystem::path path;
	};

	// Struct representing volume data.
	struct VolumeData {
		std::string name;
		std::string rootPath;

		size_t bytesTotal { 0 };
		size_t bytesFree { 0 };
	};

	using NodeWrapper = std::reference_wrapper<Tree::Node<Entry>>;

	// Retrieves the data of all available volumes.
	std::vector<VolumeData> GetVolumesData();

	// Opens the system path using the native format.
	void OpenSystemPath(const std::filesystem::path& value);

	std::filesystem::path OpenSelectFolderDialog();

	std::string BytesToString(size_t value);

	// Retrieves the local path for the settings file.
	std::string GetLocalSettingsPath();

	// Build a tree structure representing the file system hierarchy starting from the given path.
	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);

	// Build a tree representing the file system structure in parallel.
	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress);

	// Enumerates the contents of a directory.
	std::queue<NodeWrapper> EnumerateDirectory(Tree::Node<Entry>& node, std::atomic<size_t>& progress);

	// Cancels the build tree operation.
	void CancelBuildTree();

	template <typename DataType>
	std::vector<DataType> ReadFile(std::filesystem::path path) {
		std::ifstream stream(path.native(), std::ios::binary | std::ios::ate);

		const auto size = stream.tellg();
		stream.seekg(0, std::ios::beg);

		std::vector<DataType> result(size);
		stream.read(reinterpret_cast<char*>(result.data()), size);

		return result;
	}
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
