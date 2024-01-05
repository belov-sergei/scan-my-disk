// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include "fmt/xchar.h"

#if defined(WINDOWS)
	#include <Windows.h>
	#include <shlobj_core.h>
#endif

#if defined(MACOS)
	#include <dirent.h>
	#include <sys/param.h>
	#include <sys/mount.h>
#endif

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

		bool CancelFlag = false;
	} // namespace Details

	std::vector<std::string> GetLogicalDrives() {
		std::vector<std::string> logicalDrives;

#if defined(WINDOWS)
		DWORD availableDrivesBitmask = ::GetLogicalDrives();
		for (auto driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++) {
			if (availableDrivesBitmask & 1) {
				logicalDrives.emplace_back(fmt::format("{}:\\", driveLetter));
			}

			availableDrivesBitmask >>= 1;
		}
#else
		DIR* volumes = opendir("/Volumes");
		dirent* entry;

		while ((entry = readdir(volumes)) != nullptr) {
			if (entry->d_name[0] == '.') {
				continue;
			}

			logicalDrives.emplace_back(entry->d_name);
		}

		closedir(volumes);
#endif

		return logicalDrives;
	}

	std::pair<size_t, size_t> GetDriveSpace(std::string_view driveLetter) {
#if defined(WINDOWS)
		ULARGE_INTEGER bytesTotal, bytesFree;
		::GetDiskFreeSpaceEx(driveLetter.data(), nullptr, &bytesTotal, &bytesFree);

		return std::make_pair(bytesTotal.QuadPart, bytesFree.QuadPart);
#else
		struct statfs stats;
		size_t totalBytes = 0;
		size_t freeBytes = 0;

		if (statfs(("/Volumes/" + std::string(driveLetter)).c_str(), &stats) == 0) {
			totalBytes = stats.f_blocks * stats.f_bsize;
			freeBytes = stats.f_bfree * stats.f_bsize;
		}

		return std::make_pair(totalBytes, freeBytes);
#endif
	}

	void OpenPath(const std::filesystem::path& value) {
#if defined(WINDOWS)
		const std::wstring string = fmt::format(L"\"{}\"", value.c_str());
		ShellExecuteW(nullptr, nullptr, string.c_str(), nullptr, nullptr, SW_NORMAL);
#endif

#if defined(MACOS)
		system(fmt::format("open \"{}\"", path).c_str());
#endif
	}

	std::string GetLocalSettingsPath() {
#if defined(WINDOWS)
		char path[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path);

		return fmt::format("{}\\Scan My Disk\\Settings.xml", path);
#endif

#if defined(MACOS)
		return fmt::format("{}/{}", getenv("HOME"), "Library/Application Support/Scan My Disk/Settings.xml");
#endif
	}

	Tree::Node<Entry> BuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress) {
		Details::CancelFlag = false;

		Tree::Node<Entry> root = {0, 0, path};

		std::stack<decltype(&root)> pending;
		pending.emplace(&root);

		std::error_code error;
		while (!pending.empty() && !Details::CancelFlag) {
			auto& node = *pending.top();
			pending.pop();

			auto iterator = std::filesystem::directory_iterator(node->path, error);
			const auto end = std::filesystem::end(iterator);

			const auto depth = node->depth + 1;
			size_t total = 0;
			while (iterator != end) {
				if (!error) {
					if (iterator->is_symlink(error) || error) {
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

		Details::CalculateDirectorySizes(root);

		return root;
	}

	Tree::Node<Entry> ParallelBuildTree(const std::filesystem::path& path, std::atomic<size_t>& progress) {
		Details::CancelFlag = false;

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
				while (!jobs.empty() && !Details::CancelFlag) {
					auto& node = *jobs.top();
					jobs.pop();

					auto iterator = std::filesystem::directory_iterator(node->path, error);
					const auto end = std::filesystem::end(iterator);

					const auto depth = node->depth + 1;
					size_t total = 0;
					while (iterator != end) {
						if (!error) {
							if (iterator->is_symlink(error) || error) {
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

		Details::CalculateDirectorySizes(root);

		return root;
	}

	void CancelBuildTree() {
		Details::CancelFlag = true;
	}
} // namespace Filesystem
