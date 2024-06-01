// Copyright ❤️ 2023-2024, Sergei Belov

#include "Common/Checksum.h"

#include <gtest/gtest.h>

TEST(Checksum, EmptyString) {
	static_assert(Common::Checksum("") == 0x00000000);
	EXPECT_EQ(Common::Checksum(""), 0x00000000);
}

TEST(Checksum, Numbers) {
	static_assert(Common::Checksum("123456789") == 0xcbf43926);
	EXPECT_EQ(Common::Checksum("123456789"), 0xcbf43926);
}

TEST(Checksum, HelloWorld) {
	static_assert(Common::Checksum("Hello, World!") == 0xec4ac3d0);
	EXPECT_EQ(Common::Checksum("Hello, World!"), 0xec4ac3d0);
}

TEST(Checksum, Collision) {
	static_assert(Common::Checksum("Windows") == Common::Checksum("1390518529"));
	EXPECT_EQ(Common::Checksum("Windows"), Common::Checksum("1390518529"));
}

TEST(Checksum, SpecialCharacters) {
	static_assert(Common::Checksum("!@#$%^&*()") == 0xaea29b98);
	EXPECT_EQ(Common::Checksum("!@#$%^&*()"), 0xaea29b98);
}

TEST(Checksum, LongString) {
	static_assert(Common::Checksum("This is a long string with many characters!") == 0x88869ba9);
	EXPECT_EQ(Common::Checksum("This is a long string with many characters!"), 0x88869ba9);
}

TEST(Checksum, Inequality) {
	static_assert(Common::Checksum("Hello, World!") != Common::Checksum("Hello, World?"));
	EXPECT_NE(Common::Checksum("Hello, World!"), Common::Checksum("Hello, World?"));
}
