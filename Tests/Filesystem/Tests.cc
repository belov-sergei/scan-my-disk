#include <gtest/gtest.h>
#include <Filesystem.cc>

TEST(Filesystem, BuildTree) {
	std::atomic<size_t> progress;
	const auto root = Filesystem::BuildTree(std::filesystem::absolute("../Tests/Filesystem/One"), progress);

	EXPECT_TRUE(IsEqual(root, R"(
		<Tree>
			<Node Path="One" Size="4037017" Depth="0">
				<Node Path="2097152 Bytes" Size="2097152" Depth="1"></Node>
				<Node Path="Five" Size="0" Depth="1">
					<Node Path="Empty" Size="0" Depth="2"></Node>
				</Node>
				<Node Path="Three" Size="1572864" Depth="1">
					<Node Path="1048576 Bytes" Size="1048576" Depth="2"></Node>
					<Node Path="Four" Size="524288" Depth="2">
						<Node Path="524288 Bytes" Size="524288" Depth="3"></Node>
					</Node>
				</Node>
				<Node Path="Two" Size="367001" Depth="1">
					<Node Path="104857 Bytes" Size="104857" Depth="2"></Node>
					<Node Path="262144 Bytes" Size="262144" Depth="2"></Node>
				</Node>
			</Node>
		</Tree>
	)"));
}

TEST(Filesystem, ParallelBuildTree) {
	using namespace std::chrono_literals;

	std::timed_mutex mutex;

	const auto worker = [&] {
		std::atomic<size_t> progress;
		const auto root = Filesystem::ParallelBuildTree(std::filesystem::absolute("../Tests/Filesystem/One"), progress);

		EXPECT_TRUE(IsEqual(root, R"(
			<Tree>
				<Node Path="One" Size="4037017" Depth="0">
					<Node Path="2097152 Bytes" Size="2097152" Depth="1"></Node>
					<Node Path="Five" Size="0" Depth="1">
						<Node Path="Empty" Size="0" Depth="2"></Node>
					</Node>
					<Node Path="Three" Size="1572864" Depth="1">
						<Node Path="1048576 Bytes" Size="1048576" Depth="2"></Node>
						<Node Path="Four" Size="524288" Depth="2">
							<Node Path="524288 Bytes" Size="524288" Depth="3"></Node>
						</Node>
					</Node>
					<Node Path="Two" Size="367001" Depth="1">
						<Node Path="104857 Bytes" Size="104857" Depth="2"></Node>
						<Node Path="262144 Bytes" Size="262144" Depth="2"></Node>
					</Node>
				</Node>
			</Tree>
		)"));

		// Unlock the mutex to signal that the test is complete.
		mutex.unlock();
	};

	int iterations = 1000;
	while (--iterations > 0) {
		mutex.lock(); // Lock the mutex before starting a new thread.
		auto thread = std::thread(worker);
		
		if (mutex.try_lock_for(1s)) {
			// If successful, the test iteration completes successfully.
			mutex.unlock();
			thread.join();
			continue;
		}

		// If unable to lock within 1 second, test may be stuck.
		break;
	}
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
