// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

template <typename UserType>
class Storage final {
	using MutexType = std::shared_mutex;
	using DataType = typename UserType::DataType;

public:
	~Storage() = delete;

	template <typename HandlerType>
	static void Read(HandlerType handler) {
		if (++readCount == 1 && writeCount == 0) {
			mutex.lock_shared();
		}

		handler(std::as_const(storage));

		if (--readCount == 0 && writeCount == 0) {
			mutex.unlock_shared();
		}
	}

	template <typename HandlerType>
	static void Write(HandlerType handler) {
		if (readCount != 0 && writeCount == 0) {
			mutex.unlock_shared();
		}

		if (++writeCount == 1) {
			mutex.lock();
		}

		handler(storage);

		if (--writeCount == 0) {
			mutex.unlock();
		}

		if (readCount != 0 && writeCount == 0) {
			mutex.lock_shared();
		}
	}

private:
	inline static MutexType mutex;
	inline static DataType storage;

	inline static thread_local size_t readCount = 0;
	inline static thread_local size_t writeCount = 0;
};
