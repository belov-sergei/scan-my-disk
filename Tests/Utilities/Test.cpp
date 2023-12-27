// Copyright ❤️ 2023-2024, Sergei Belov

#include <Utilities.h>
#include <gtest/gtest.h>

TEST(Utilities, CRC32EmptyString) {
	static_assert(Utilities::CRC32("") == 0x00000000);
	EXPECT_EQ(Utilities::CRC32(""), 0x00000000);
}

TEST(Utilities, CRC32Numbers) {
	static_assert(Utilities::CRC32("123456789") == 0xcbf43926);
	EXPECT_EQ(Utilities::CRC32("123456789"), 0xcbf43926);
}

TEST(Utilities, CRC32HelloWorld) {
	static_assert(Utilities::CRC32("Hello, World!") == 0xec4ac3d0);
	EXPECT_EQ(Utilities::CRC32("Hello, World!"), 0xec4ac3d0);
}

TEST(Utilities, CRC32Collision) {
	static_assert(Utilities::CRC32("Windows") == Utilities::CRC32("1390518529"));
	EXPECT_EQ(Utilities::CRC32("Windows"), Utilities::CRC32("1390518529"));
}

TEST(Utilities, CRC32SpecialCharacters) {
	static_assert(Utilities::CRC32("!@#$%^&*()") == 0xaea29b98);
	EXPECT_EQ(Utilities::CRC32("!@#$%^&*()"), 0xaea29b98);
}

TEST(Utilities, CRC32LongString) {
	static_assert(Utilities::CRC32("This is a long string with many characters!") == 0x88869ba9);
	EXPECT_EQ(Utilities::CRC32("This is a long string with many characters!"), 0x88869ba9);
}

TEST(Utilities, CRC32Inequality) {
	static_assert(Utilities::CRC32("Hello, World!") != Utilities::CRC32("Hello, World?"));
	EXPECT_NE(Utilities::CRC32("Hello, World!"), Utilities::CRC32("Hello, World?"));
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
