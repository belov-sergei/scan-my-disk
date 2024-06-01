// Copyright ❤️ 2023-2024, Sergei Belov

#include "Parallel.h"

#include <Filesystem.h>

namespace Filesystem {
	namespace Detail {
		// Calculate the size of directories based on the size of their contents.
		void CalculateDirectorySizes(Tree::Node<Entry>& rootNode) {
			using NodeType = std::remove_reference_t<decltype(rootNode)>;

			// Stack to track parent nodes and the count of their remaining children to be processed.
			std::stack<std::pair<std::reference_wrapper<NodeType>, size_t>> parentStack;

			rootNode.depthTraversal([&](NodeType& currentNode) {
				if (!currentNode.isLeaf()) {
					parentStack.emplace(currentNode, currentNode.getChildCount());
				} else if (!parentStack.empty()) {
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

		bool CancelFlag = false;
	} // namespace Detail

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress) {
		Detail::CancelFlag = false;

		Tree::Node<Entry> root = { 0, 0, path };

		std::queue<NodeWrapper> pending;
		pending.emplace(root);

		while (!pending.empty() && !Detail::CancelFlag) {
			NodeWrapper node = pending.front();
			pending.pop();

			std::queue<NodeWrapper> queue = EnumerateDirectory(node, progress);

			if (queue.size() > pending.size()) {
				std::swap(queue, pending);
			}

			while (!queue.empty()) {
				pending.emplace(queue.front());
				queue.pop();
			}
		}

		Detail::CalculateDirectorySizes(root);

		return root;
	}

	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress) {
		Detail::CancelFlag = false;

		Tree::Node<Entry> root = { 0, 0, path };
		Parallel::Execute(&EnumerateDirectory, std::ref(root), Detail::CancelFlag, progress);

		Detail::CalculateDirectorySizes(root);

		return root;
	}

	void CancelBuildTree() {
		Detail::CancelFlag = true;
	}
} // namespace Filesystem
