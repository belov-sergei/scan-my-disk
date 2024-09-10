// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

template <typename ObjectType, size_t SegmentSize = 100>
struct MemoryPoolAllocator final {
	MemoryPoolAllocator() = default;

	~MemoryPoolAllocator() {
		// Clean up memory segments.
		std::for_each(memoryPool.begin(), memoryPool.end(), &Delete);
	}

	MemoryPoolAllocator(const MemoryPoolAllocator&) = delete;
	MemoryPoolAllocator& operator=(const MemoryPoolAllocator&) = delete;

	MemoryPoolAllocator(MemoryPoolAllocator&&) = default;
	MemoryPoolAllocator& operator=(MemoryPoolAllocator&&) = default;

	ObjectType* Allocate() {
		if (objectCount % SegmentSize == 0) {
			// Allocate a new memory segment.
			memoryPool.emplace_back(New());
		}

		// Return a pointer to an object in the current segment.
		return &memoryPool.back()[objectCount++ % SegmentSize];
	}

	void Deallocate(ObjectType*) {
		static_assert(false, "Deallocate is not supported.");
	}

private:
	static ObjectType* New() {
		// Allocate memory for one segment.
		return static_cast<ObjectType*>(operator new(sizeof(ObjectType) * SegmentSize));
	}

	static void Delete(ObjectType* memorySegment) {
		// Free the allocated memory segment.
		operator delete(memorySegment);
	}

private:
	size_t objectCount = 0;
	std::vector<ObjectType*> memoryPool;
};
