// Copyright ❤️ 2023-2024, Sergei Belov

#include <gtest/gtest.h>
#include <Common/Log.h>

namespace Log {
	auto Debug = Log::Logger<Level::Debug>();
	auto Message = Log::Logger<Level::Message>();
	auto Warning = Log::Logger<Level::Warning>();
	auto Error = Log::Logger<Level::Error>();
}

namespace Detail {
	std::string TestLogHandlerBuffer;
}

constexpr auto Message = "Some text here";
constexpr auto Time = "00:00:00";

struct TestLogHandler {
	TestLogHandler() {
		Detail::TestLogHandlerBuffer = {};
	}

	void operator()(std::string_view string) const {
		Detail::TestLogHandlerBuffer += string;
	}
};

TEST(Log, Basic) {
	Log::Handler::Reset();
	Log::Time::Set([] {
		return Time;
	});

	Log::Handler::Register<TestLogHandler>();
	Log::Level::Set(Log::Level::Debug::Level);

	Log::Debug(Message);
	EXPECT_EQ(Detail::TestLogHandlerBuffer, fmt::format("{} <{}> {}\n", Time, Log::Level::Debug::Name, Message));
}

TEST(Log, Reset) {
	Log::Handler::Reset();
	Log::Time::Set([] {
		return Time;
	});

	Log::Handler::Register<TestLogHandler>();
	Log::Level::Set(Log::Level::Debug::Level);

	Log::Debug(Message);
	EXPECT_EQ(Detail::TestLogHandlerBuffer, fmt::format("{} <{}> {}\n", Time, Log::Level::Debug::Name, Message));

	Detail::TestLogHandlerBuffer = {};

	Log::Handler::Reset();
	Log::Debug(Message);

	EXPECT_EQ(Detail::TestLogHandlerBuffer, "");
}

TEST(Log, Consistency) {
	Log::Handler::Reset();
	Log::Time::Set([] {
		return Time;
	});

	Log::Handler::Register<TestLogHandler>();
	Log::Level::Set(Log::Level::Debug::Level);

	Log::Debug(Message);

	const std::string first = Detail::TestLogHandlerBuffer;
	Detail::TestLogHandlerBuffer = {};

	Log::Debug(Message);

	const std::string second = Detail::TestLogHandlerBuffer;
	Detail::TestLogHandlerBuffer = {};

	EXPECT_EQ(first, second);
}

TEST(Log, Formatting) {
	Log::Handler::Reset();
	Log::Time::Set([] {
		return Time;
	});

	Log::Handler::Register<TestLogHandler>();
	Log::Level::Set(Log::Level::Debug::Level);

	Log::Debug("{} {} {}.", "Some", "text", "here");
	EXPECT_EQ(Detail::TestLogHandlerBuffer, "00:00:00 <Debug> Some text here.\n");
}

TEST(Log, Level) {
	Log::Handler::Reset();
	Log::Time::Set([] {
		return Time;
	});

	Log::Handler::Register<TestLogHandler>();

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
		Log::Level::Set(level);

		Log::Error(Message);
		Log::Warning(Message);
		Log::Message(Message);
		Log::Debug(Message);

		std::string expected;
		for (size_t index = 0; index <= levels.size() - level; index++) {
			expected += fmt::format("{} <{}> {}\n", Time, names[index], Message);
		}

		EXPECT_EQ(Detail::TestLogHandlerBuffer, expected);
		Detail::TestLogHandlerBuffer = {};
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

	Log::Handler::Reset();
	Log::Time::Set([] {
		return Time;
	});

	Log::Handler::Register<Log::Handler::FileStream>(file);
	Log::Level::Set(Log::Level::Debug::Level);

	Log::Debug(Message);

	std::ifstream input(file);

	const std::stringstream buffer;
	while (input >> buffer.rdbuf()) {}

	EXPECT_EQ(buffer.str(), fmt::format("{} <{}> {}\n", Time, Log::Level::Debug::Name, Message));
}

int main(int argc, char* argv[]) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
