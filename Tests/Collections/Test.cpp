// Copyright ❤️ 2023-2024, Sergei Belov

#include <gtest/gtest.h>

#include <Debug/CopyMoveData.h>
#include <Tree/Node.h>

using DataType = Debug::CopyMoveData<int>;

TEST(TreeNode, EmptyNodeCreation) {
	const Tree::Node<Debug::CopyMoveData<int>> rootNode;

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0"></Node>
		</Tree>
	)"));

	EXPECT_FALSE(IsEqual(rootNode, R"(
		<Tree></Tree>
	)"));
}

TEST(TreeNode, NodeInsertion) {
	Tree::Node<DataType> rootNode;

	Debug::CopyMoveData<int> counter;
	counter.value = 1;

	rootNode.insert(counter);
	rootNode.insert(2);

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="1" Value="1"></Node>
				<Node Moved="1" Copied="0" Value="2"></Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, NodeEmplacement) {
	Tree::Node<DataType> rootNode;

	rootNode.emplace(1);
	rootNode.emplace(2);

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Moved="0" Copied="0" Value="0">
				<Node Moved="0" Copied="0" Value="1"></Node>
				<Node Moved="0" Copied="0" Value="2"></Node>
			</Node>
		</Tree>
	)"));
}

TEST(TreeNode, ChildNodesEmplacement) {
	Tree::Node<DataType> rootNode;

	for (const auto& x : {1, 2, 3}) {
		auto& first = rootNode.emplace(x);

		for (const auto& y : {4, 5}) {
			auto& second = first.emplace(y);
			for (const auto& z : {6, 7, 8}) {
				second.emplace(z);
			}
		}
	}

	EXPECT_TRUE(IsEqual(rootNode, R"(
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

TEST(TreeNode, DepthFirstTraversal) {
	Tree::Node<DataType> rootNode;

	rootNode.emplace(1).emplace(2).emplace(3);
	rootNode.emplace(1).emplace(2).emplace(3);
	rootNode.emplace(1).emplace(2).emplace(3);

	int next = 0;
	rootNode.depthTraversal([&next](Tree::Node<DataType>& value) {
		value->value = next;
		next += 1;

		return false;
	});

	EXPECT_TRUE(IsEqual(rootNode, R"(
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

TEST(TreeNode, BreadthFirstTraversal) {
	Tree::Node<DataType> rootNode;

	rootNode.emplace(1).emplace(2).emplace(3);
	rootNode.emplace(1).emplace(2).emplace(3);
	rootNode.emplace(1).emplace(2).emplace(3);

	int next = 0;
	rootNode.breadthTraversal([&next](Tree::Node<DataType>& value) {
		value->value = next;
		next += 1;

		return false;
	});

	EXPECT_TRUE(IsEqual(rootNode, R"(
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

TEST(TreeNode, NodeDeletion) {}

TEST(TreeNode, SubtreeCopying) {
	Tree::Node<DataType> rootNode;
	rootNode.emplace(1).emplace(2).emplace(3);

	const auto nodeCopy = rootNode;
	EXPECT_TRUE(IsEqual(nodeCopy, R"(
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

TEST(TreeNode, CheckIndependenceCopiedSubtree) {}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
