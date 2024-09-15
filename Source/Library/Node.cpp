// Copyright ❤️ 2023-2024, Sergei Belov

#include "Node.h"

#include "DepthFirstIterator.h"
#include "HierarchyUpIterator.h"

Node::Node() noexcept = default;

Node::~Node() {
	if (HasParent()) {
		return;
	}

	std::vector<Node*> nodesToDestory;

	DepthFirstIterator iterator(this);
	while (++iterator) {
		nodesToDestory.emplace_back(&*iterator);
	}

	for (auto* node : nodesToDestory) {
		node->~Node();
	}

	ReleaseMemory();
}

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

std::vector<Node*> Node::GetChildren() {
	if (!HasChildren()) {
		return {};
	}

	std::vector<Node*> children = { lastChild };
	while (children.back()->nextChild) {
		Node* child = children.back()->nextChild;
		children.emplace_back(child);
	}

	return children;
}

std::vector<const Node*> Node::GetChildren() const {
	if (!HasChildren()) {
		return {};
	}

	std::vector<const Node*> children = { lastChild };
	while (children.back()->nextChild) {
		const Node* child = children.back()->nextChild;
		children.emplace_back(child);
	}

	return children;
}

size_t Node::GetSize() const {
	return nodeSize;
}

void Node::SetSize(size_t newSize) {
	nodeSize = newSize;
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
