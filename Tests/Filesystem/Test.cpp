// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#include <gtest/gtest.h>

TEST(Filesystem, BuildTree) {
	std::atomic<size_t> progress;
	const auto root = Filesystem::BuildTree(std::filesystem::absolute("../Tests/Filesystem/Sandbox"), progress);

	EXPECT_TRUE(IsEqual(root, R"(
		<Tree>
			<Node Path="Sandbox" Size="4037017" Depth="0">
				<Node Path="Documents" Size="1572864" Depth="1">
					<Node Path="Personal" Size="524288" Depth="2">
						<Node Path="Profile Picture" Size="524288" Depth="3"></Node>
					</Node>
					<Node Path="Research Paper" Size="1048576" Depth="2"></Node>
				</Node>
				<Node Path="Journal" Size="2097152" Depth="1"></Node>
				<Node Path="Program Data" Size="367001" Depth="1">
					<Node Path="Reader" Size="104857" Depth="2"></Node>
					<Node Path="Report" Size="262144" Depth="2"></Node>
				</Node>
				<Node Path="Temp" Size="0" Depth="1">
					<Node Path="Empty" Size="0" Depth="2"></Node>
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
		const auto root = Filesystem::ParallelBuildTree(std::filesystem::absolute("../Tests/Filesystem/Sandbox"), progress);

		EXPECT_TRUE(IsEqual(root, R"(
			<Tree>
				<Node Path="Sandbox" Size="4037017" Depth="0">
					<Node Path="Documents" Size="1572864" Depth="1">
						<Node Path="Personal" Size="524288" Depth="2">
							<Node Path="Profile Picture" Size="524288" Depth="3"></Node>
						</Node>
						<Node Path="Research Paper" Size="1048576" Depth="2"></Node>
					</Node>
					<Node Path="Journal" Size="2097152" Depth="1"></Node>
					<Node Path="Program Data" Size="367001" Depth="1">
						<Node Path="Reader" Size="104857" Depth="2"></Node>
						<Node Path="Report" Size="262144" Depth="2"></Node>
					</Node>
					<Node Path="Temp" Size="0" Depth="1">
						<Node Path="Empty" Size="0" Depth="2"></Node>
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
