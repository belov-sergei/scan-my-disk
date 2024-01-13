// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include <fmt/format.h>

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

	void Explore(std::string_view path) {
#if defined(WINDOWS)
		ShellExecute(nullptr, nullptr, path.data(), nullptr, nullptr, SW_NORMAL);
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

#if defined(MACOS)
		std::unordered_set<std::filesystem::path::string_type> links;

		std::ifstream stream("/usr/share/firmlinks");
		if (stream.is_open()) {
			std::string line;
			while (std::getline(stream, line)) {
				if (const size_t delimiter = line.find('\t'); delimiter != std::string::npos) {
					std::filesystem::path::string_type value = {line.begin(), line.begin() + delimiter};
					links.emplace(path.native() + value);
				}
			}
		}
#endif

		// Stack of pending tasks shared among threads.
		std::stack<Tree::Node<Entry>*> pending;
		pending.emplace(&root);

		// Mutex to guard access to the shared task list.
		std::mutex mutex;

		// Semaphore becomes available when a new task is added to the shared list.
		std::binary_semaphore semaphore(1);

		// Number of threads that have tasks.
		size_t workers = 0;

		const auto worker = [&] {
			std::error_code error;

			// List of tasks for the current thread.
			std::stack<Tree::Node<Entry>*> jobs;

			while (true) {
				// Wait for a new task.
				semaphore.acquire();

				{
					std::lock_guard lock(mutex);
					const size_t size = pending.size();

					// If there are no pending tasks and threads are idle, wake them up for termination.
					if (size == 0) {
						if (workers == 0) {
							semaphore.release();
							break;
						}
					}
					// Take one task and wake up a next thread if there are more tasks.
					else {
						jobs.emplace(pending.top());
						pending.pop();

						if (size > 1) {
							semaphore.release();
						}
					}

					// Signal that the thread is busy.
					++workers;
				}

				while (!jobs.empty() && !Details::CancelFlag) {
					auto& node = *jobs.top();
					jobs.pop();

					auto iterator = std::filesystem::directory_iterator(node->path, error);
					const auto end = std::filesystem::end(iterator);

					const auto depth = node->depth + 1;
					size_t total = 0;
					while (iterator != end) {
						if (!error) {
#if defined(MACOS)
							if (links.count(iterator->path().native())) {
								iterator.increment(error);
								continue;
							}
#endif
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

					if (jobs.size() > 1) {
						std::lock_guard lock(mutex);
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

						// Wake up the next thread.
						semaphore.release();
					}
				}

				{
					std::lock_guard lock(mutex);

					// Signal that the thread is idle, and if there are no tasks, wake up a next thread as it might be time to exit.
					if (--workers == 0 && pending.empty()) {
						semaphore.release();
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
