// Copyright ❤️ 2023-2024, Sergei Belov

#include "DepthFirstIterator.h"

#include <Debug/CopyMoveData.h>
#include <gtest/gtest.h>
#include <Node.h>

using DataType = Debug::CopyMoveData<int>;

TEST(Node, CreateRoot) {
	const Node rootNode;

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Path="" Size="0" Depth="0"></Node>
		</Tree>
	)"));

	EXPECT_FALSE(IsEqual(rootNode, R"(
		<Tree></Tree>
	)"));
}

TEST(Node, CreateChild) {
	Node rootNode;

	rootNode.CreateChild();
	rootNode.CreateChild();

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Path="" Size="0" Depth="0">
				<Node Path="" Size="0" Depth="1"></Node>
				<Node Path="" Size="0" Depth="1"></Node>
			</Node>
		</Tree>
	)"));
}

TEST(Node, CreateTree) {
	Node rootNode;

	for (const auto& x : { 1, 2, 3 }) {
		auto& first = rootNode.CreateChild();

		for (const auto& y : { 4, 5 }) {
			auto& second = first.CreateChild();
			for (const auto& z : { 6, 7, 8 }) {
				second.CreateChild();
			}
		}
	}

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Path="" Size="0" Depth="0">
				<Node Path="" Size="0" Depth="1">
					<Node Path="" Size="0" Depth="2">
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
					</Node>
					<Node Path="" Size="0" Depth="2">
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
					</Node>
				</Node>
				<Node Path="" Size="0" Depth="1">
					<Node Path="" Size="0" Depth="2">
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
					</Node>
					<Node Path="" Size="0" Depth="2">
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
					</Node>
				</Node>
				<Node Path="" Size="0" Depth="1">
					<Node Path="" Size="0" Depth="2">
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
					</Node>
					<Node Path="" Size="0" Depth="2">
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
						<Node Path="" Size="0" Depth="3"></Node>
					</Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

TEST(Node, DepthFirstTraversal) {
	Node rootNode;

	rootNode.CreateChild().CreateChild().CreateChild();
	rootNode.CreateChild().CreateChild().CreateChild();
	rootNode.CreateChild().CreateChild().CreateChild();

	int next = 0;
	DepthFirstIterator iterator(&rootNode);

	while (iterator) {
		Node& currentNode = *iterator;
		++iterator;

		currentNode.SetSize(next++);
	}

	EXPECT_TRUE(IsEqual(rootNode, R"(
		<Tree>
			<Node Path="" Size="0" Depth="0">
				<Node Path="" Size="1" Depth="1">
					<Node Path="" Size="2" Depth="2">
						<Node Path="" Size="3" Depth="3"></Node>
					</Node>
				</Node>
				<Node Path="" Size="4" Depth="1">
					<Node Path="" Size="5" Depth="2">
						<Node Path="" Size="6" Depth="3"></Node>
					</Node>
				</Node>
				<Node Path="" Size="7" Depth="1">
					<Node Path="" Size="8" Depth="2">
						<Node Path="" Size="9" Depth="3"></Node>
					</Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
