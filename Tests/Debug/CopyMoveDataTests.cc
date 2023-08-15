#include <gtest/gtest.h>

#include <Debug/CopyMoveData.h>

TEST(CopyMoveCounter, InitializingCounters) {
	Debug::CopyMoveData<int> defaultInitialized;
	Debug::CopyMoveData<int> copyInitialized = 1;

	EXPECT_EQ(defaultInitialized.Moved, 0);
	EXPECT_EQ(defaultInitialized.Copied, 0);
	EXPECT_EQ(defaultInitialized.Value, 0);

	EXPECT_EQ(copyInitialized.Moved, 0);
	EXPECT_EQ(copyInitialized.Copied, 0);
	EXPECT_EQ(copyInitialized.Value, 1);
}

TEST(CopyMoveCounter, CopyingObject) {
	Debug::CopyMoveData<int> counter;
	counter.Value = 1;

	auto first = counter;

	auto second = first;
	second.Value = 2;

	EXPECT_EQ(first.Moved, 0);
	EXPECT_EQ(first.Copied, 1);
	EXPECT_EQ(first.Value, 1);

	EXPECT_EQ(second.Moved, 0);
	EXPECT_EQ(second.Copied, 2);
	EXPECT_EQ(second.Value, 2);

	auto copied = counter;
	for (const auto& value : {2, 3, 4, 5}) {
		copied = copied;
		copied.Value += 1;

		EXPECT_EQ(copied.Moved, 0);
		EXPECT_EQ(copied.Copied, value);
		EXPECT_EQ(copied.Value, value);
	}
}

TEST(CopyMoveCounter, MovingObject) {
	Debug::CopyMoveData<int> counter;
	counter.Value = 1;

	auto first = std::move(counter);

	auto second = std::move(first);
	second.Value = 2;

	EXPECT_EQ(first.Moved, 1);
	EXPECT_EQ(first.Copied, 0);
	EXPECT_EQ(first.Value, 1);

	EXPECT_EQ(second.Moved, 2);
	EXPECT_EQ(second.Copied, 0);
	EXPECT_EQ(second.Value, 2);

	auto moved = std::move(counter);
	for (const auto& value : {2, 3, 4, 5}) {
		moved = std::move(moved);
		moved.Value += 1;

		EXPECT_EQ(moved.Moved, value);
		EXPECT_EQ(moved.Copied, 0);
		EXPECT_EQ(moved.Value, value);
	}
}

TEST(CopyMoveCounter, CopyingAndMoving) {
	Debug::CopyMoveData<int> counter;
	counter.Value = 1;

	auto copied = counter;
	auto moved = std::move(copied);

	copied = moved;
	moved = std::move(copied);

	EXPECT_EQ(moved.Moved, 2);
	EXPECT_EQ(moved.Copied, 2);
	EXPECT_EQ(moved.Value, 1);
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
