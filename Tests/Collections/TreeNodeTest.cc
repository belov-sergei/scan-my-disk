#include <gtest/gtest.h>

#include <Tree/Node.h>
#include <Debug/CopyMoveData.h>

using DataType = Debug::CopyMoveData<int>;

std::string RemoveWhitespace(std::string string)
{
	std::erase_if(string, [](const auto& character)
	{
		return std::isspace(character);
	});

	return string;
}

template <typename TreeNodeType>
auto NodeToXml(const TreeNodeType& treeNode)
{
	std::stack<decltype(&treeNode)> stack;
	stack.emplace(&treeNode);

	std::string result = "<Tree>";

	while (!stack.empty())
	{
		if (stack.top() == nullptr)
		{
			result += "</Node>";

			stack.pop();
			continue;
		}

		auto& node = *stack.top();
		stack.pop();

		result += std::format(R"(<Node Moved="{}" Copied="{}" Value="{}">)", node->Moved, node->Copied, node->Value);

		if (node.IsParent())
		{
			// Close node later, after all children have been processed.
			stack.emplace(nullptr);
		}
		else
		{
			result += "</Node>";
		}

		for (const auto& child : node)
		{
			stack.emplace(&child);
		}
	}

	return RemoveWhitespace(result + "</Tree>");
}

TEST(TreeNode, EmptyNodeCreation)
{
	const Tree::Node<DataType> rootNode;
	EXPECT_EQ(NodeToXml(rootNode), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0"></Node>
		</Tree>
	)"));
}

TEST(TreeNode, NodeInsertion)
{
	Tree::Node<DataType> rootNode;

	Debug::CopyMoveData<int> counter;
	counter.Value = 1;

	rootNode.Insert(counter);
	rootNode.Insert(2);

	EXPECT_EQ(NodeToXml(rootNode), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="1" Value="1"></Node>
				<Node Moved="1" Copied="0" Value="2"></Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, NodeEmplacement)
{
	Tree::Node<DataType> rootNode;

	rootNode.Emplace(1);
	rootNode.Emplace(2);

	EXPECT_EQ(NodeToXml(rootNode), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="0" Value="1"></Node>
				<Node Moved="0" Copied="0" Value="2"></Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, ChildNodesEmplacement)
{
	Tree::Node<DataType> rootNode;

	for (const auto& x : {1, 2, 3})
	{
		auto& first = rootNode.Emplace(x);

		for (const auto& y : {4, 5})
		{
			auto& second = first.Emplace(y);
			for (const auto& z : {6, 7, 8})
			{
				second.Emplace(z);
			}
		}
	}

	EXPECT_EQ(NodeToXml(rootNode), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="0" Value="1">
					<Node Moved="0" Copied="0" Value="4">
						<Node Moved="0" Copied="0" Value="6"></Node>
						<Node Moved="0" Copied="0" Value="7"></Node>
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
					<Node Moved="0" Copied="0" Value="5">
						<Node Moved="0" Copied="0" Value="6"></Node>
						<Node Moved="0" Copied="0" Value="7"></Node>
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
				</Node>
				<Node Moved="0" Copied="0" Value="2">
					<Node Moved="0" Copied="0" Value="4">
						<Node Moved="0" Copied="0" Value="6"></Node>
						<Node Moved="0" Copied="0" Value="7"></Node>
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
					<Node Moved="0" Copied="0" Value="5">
						<Node Moved="0" Copied="0" Value="6"></Node>
						<Node Moved="0" Copied="0" Value="7"></Node>
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
				</Node>
				<Node Moved="0" Copied="0" Value="3">
					<Node Moved="0" Copied="0" Value="4">
						<Node Moved="0" Copied="0" Value="6"></Node>
						<Node Moved="0" Copied="0" Value="7"></Node>
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
					<Node Moved="0" Copied="0" Value="5">
						<Node Moved="0" Copied="0" Value="6"></Node>
						<Node Moved="0" Copied="0" Value="7"></Node>
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, DepthFirstTraversal)
{
	Tree::Node<DataType> rootNode;

	rootNode.Emplace(1).Emplace(2).Emplace(3);
	rootNode.Emplace(1).Emplace(2).Emplace(3);
	rootNode.Emplace(1).Emplace(2).Emplace(3);

	int next = 0;
	rootNode.DepthFirstTraversal([&next](DataType& value)
	{
		value.Value = next;
		next += 1;

		return false;
	});

	EXPECT_EQ(NodeToXml(rootNode), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="0" Value="1">
					<Node Moved="0" Copied="0" Value="2">
						<Node Moved="0" Copied="0" Value="3"></Node>
					</Node>
				</Node>
				<Node Moved="0" Copied="0" Value="4">
					<Node Moved="0" Copied="0" Value="5">
						<Node Moved="0" Copied="0" Value="6"></Node>
					</Node>
				</Node>
				<Node Moved="0" Copied="0" Value="7">
					<Node Moved="0" Copied="0" Value="8">
						<Node Moved="0" Copied="0" Value="9"></Node>
					</Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, BreadthFirstTraversal)
{
	Tree::Node<DataType> rootNode;

	rootNode.Emplace(1).Emplace(2).Emplace(3);
	rootNode.Emplace(1).Emplace(2).Emplace(3);
	rootNode.Emplace(1).Emplace(2).Emplace(3);

	int next = 0;
	rootNode.BreadthFirstTraversal([&next](DataType& value)
	{
		value.Value = next;
		next += 1;

		return false;
	});

	EXPECT_EQ(NodeToXml(rootNode), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="0" Value="1">
					<Node Moved="0" Copied="0" Value="4">
						<Node Moved="0" Copied="0" Value="7"></Node>
					</Node>
				</Node>
				<Node Moved="0" Copied="0" Value="2">
					<Node Moved="0" Copied="0" Value="5">
						<Node Moved="0" Copied="0" Value="8"></Node>
					</Node>
				</Node>
				<Node Moved="0" Copied="0" Value="3">
					<Node Moved="0" Copied="0" Value="6">
						<Node Moved="0" Copied="0" Value="9"></Node>
					</Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, NodeDeletion)
{
}

TEST(TreeNode, SubtreeCopying)
{
	Tree::Node<Tree::Node<DataType>> dd;
	auto ff = dd;

	Tree::Node<DataType> rootNode;
	rootNode.Emplace(1).Emplace(2).Emplace(3);

	const auto nodeCopy = rootNode;
	EXPECT_EQ(NodeToXml(nodeCopy), RemoveWhitespace(R"(
		<Tree>
			<Node Moved="0" Copied="1" Value="0">
				<Node Moved="0" Copied="1" Value="1">
					<Node Moved="0" Copied="1" Value="2">
						<Node Moved="0" Copied="1" Value="3">
						</Node>
					</Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, CheckIndependenceCopiedSubtree)
{
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
