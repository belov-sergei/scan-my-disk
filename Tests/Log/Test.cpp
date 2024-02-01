// Copyright ❤️ 2023-2024, Sergei Belov

#include <gtest/gtest.h>
#include <Log.h>

namespace Log {
	namespace Level {
		struct Debug {
			constexpr static auto Level = 1;
			constexpr static auto Name = "Debug";
		};

		struct Message {
			constexpr static auto Level = 2;
			constexpr static auto Name = "Message";
		};

		struct Warning {
			constexpr static auto Level = 3;
			constexpr static auto Name = "Warning";
		};

		struct Error {
			constexpr static auto Level = 4;
			constexpr static auto Name = "Error";
		};
	} // namespace Level

	auto Debug = Log::Log<Level::Debug>;
	auto Message = Log::Log<Level::Message>;
	auto Warning = Log::Log<Level::Warning>;
	auto Error = Log::Log<Level::Error>;
} // namespace Log

std::string TestLoggerBuffer;
constexpr auto Message = "Some text here";

struct TestLogger {
	TestLogger() {
		TestLoggerBuffer = {};
	}

	void operator()(std::string_view string) const {
		TestLoggerBuffer += string;
	}
};

TEST(Log, Basic) {
	Log::Reset();

	Log::CreateLogger<TestLogger>();
	Log::SetLevel(Log::Level::Debug::Level);

	Log::Debug(Message);
	EXPECT_EQ(TestLoggerBuffer, fmt::format("[{}] {}\n", Log::Level::Debug::Name, Message));
}

TEST(Log, Reset) {
	Log::Reset();

	Log::CreateLogger<TestLogger>();
	Log::SetLevel(Log::Level::Debug::Level);

	Log::Debug(Message);
	EXPECT_EQ(TestLoggerBuffer, fmt::format("[{}] {}\n", Log::Level::Debug::Name, Message));

	TestLoggerBuffer = {};

	Log::Reset();
	Log::Debug(Message);

	EXPECT_EQ(TestLoggerBuffer, "");
}

TEST(Log, Consistency) {
	Log::Reset();

	Log::CreateLogger<TestLogger>();
	Log::SetLevel(Log::Level::Debug::Level);

	Log::Debug(Message);

	const std::string first = TestLoggerBuffer;
	TestLoggerBuffer = {};

	Log::Debug(Message);

	const std::string second = TestLoggerBuffer;
	TestLoggerBuffer = {};

	EXPECT_EQ(first, second);
}

TEST(Log, Formatting) {
	Log::Reset();

	Log::CreateLogger<TestLogger>();
	Log::SetLevel(Log::Level::Debug::Level);

	Log::Debug("{} {} {}.", "Some", "text", "here");
	EXPECT_EQ(TestLoggerBuffer, "[Debug] Some text here.\n");
}

TEST(Log, Level) {
	Log::Reset();

	Log::CreateLogger<TestLogger>();

	constexpr std::array levels = {
		Log::Level::Error::Level,
		Log::Level::Warning::Level,
		Log::Level::Message::Level,
		Log::Level::Debug::Level,
	};

	constexpr std::array names = {
		Log::Level::Error::Name,
		Log::Level::Warning::Name,
		Log::Level::Message::Name,
		Log::Level::Debug::Name,
	};

	for (const auto level : levels) {
		Log::SetLevel(level);

		Log::Error(Message);
		Log::Warning(Message);
		Log::Message(Message);
		Log::Debug(Message);

		std::string expected;
		for (size_t index = 0; index <= levels.size() - level; index++) {
			expected += fmt::format("[{}] {}\n", names[index], Message);
		}

		EXPECT_EQ(TestLoggerBuffer, expected);
		TestLoggerBuffer = {};
	}
}

TEST(Log, Multithreading) {
	// Feature not implemented.
}

TEST(Log, FileOutput) {
	constexpr auto file = "Log.txt";

	if (std::filesystem::exists(file)) {
		std::filesystem::remove(file);
	}

	Log::Reset();

	Log::CreateLogger<Log::FileStreamLogger>(file);
	Log::SetLevel(Log::Level::Debug::Level);

	Log::Debug(Message);

	std::ifstream input(file);

	const std::stringstream buffer;
	while (input >> buffer.rdbuf()) {}

	EXPECT_EQ(buffer.str(), fmt::format("[{}] {}\n", Log::Level::Debug::Name, Message));
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
