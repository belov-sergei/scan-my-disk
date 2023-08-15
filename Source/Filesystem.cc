#include <Filesystem.h>

namespace Filesystem {
	namespace Details {
		// Calculate the size of directories based on the size of their contents.
		void CalculateDirectorySizes(Tree::Node<Entry>& rootNode) {
			using NodeType = std::remove_reference_t<decltype(rootNode)>;

			// Stack to track parent nodes and the count of their remaining children to be processed.
			std::stack<std::pair<std::reference_wrapper<NodeType>, size_t>> parentStack;

			rootNode.depthTraversal([&](NodeType& currentNode) {
				if (!currentNode.isLeaf()) {
					parentStack.emplace(currentNode, currentNode.getChildCount());
				}
				else if (!parentStack.empty()) {
					auto accumulatedSize = parentStack.top().first.get()->size += currentNode->size;
					auto childrenLeft = parentStack.top().second -= 1;

					// If all children of the current parent have been processed, propagate its accumulated size to its own parent.
					while (childrenLeft == 0) {
						// Remove the fully processed parent.
						parentStack.pop();

						// If the stack is empty, we've processed all nodes.
						if (parentStack.empty()) {
							break;
						}

						accumulatedSize = parentStack.top().first.get()->size += accumulatedSize;
						childrenLeft = parentStack.top().second -= 1;
					}
				}

				return false;
			});
		}
	} // namespace Details

	std::vector<std::string> GetLogicalDrives() {
		std::vector<std::string> logicalDrives;

		DWORD availableDrivesBitmask = ::GetLogicalDrives();
		for (auto driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++) {
			if (availableDrivesBitmask & 1) {
				logicalDrives.emplace_back(std::format("{}://", driveLetter));
			}

			availableDrivesBitmask >>= 1;
		}

		return logicalDrives;
	}

	std::pair<ULONGLONG, ULONGLONG> GetDriveSpace(std::string_view driveLetter) {
		ULARGE_INTEGER bytesTotal, bytesFree;
		::GetDiskFreeSpaceEx(driveLetter.data(), nullptr, &bytesTotal, &bytesFree);

		return std::make_pair(bytesTotal.QuadPart, bytesFree.QuadPart);
	}

	Tree::Node<Entry> BuildTree(const std::filesystem::path& rootPath) {
		Tree::Node<Entry> filesystemTree;

		auto& rootNode = filesystemTree;
		rootNode->path = rootPath;

		std::stack<decltype(&rootNode)> stack;
		stack.emplace(&rootNode);

		std::error_code errorCode;
		while (!stack.empty()) {
			auto* nextNode = stack.top();
			stack.pop();

			for (const auto& entry : std::filesystem::directory_iterator((*nextNode)->path, errorCode)) {
				auto& childNode = nextNode->emplace(entry.file_size(errorCode), entry.path());
				if (std::filesystem::is_directory(entry, errorCode)) {
					stack.emplace(&childNode);
				}
			}
		}

		Details::CalculateDirectorySizes(filesystemTree);

		return filesystemTree;
	}
} // namespace Filesystem
