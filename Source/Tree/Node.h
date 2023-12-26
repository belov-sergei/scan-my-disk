#pragma once
#include <fmt/format.h>

namespace Tree {
	template <typename ValueType>
	class Node final {
	public:
		Node() = default;

		// Creates a new node, with the condition that the value can be constructed from the provided arguments.
		template <typename... ArgumentTypes>
		requires std::is_constructible_v<ValueType, ArgumentTypes...>
		Node(ArgumentTypes&&... arguments)
			: _value(std::forward<ArgumentTypes>(arguments)...) {}

		// Provide direct access to the underlying value.
		ValueType* operator->() {
			return &_value;
		}

		const ValueType* operator->() const {
			return &_value;
		}

		// Allow the node to be used as if it were the underlying value.
		ValueType& operator*() {
			return _value;
		}

		const ValueType& operator*() const {
			return _value;
		}

		std::forward_list<Node>& getChildNodes() {
			return _children;
		}

		const std::forward_list<Node>& getChildNodes() const {
			return _children;
		}

		size_t getChildCount() const {
			return std::distance(_children.cbegin(), _children.cend());
		}

		// A node is considered a leaf if it has no child nodes.
		bool isLeaf() const {
			return getChildCount() == 0;
		}

		template <typename ValueTypeForward = ValueType>
		Node& insert(ValueTypeForward&& value) {
			*_children.emplace_front() = std::forward<ValueTypeForward>(value);
			return _children.front();
		}

		template <typename... Types>
		Node& emplace(Types&&... values) {
			return _children.emplace_front(std::forward<Types>(values)...);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void depthTraversal(const std::function<bool(Node&)>& callback) {
			DepthTraversalInternal(this, callback);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void depthTraversal(const std::function<bool(const Node&)>& callback) const {
			DepthTraversalInternal(this, callback);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void breadthTraversal(const std::function<bool(Node&)>& callback) {
			BreadthTraversalInternal(this, callback);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void breadthTraversal(const std::function<bool(const Node&)>& callback) const {
			BreadthTraversalInternal(this, callback);
		}

	private:
		template <typename NodeType, typename CallbackType>
		static void DepthTraversalInternal(NodeType* start, CallbackType&& callback) {
			std::stack<NodeType*> pending;
			pending.emplace(start);

			while (!pending.empty()) {
				auto& current = *pending.top();
				pending.pop();

				for (auto& child : current) {
					pending.emplace(&child);
				}

				// This allows for flexible conditions to end the traversal, like stopping after finding a specific node or after a certain number of nodes.
				if (callback(current)) {
					return;
				}
			}
		}

		template <typename NodeType, typename CallbackType>
		static void BreadthTraversalInternal(NodeType* start, CallbackType&& callback) {
			std::deque<NodeType*> pending;
			pending.emplace_back(start);

			while (!pending.empty()) {
				auto& current = *pending.front();
				pending.pop_front();

				for (auto& child : current) {
					pending.emplace_back(&child);
				}

				std::reverse(pending.end() - current.getChildCount(), pending.end());

				// This allows for flexible conditions to end the traversal, like stopping after finding a specific node or after a certain number of nodes.
				if (callback(current)) {
					return;
				}
			}
		}

	private:
		ValueType _value;
		std::forward_list<Node> _children;
	};
} // namespace Tree

// It is prohibited to add declarations or definitions into namespace "std". This could lead to undefined behavior.
namespace std {
	template <typename ValueType>
	auto begin(Tree::Node<ValueType>& node) {
		return node.getChildNodes().begin();
	}

	template <typename ValueType>
	auto end(Tree::Node<ValueType>& node) {
		return node.getChildNodes().end();
	}

	template <typename ValueType>
	auto begin(const Tree::Node<ValueType>& node) {
		return node.getChildNodes().begin();
	}

	template <typename ValueType>
	auto end(const Tree::Node<ValueType>& node) {
		return node.getChildNodes().end();
	}

	template <typename ValueType>
	auto cbegin(Tree::Node<ValueType>& node) {
		return node.getChildNodes().cbegin();
	}

	template <typename ValueType>
	auto cend(Tree::Node<ValueType>& node) {
		return node.getChildNodes().cend();
	}
} // namespace std

template <typename ValueType>
struct fmt::formatter<Tree::Node<ValueType>> : fmt::formatter<std::string_view> {
	auto format(const auto& value, auto& context) const {
		std::stack<decltype(&value)> pending;
		pending.emplace(&value);

		std::string result = "<Tree>";

		while (!pending.empty()) {
			if (pending.top() == nullptr) {
				result += "</Node>";

				pending.pop();
				continue;
			}

			const auto& current = *pending.top();
			pending.pop();

			result += fmt::format("<Node {}>", *current);

			if (!current.isLeaf()) {
				// Close node later, after all children have been processed.
				pending.emplace(nullptr);
			}
			else {
				result += "</Node>";
			}

			for (const auto& child : current) {
				pending.emplace(&child);
			}
		}

		return fmt::formatter<std::string_view>::format(result + "</Tree>", context);
	}
};

template <template <typename> typename NodeType, typename DataType>
requires std::is_same_v<NodeType<DataType>, Tree::Node<DataType>>
bool IsEqual(const NodeType<DataType>& node, std::string xmlString) {
	std::string xmlNodeString = fmt::format("{}", node);

	std::erase_if(xmlNodeString, [](const auto& character) {
		return std::isspace(character);
	});

	std::erase_if(xmlString, [](const auto& character) {
		return std::isspace(character);
	});

	return xmlNodeString == xmlString;
}
