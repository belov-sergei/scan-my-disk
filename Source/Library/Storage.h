// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

template <typename UserType>
class Storage final {
	using MutexType = std::shared_mutex;
	using DataType = typename UserType::DataType;

public:
	~Storage() = delete;
	
	template <typename HandlerType>
	static void Read(HandlerType& handler) {
		std::shared_lock lock(mutex);
		handler(storage);
	}

	template <typename HandlerType>
	static void Write(HandlerType& handler) {
		std::unique_lock lock(mutex);
		handler(storage);
	}

private:
	inline static MutexType mutex;
	inline static DataType storage;
};
