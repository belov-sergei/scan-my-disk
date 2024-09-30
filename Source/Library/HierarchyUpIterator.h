// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "GenericForwardIterator.h"

template <typename EntryType>
class HierarchyUpIterator final : public GenericForwardIterator<HierarchyUpIterator<EntryType>, EntryType> {
public:
	HierarchyUpIterator() noexcept = default;

	HierarchyUpIterator(EntryType* startEntry)
	    : currentEntry(startEntry) {}

	bool Equal(const HierarchyUpIterator& otherIterator) const {
		return currentEntry == otherIterator.currentEntry;
	}

	HierarchyUpIterator& Increment() {
		if (currentEntry->HasParent()) {
			currentEntry = &currentEntry->GetParent();
		} else {
			currentEntry = nullptr;
		}

		return *this;
	}

	EntryType& Dereference() const {
		return *currentEntry;
	}

private:
	EntryType* currentEntry = nullptr;
};
