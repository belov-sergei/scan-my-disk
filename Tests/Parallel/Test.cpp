// Copyright ❤️ 2023-2024, Sergei Belov

#include "Parallel.h"

#include <gtest/gtest.h>

struct Job {
	size_t iterations = 0;
};

TEST(Parallel, Test) {
	bool cancel = false;

	Job job = { 5 };

	Parallel::Execute(
	[](Job& job) {
		std::queue<Job> jobs;

		for (size_t i = 0; i < job.iterations; i++) {
			auto& newJob      = jobs.emplace();
			newJob.iterations = i;
		}

		return jobs;
	},
	job, cancel);
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
