// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "MemoryPoolAllocator.h"

template <typename ObjectType>
class ThreadAllocatorPool {
public:
	static void ReleaseMemory() {
		std::unique_lock scopedLock(poolMutex);
		pool.clear();
	}

	ObjectType* Allocate() {
		thread_local MemoryPoolAllocator<ObjectType> allocator;

		thread_local struct AllocatorFinalizer {
			~AllocatorFinalizer() {
				std::unique_lock scopedLock(poolMutex);
				pool.emplace_back(std::move(allocator));
			}
		} finalizer;

		return allocator.Allocate();
	}

private:
	inline static std::mutex poolMutex;
	inline static std::vector<MemoryPoolAllocator<ObjectType>> pool;
};
