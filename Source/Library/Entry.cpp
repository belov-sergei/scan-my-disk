// Copyright ❤️ 2023-2024, Sergei Belov

#include "Entry.h"

#include "HierarchyUpIterator.h"

Entry::Entry() noexcept = default;
Entry::~Entry() = default;

bool Entry::HasParent() const {
	return parent != nullptr;
}

Entry& Entry::GetParent() {
	return *parent;
}

const Entry& Entry::GetParent() const {
	return *parent;
}

bool Entry::HasChildren() const {
	return lastChild != nullptr;
}

size_t Entry::GetSize() const {
	return size;
}

void Entry::SetSize(size_t newSize) {
	if (size != newSize) {
		if (HasParent()) {
			const size_t parentSize = parent->GetSize() - size;
			parent->SetSize(parentSize + newSize);
		}

		size = newSize;
	}
}

const std::string& Entry::GetPath() const {
	return path;
}

void Entry::SetPath(std::string newPath) {
	path = std::move(newPath);
}

size_t Entry::GetDepth() const {
	const HierarchyUpIterator iterator(this);
	return std::distance(iterator, HierarchyUpIterator<const Entry>()) - 1;
}

std::string Entry::GetFullPath() const {
	std::string resultPath;

	HierarchyUpIterator iterator(this);
	while (iterator) {
		thread_local std::string pathBuffer;
		pathBuffer.clear();

		pathBuffer += iterator->GetPath();
		pathBuffer += "\\";
		pathBuffer += resultPath;

		resultPath = pathBuffer;

		++iterator;
	}

	return resultPath;
}

Entry& Entry::CreateChild() {
	Entry* newEntry = allocator.Allocate();
	newEntry->parent = this;

	if (HasChildren()) {
		newEntry->nextChild = lastChild;
	}

	return *(lastChild = newEntry);
}
