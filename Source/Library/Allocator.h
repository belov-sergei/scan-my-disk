// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

template <typename EntryType, size_t BlockSize = 100>
struct Allocator {
	EntryType* Allocate() {
		if (objectCount % BlockSize == 0) {
			memoryPool.emplace_back(new EntryType[BlockSize]);
		}

		EntryType* pointer = &memoryPool[objectCount / BlockSize][objectCount % BlockSize];
		objectCount++;

		return pointer;
	}

private:
	size_t objectCount = 0;
	std::vector<std::unique_ptr<EntryType[]>> memoryPool;
};
