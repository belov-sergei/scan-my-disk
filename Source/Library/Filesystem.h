// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <fmt/format.h>
#include <Node.h>

namespace Filesystem {
	// Struct representing volume data.
	struct VolumeData {
		std::string name;
		std::string rootPath;

		size_t bytesTotal { 0 };
		size_t bytesFree { 0 };
	};

	// Retrieves the data of all available volumes.
	std::vector<VolumeData> GetVolumesData();

	// Opens the system path using the native format.
	void OpenSystemPath(const std::filesystem::path& value);

	std::string OpenSelectFolderDialog();

	std::string BytesToString(size_t value);

	// Retrieves the local path for the settings file.
	std::string GetLocalSettingsPath();

	bool Exists(std::string path);

	// Build a tree structure representing the file system hierarchy starting from the given path.
	void BuildTree(Node& pathNode, std::atomic<size_t>& progress);

	// Build a tree representing the file system structure in parallel.
	void ParallelBuildTree(Node& pathNode, std::atomic<size_t>& progress);

	// Enumerates the contents of a directory.
	std::vector<Node*> EnumerateDirectory(Node* pathNode, std::atomic<size_t>& progress);

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
struct fmt::formatter<Node> : fmt::formatter<std::string_view> {
	auto format(const Node& value, fmt::format_context& context) const {
		std::string result;

		fmt::format_to(std::back_inserter(result), "Path= \"{}\" ", value.GetFullPath());
		fmt::format_to(std::back_inserter(result), "Size= \"{}\" ", value.GetSize());
		fmt::format_to(std::back_inserter(result), "Depth=\"{}\" ", value.GetDepth());

		return fmt::formatter<std::string_view>::format(result, context);
	}
};
