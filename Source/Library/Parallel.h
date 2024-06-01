// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Parallel {
	template <typename T>
	using TaskType = std::decay_t<T>;

	template <typename T>
	using QueueType = std::queue<TaskType<T>>;

	template <typename T>
	using OptionalType = std::optional<TaskType<T>>;

	// Execute tasks in parallel using worker threads. A handler, which should have the signature QueueType<T>(T, V...),
	// processes each task and produces a queue of new tasks.
	//
	// Tasks are managed in a shared queue and are distributed among threads. Synchronization occurs via a mutex and a condition variable.
	//
	// Execution can be canceled via the cancel flag.
	template <typename H, typename T, typename... V>
	void Execute(H&& handler, T&& task, bool& cancel, V&&... values) {
		// Shared queue of tasks for all threads.
		QueueType<T> pending;
		pending.emplace(std::forward<T>(task));

		// Mutex and condition variable for thread synchronization.
		std::mutex mutex;
		std::condition_variable condition;

		// Active worker thread count.
		size_t workers = 0;

		const auto worker = [&] {
			// Current task of this thread.
			OptionalType<T> current;

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

					// Fetch next task from the shared queue.
					current = std::move(pending.front());
					pending.pop();

					++workers;
				}

				while (current.has_value() && !cancel) {
					// Execute the callback function on the current task and obtain new tasks.
					QueueType<T> tasks = handler(current.value(), std::forward<V>(values)...);
					current.reset();

					if (!tasks.empty()) {
						// Keeps one task for this thread.
						current = std::move(tasks.front());
						tasks.pop();
					}

					// If multiple tasks are created, share them with other threads.
					if (!tasks.empty()) {
						std::unique_lock lock(mutex);

						// Swap queues if local queue has more tasks.
						if (tasks.size() > pending.size()) {
							std::swap(tasks, pending);
						}

						// Merge all tasks into the shared queue.
						while (!tasks.empty()) {
							pending.emplace(std::move(tasks.front()));
							tasks.pop();
						}

						// Notify other threads that new tasks are available.
						condition.notify_all();
					}
				}

				{
					std::unique_lock lock(mutex);

					// If this thread is idle and no more tasks are pending, notify others.
					if (--workers == 0 && pending.empty()) {
						condition.notify_all();
					}
				}
			}
		};

		std::vector<std::thread> threads(std::thread::hardware_concurrency() * static_cast<size_t>(2));

		for (auto& thread : threads) {
			thread = std::thread(worker);
		}

		for (auto& thread : threads) {
			thread.join();
		}
	}
} // namespace Parallel
