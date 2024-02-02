// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include "fmt/xchar.h"

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

		bool CancelFlag = false;
	}

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress) {
		Detail::CancelFlag = false;

		Tree::Node<Entry> root = {0, 0, path};

		std::stack<decltype(&root)> pending;
		pending.emplace(&root);

		std::error_code error;
		while (!pending.empty() && !Detail::CancelFlag) {
			auto& node = *pending.top();
			pending.pop();

			auto iterator = std::filesystem::directory_iterator(node->path, error);
			const auto end = std::filesystem::end(iterator);

			const auto depth = node->depth + 1;
			size_t total = 0;
			while (iterator != end) {
				if (!error) {
					if (IsSymlink(iterator, error) || error) {
						iterator.increment(error);
						continue;
					}

					auto& child = node.emplace(0, depth, iterator->path());

					if (iterator->is_directory(error) && !error) {
						pending.emplace(&child);
					}
					else if (iterator->file_size(error) && !error) {
						child->size = iterator->file_size(error);
						total += child->size;
					}
				}

				iterator.increment(error);
			}

			progress += total;
		}

		Detail::CalculateDirectorySizes(root);

		return root;
	}

	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress) {
		Detail::CancelFlag = false;

		Tree::Node<Entry> root = {0, 0, path};
		
		// Shared stack of tasks for all threads to work on.
		std::stack<Tree::Node<Entry>*> pending;
		pending.emplace(&root);

		// Mutex and condition variable for thread synchronization.
		std::mutex mutex;
		std::condition_variable condition;

		// Active worker thread count.
		size_t workers = 0;

		// Worker lambda function to process tasks.
		const auto worker = [&] {
			// For capturing filesystem errors.
			std::error_code error;

			// Local stack of tasks for this thread.
			std::stack<Tree::Node<Entry>*> jobs;

			while (true) {
				{
					std::unique_lock lock(mutex);

					// Wait until there are tasks available or all work is done.
					condition.wait(lock, [&] {
						return !pending.empty() || workers == 0;
					});

					// Exit loop if no tasks are pending and all workers are idle.
					if (pending.empty() && workers == 0) {
						break;
					}

					// Fetch next task from the shared stack.
					jobs.emplace(pending.top());
					pending.pop();

					++workers;
				}

				// Process each job until the local stack is empty or cancellation is signaled.
				while (!jobs.empty() && !Detail::CancelFlag) {
					auto& node = *jobs.top();
					jobs.pop();

					auto iterator = std::filesystem::directory_iterator(node->path, error);
					const auto end = std::filesystem::end(iterator);

					const auto depth = node->depth + 1;
					size_t total = 0;
					while (iterator != end) {
						if (!error) {
							if (IsSymlink(iterator, error) || error) {
								iterator.increment(error);
								continue;
							}

							auto& child = node.emplace(0, depth, iterator->path());

							if (iterator->is_directory(error) && !error) {
								jobs.emplace(&child);
							}
							else if (iterator->file_size(error) && !error) {
								child->size = iterator->file_size(error);
								total += child->size;
							}
						}

						iterator.increment(error);
					}

					progress += total;

					 // If multiple jobs are created, share them with other threads.
					if (jobs.size() > 1) {
						std::lock_guard lock(mutex);

						// Swap stacks if more jobs are created.
						if (jobs.size() > pending.size()) {
							std::swap(jobs, pending);
						}

						// Keep one task and give the rest to other threads.
						size_t size = jobs.size();
						while (size > 1) {
							pending.emplace(jobs.top());
							jobs.pop();

							--size;
						}

						// Notify other threads that new tasks are available.
						condition.notify_all();
					}
				}

				{
					std::lock_guard lock(mutex);

					// If this thread is idle and no more tasks are pending, notify others.
					if (--workers == 0 && pending.empty()) {
						condition.notify_all();
					}
				}
			}
		};

		std::vector<std::thread> threads(std::thread::hardware_concurrency());

		for (auto& thread : threads) {
			thread = std::thread(worker);
		}

		for (auto& thread : threads) {
			thread.join();
		}

		Detail::CalculateDirectorySizes(root);

		return root;
	}

	void CancelBuildTree() {
		Detail::CancelFlag = true;
	}
}
