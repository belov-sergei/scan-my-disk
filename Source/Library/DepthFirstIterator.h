// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "GenericForwardIterator.h"

template <typename NodeType>
class DepthFirstIterator final : public GenericForwardIterator<DepthFirstIterator<NodeType>, NodeType> {
public:
	DepthFirstIterator() noexcept = default;

	DepthFirstIterator(NodeType* startEntry) {
		nodeQueue.emplace_back(startEntry);
	}

	bool Equal(const DepthFirstIterator& otherIterator) const {
		if (nodeQueue.size() > 0 && otherIterator.nodeQueue.size() > 0) {
			return nodeQueue.back() == otherIterator.nodeQueue.back();
		}

		return nodeQueue.size() == otherIterator.nodeQueue.size();
	}

	DepthFirstIterator& Increment() {
		NodeType* node = nodeQueue.back();
		nodeQueue.pop_back();

		for (NodeType* child : node->GetChildren()) {
			nodeQueue.emplace_back(child);
		}

		return *this;
	}

	NodeType& Dereference() const {
		return *nodeQueue.back();
	}

private:
	std::vector<NodeType*> nodeQueue;
};
