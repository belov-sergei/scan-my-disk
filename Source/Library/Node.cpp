// Copyright ❤️ 2023-2024, Sergei Belov

#include "HierarchyUpIterator.h"
#include "Node.h"

Node::Node() noexcept = default;
Node::~Node() = default;

bool Node::HasParent() const {
	return parentNode != nullptr;
}

Node& Node::GetParent() {
	return *parentNode;
}

const Node& Node::GetParent() const {
	return *parentNode;
}

bool Node::HasChildren() const {
	return lastChild != nullptr;
}

size_t Node::GetSize() const {
	return nodeSize;
}

void Node::SetSize(size_t newSize) {
	if (nodeSize != newSize) {
		if (HasParent()) {
			const size_t parentSize = parentNode->GetSize() - nodeSize;
			parentNode->SetSize(parentSize + newSize);
		}

		nodeSize = newSize;
	}
}

const std::string& Node::GetPath() const {
	return nodePath;
}

void Node::SetPath(std::string newPath) {
	nodePath = std::move(newPath);
}

size_t Node::GetDepth() const {
	const HierarchyUpIterator iterator(this);
	return std::distance(iterator, HierarchyUpIterator<const Node>()) - 1;
}

std::string Node::GetFullPath() const {
	std::string pathFull;

	HierarchyUpIterator iterator(this);
	while (iterator) {
		thread_local std::string pathBuffer;
		pathBuffer.clear();

		pathBuffer += iterator->GetPath();
		pathBuffer += "\\";
		pathBuffer += pathFull;

		pathFull = pathBuffer;

		++iterator;
	}

	return pathFull;
}

Node& Node::CreateChild() {
	Node* newEntry = new (Allocate()) Node();
	newEntry->parentNode = this;

	if (HasChildren()) {
		newEntry->nextChild = lastChild;
	}

	return *(lastChild = newEntry);
}
