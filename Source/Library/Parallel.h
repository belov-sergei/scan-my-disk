// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Parallel {
	template <typename T>
	using TaskType = std::decay_t<T>;

	template <typename T>
	using QueueType = std::vector<TaskType<T>>;

	template <typename T>
	using OptionalType = std::optional<TaskType<T>>;

	// Execute tasks in parallel using worker threads. A handler, which should have the signature QueueType<T>(T, V...),
	// processes each task and produces a queue of new tasks.
	//
	// Tasks are managed in a shared queue and are distributed among threads. Synchronization occurs via a mutex and a condition variable.
	//
	// Execution can be canceled via the cancel flag.
	template <typename HandlerType, typename TaskType, typename... ValueTypes>
	void Execute(HandlerType&& handler, TaskType&& task, const bool& cancelFlag, ValueTypes&&... values) {
		// Shared queue of tasks for all threads.
		QueueType<TaskType> sharedQueue;
		sharedQueue.emplace_back(std::forward<TaskType>(task));

		// Mutex and condition variable for thread synchronization.
		std::mutex queueMutex;
		std::condition_variable condition;

		// Active worker thread count.
		size_t activeWorkers = 0;

		const auto workerLambda = [&] {
			// Current task of this thread.
			OptionalType<TaskType> currentTask;

			while (true) {
				{
					std::unique_lock scopedLock(queueMutex);

					// Wait until there are tasks available or all work is done.
					condition.wait(scopedLock, [&] {
						return !sharedQueue.empty() || activeWorkers == 0;
					});

					// Exit loop if no tasks are pending and all workers are idle.
					if (sharedQueue.empty() && activeWorkers == 0) {
						break;
					}

					// Fetch next task from the shared queue.
					currentTask = std::move(sharedQueue.back());
					sharedQueue.pop_back();

					++activeWorkers;
				}

				while (currentTask.has_value() && !cancelFlag) {
					// Execute the callback function on the current task and obtain new tasks.
					QueueType<TaskType> newTasks = handler(currentTask.value(), std::forward<ValueTypes>(values)...);
					currentTask.reset();

					if (!newTasks.empty()) {
						// Keeps one task for this thread.
						currentTask = std::move(newTasks.back());
						newTasks.pop_back();
					}

					// If multiple tasks are created, share them with other threads.
					if (!newTasks.empty()) {
						std::unique_lock scopedLock(queueMutex);

						// Merge all tasks into the shared queue.
						while (!newTasks.empty()) {
							sharedQueue.emplace_back(std::move(newTasks.back()));
							newTasks.pop_back();
						}

						// Notify other threads that new tasks are available.
						condition.notify_all();
					}
				}

				{
					std::unique_lock scopedLock(queueMutex);

					// If this thread is idle and no more tasks are pending, notify others.
					if (--activeWorkers == 0 && sharedQueue.empty()) {
						condition.notify_all();
					}
				}
			}
		};

		const size_t kThreadsSize = std::thread::hardware_concurrency() * static_cast<size_t>(2);

		std::vector<std::thread> scanThreads;
		scanThreads.reserve(kThreadsSize);

		for (size_t threadIndex = 0; threadIndex < kThreadsSize; threadIndex++) {
			scanThreads.emplace_back(workerLambda);
		}

		for (size_t threadIndex = 0; threadIndex < kThreadsSize; threadIndex++) {
			scanThreads[threadIndex].join();
		}
	}
} // namespace Parallel
