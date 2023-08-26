#include <gtest/gtest.h>
#include <Filesystem.cc>

TEST(Filesystem, BuildTree) {
	const auto filesystemTree = Filesystem::BuildTree(std::filesystem::absolute("../Tests/Filesystem/One"));

	auto str = ToString(filesystemTree);

	EXPECT_TRUE(IsEqual(filesystemTree, R"(
		<Tree>
			<Node Path="One" Size="4037017">
				<Node Path="2097152 Bytes" Size="2097152"></Node>
				<Node Path="Five" Size="0">
					<Node Path="Empty" Size="0"></Node>
				</Node>
				<Node Path="Three" Size="1572864">
					<Node Path="1048576 Bytes" Size="1048576"></Node>
					<Node Path="Four" Size="524288">
						<Node Path="524288 Bytes" Size="524288"></Node>
					</Node>
				</Node>
				<Node Path="Two" Size="367001">
					<Node Path="104857 Bytes" Size="104857"></Node>
					<Node Path="262144 Bytes" Size="262144"></Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
