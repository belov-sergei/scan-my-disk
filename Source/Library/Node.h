// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "ThreadAllocatorPool.h"

class Node final : public ThreadAllocatorPool<Node> {
public:
	Node() noexcept;
	~Node();

	Node(const Node&) noexcept = delete;
	Node& operator=(const Node&) noexcept = delete;

	Node(Node&&) noexcept = delete;
	Node& operator=(Node&&) noexcept = delete;

	bool HasParent() const;
	Node& GetParent();
	const Node& GetParent() const;

	bool HasChildren() const;

	size_t GetSize() const;
	void SetSize(size_t newSize);

	const std::string& GetPath() const;
	void SetPath(std::string newPath);

	size_t GetDepth() const;
	std::string GetFullPath() const;

	Node& CreateChild();

private:
	Node* parentNode = nullptr;

	Node* lastChild = nullptr;
	Node* nextChild = nullptr;

	std::string nodePath;
	std::atomic_size_t nodeSize = 0;
};
