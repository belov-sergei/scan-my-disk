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

	std::string ToString(const Entry& entry) {
		return std::format(R"(Path="{}" Size="{}" Depth="{}")", entry.path.stem().string(), entry.size, entry.depth);
	}

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

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path) {
		Tree::Node<Entry> root = {0, 0, path};

		std::stack<decltype(&root)> pending;
		pending.emplace(&root);

		std::error_code error;
		while (!pending.empty()) {
			auto& node = *pending.top();
			pending.pop();

			auto iterator = std::filesystem::directory_iterator(node->path, error);
			const auto end = std::filesystem::end(iterator);

			const auto depth = node->depth + 1;
			while (iterator != end) {
				if (!error) {
					const auto size = iterator->file_size(error);
					if (!error) {
						auto& child = node.emplace(size, depth, iterator->path());

						if (iterator->is_directory(error)) {
							pending.emplace(&child);
						}
					}
				}

				iterator.increment(error);
			}
		}

		Details::CalculateDirectorySizes(root);

		return root;
	}
} // namespace Filesystem
