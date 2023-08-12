#pragma once

namespace Tree
{
	template <typename ValueType>
	class Node
	{
	public:
		Node() = default;

		// Creates a new node, with the condition that the Value can be constructed from the provided argument types.
		template <typename... ArgumentTypes> requires std::is_constructible_v<ValueType, ArgumentTypes...>
		Node(ArgumentTypes&&... arguments) : m_Value(std::forward<ArgumentTypes>(arguments)...)
		{
		}

		// Provide direct access to the underlying value.
		ValueType* operator->() { return &m_Value; }
		const ValueType* operator->() const { return &m_Value; }

		// Allow the node to be used as if it were the underlying value.
		ValueType& operator*() { return m_Value; }
		const ValueType& operator*() const { return m_Value; }

		std::forward_list<Node>& GetChildNodes() { return m_Children; }
		const std::forward_list<Node>& GetChildNodes() const { return m_Children; }

		size_t GetChildCount() const
		{
			return std::distance(m_Children.cbegin(), m_Children.cend());
		}

		// A node is considered a parent if it has one or more child nodes.
		bool IsParent() const
		{
			return GetChildCount() > 0;
		}

		template <typename ValueTypeForward = ValueType>
		Node& Insert(ValueTypeForward&& value)
		{
			*m_Children.emplace_front() = std::forward<ValueTypeForward>(value);
			return m_Children.front();
		}

		template <typename... Types>
		Node& Emplace(Types&&... values)
		{
			return m_Children.emplace_front(std::forward<Types>(values)...);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void DepthFirstTraversal(const std::function<bool(ValueType&)>& callback)
		{
			DepthFirstTraversalInternal(callback);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void DepthFirstTraversal(const std::function<bool(const ValueType&)>& callback) const
		{
			DepthFirstTraversalInternal(callback);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void BreadthFirstTraversal(const std::function<bool(ValueType&)>& callback)
		{
			BreadthFirstTraversalInternal(callback);
		}

		// Executes a user-provided callback function on each visited node. The traversal stops if the callback function returns true.
		void BreadthFirstTraversal(const std::function<bool(const ValueType&)>& callback) const
		{
			BreadthFirstTraversalInternal(callback);
		}

	private:
		template <typename CallbackType>
		void DepthFirstTraversalInternal(CallbackType&& callback)
		{
			std::stack<decltype(this)> processedNodes;
			processedNodes.emplace(this);

			while (!processedNodes.empty())
			{
				auto& nextNode = *processedNodes.top();
				processedNodes.pop();

				for (auto& childNode : nextNode)
				{
					processedNodes.emplace(&childNode);
				}

				// This allows for flexible conditions to end the traversal, like stopping after finding a specific node or after a certain number of nodes.
				if (callback(*nextNode))
				{
					return;
				}
			}
		}

		template <typename CallbackType>
		void BreadthFirstTraversalInternal(CallbackType&& callback)
		{
			std::deque<decltype(this)> processedNodes;
			processedNodes.emplace_back(this);

			while (!processedNodes.empty())
			{
				auto& nextNode = *processedNodes.front();
				processedNodes.pop_front();

				for (auto& childNode : nextNode)
				{
					processedNodes.emplace_back(&childNode);
				}

				const auto childCount = nextNode.GetChildCount();
				std::reverse(processedNodes.end() - childCount, processedNodes.end());

				// This allows for flexible conditions to end the traversal, like stopping after finding a specific node or after a certain number of nodes.
				if (callback(*nextNode))
				{
					return;
				}
			}
		}

	private:
		ValueType m_Value;
		std::forward_list<Node> m_Children;
	};
}

// It is prohibited to add declarations or definitions into namespace "std". This could lead to undefined behavior.
namespace std
{
	template <typename ValueType>
	auto begin(Tree::Node<ValueType>& node)
	{
		return node.GetChildNodes().begin();
	}

	template <typename ValueType>
	auto end(Tree::Node<ValueType>& node)
	{
		return node.GetChildNodes().end();
	}

	template <typename ValueType>
	auto begin(const Tree::Node<ValueType>& node)
	{
		return node.GetChildNodes().begin();
	}

	template <typename ValueType>
	auto end(const Tree::Node<ValueType>& node)
	{
		return node.GetChildNodes().end();
	}

	template <typename ValueType>
	auto cbegin(Tree::Node<ValueType>& node)
	{
		return node.GetChildNodes().cbegin();
	}

	template <typename ValueType>
	auto cend(Tree::Node<ValueType>& node)
	{
		return node.GetChildNodes().cend();
	}
}