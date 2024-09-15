// Copyright ❤️ 2023-2024, Sergei Belov

#include "DepthFirstIterator.h"
#include "Parallel.h"

#include <Filesystem.h>

namespace Filesystem {
	namespace Detail {
		// Calculate the size of directories based on the size of their contents.
		void CalculateDirectorySizes(Node& rootNode) {
			// Stack to track parent nodes and the count of their remaining children to be processed.
			std::stack<std::pair<Node*, size_t>> parentStack;

			DepthFirstIterator iterator(&rootNode);

			while (iterator) {
				Node& currentNode = *iterator;
				++iterator;

				if (currentNode.HasChildren()) {
					parentStack.emplace(&currentNode, currentNode.GetChildren().size());
				} else if (!parentStack.empty()) {
					Node* first = parentStack.top().first;
					first->SetSize(first->GetSize() + currentNode.GetSize());

					auto accumulatedSize = first->GetSize();
					auto childrenLeft = parentStack.top().second -= 1;

					// If all children of the current parent have been processed, propagate its accumulated size to its own parent.
					while (childrenLeft == 0) {
						// Remove the fully processed parent.
						parentStack.pop();

						// If the stack is empty, we've processed all nodes.
						if (parentStack.empty()) {
							break;
						}

						first = parentStack.top().first;
						first->SetSize(first->GetSize() + accumulatedSize);

						accumulatedSize = first->GetSize();
						childrenLeft = parentStack.top().second -= 1;
					}
				}
			}
		}

		bool CancelFlag = false;
	} // namespace Detail

	void BuildTree(Node& pathNode, std::atomic<size_t>& progress) {
		Detail::CancelFlag = false;

		std::vector<Node*> scanQueue;
		scanQueue.emplace_back(&pathNode);

		while (!scanQueue.empty() && !Detail::CancelFlag) {
			Node* currentNode = scanQueue.back();
			scanQueue.pop_back();

			std::vector<Node*> newTasks = EnumerateDirectory(currentNode, progress);

			while (!newTasks.empty()) {
				scanQueue.emplace_back(newTasks.back());
				newTasks.pop_back();
			}
		}

		Detail::CalculateDirectorySizes(pathNode);
	}

	void ParallelBuildTree(Node& pathNode, std::atomic<size_t>& progress) {
		Detail::CancelFlag = false;

		Parallel::Execute(&EnumerateDirectory, &pathNode, Detail::CancelFlag, progress);
		Detail::CalculateDirectorySizes(pathNode);
	}

	void CancelBuildTree() {
		Detail::CancelFlag = true;
	}
} // namespace Filesystem
