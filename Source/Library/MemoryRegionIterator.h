// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "GenericForwardIterator.h"

// This class serves as an iterator for traversing a specific region of memory.
// The iterator is parameterized by a type, which represents the type of value that the iterator points to.
//
// The iterator assumes that the memory region is a 2D region with a specified width and height,
// and it allows traversal through this memory region in a row-major order.
template <typename ValueType>
struct MemoryRegionIterator final : GenericForwardIterator<MemoryRegionIterator<ValueType>, ValueType> {
	MemoryRegionIterator() = default;

	MemoryRegionIterator(int startX, int startY, int regionWidth, int regionHeight, int rowStride, ValueType* memoryStart)
	    : regionWidth(regionWidth)
	    , regionHeight(regionHeight)
	    , rowStride(rowStride)
	    , memoryStart(memoryStart + startY * rowStride + startX) {}

	bool Equal(const MemoryRegionIterator& other) const {
		if (other.memoryStart == nullptr) {
			return currentY * regionWidth + currentX >= regionWidth * regionHeight;
		}

		return &Dereference() == &other.Dereference();
	}

	MemoryRegionIterator& Increment() {
		currentX = (currentX + 1) % regionWidth;
		currentY += currentX == 0;

		return *this;
	}

	ValueType& Dereference() {
		return *(memoryStart + currentY * rowStride + currentX);
	}

	const ValueType& Dereference() const {
		return *(memoryStart + currentY * rowStride + currentX);
	}

private:
	int currentX = 0;
	int currentY = 0;
	int regionWidth = 0;
	int regionHeight = 0;

	// Distance between rows in the memory.
	int rowStride = 0;
	ValueType* memoryStart = nullptr;
};
