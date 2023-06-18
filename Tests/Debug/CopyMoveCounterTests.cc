#include <gtest/gtest.h>

#include <Debug/CopyMoveCounter.h>

TEST(CopyMoveCounter, InitializingCounters)
{
	Debug::CopyMoveCounter<int> defaultInitialized;
	Debug::CopyMoveCounter<int> copyInitialized = 1;

	EXPECT_EQ(defaultInitialized.moved, 0);
	EXPECT_EQ(defaultInitialized.copied, 0);
	EXPECT_EQ(defaultInitialized.value, 0);

	EXPECT_EQ(copyInitialized.moved, 0);
	EXPECT_EQ(copyInitialized.copied, 0);
	EXPECT_EQ(copyInitialized.value, 1);
}

TEST(CopyMoveCounter, CopyingObject)
{
	Debug::CopyMoveCounter<int> counter;
	counter.value = 1;

	auto first = counter;

	auto second = first;
	second.value = 2;

	EXPECT_EQ(first.moved, 0);
	EXPECT_EQ(first.copied, 1);
	EXPECT_EQ(first.value, 1);

	EXPECT_EQ(second.moved, 0);
	EXPECT_EQ(second.copied, 2);
	EXPECT_EQ(second.value, 2);

	auto copied = counter;
	for (const auto& value : {2, 3, 4, 5})
	{
		copied = copied;
		copied.value += 1;

		EXPECT_EQ(copied.moved, 0);
		EXPECT_EQ(copied.copied, value);
		EXPECT_EQ(copied.value, value);
	}
}

TEST(CopyMoveCounter, MovingObject)
{
	Debug::CopyMoveCounter<int> counter;
	counter.value = 1;

	auto first = std::move(counter);

	auto second = std::move(first);
	second.value = 2;

	EXPECT_EQ(first.moved, 1);
	EXPECT_EQ(first.copied, 0);
	EXPECT_EQ(first.value, 1);

	EXPECT_EQ(second.moved, 2);
	EXPECT_EQ(second.copied, 0);
	EXPECT_EQ(second.value, 2);

	auto moved = std::move(counter);
	for (const auto& value : {2, 3, 4, 5})
	{
		moved = std::move(moved);
		moved.value += 1;

		EXPECT_EQ(moved.moved, value);
		EXPECT_EQ(moved.copied, 0);
		EXPECT_EQ(moved.value, value);
	}
}

TEST(CopyMoveCounter, CopyingAndMoving)
{
	Debug::CopyMoveCounter<int> counter;
	counter.value = 1;

	auto copied = counter;
	auto moved = std::move(copied);

	copied = moved;
	moved = std::move(copied);

	EXPECT_EQ(moved.moved, 2);
	EXPECT_EQ(moved.copied, 2);
	EXPECT_EQ(moved.value, 1);
}

int main(int argc, char* argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
