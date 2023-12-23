#pragma once

namespace Log {
	namespace Details {
		inline auto Level = 0;
		inline std::vector<std::function<void(std::string_view)>> Loggers;
	} // namespace Details

	// The user must ensure that the logger will function correctly throughout the application's lifetime.
	template <typename Logger, typename... Value>
	void CreateLogger(Value&&... values) {
		Details::Loggers.emplace_back(Logger(std::forward<Value>(values)...));
	}

	// The main logging function. Important! Do not use it directly.
	// Define the necessary logging levels and add your own copies of the function.
	//
	// For example:
	// auto Debug = Log::Log<Level::Debug>;
	// auto Error = Log::Log<Level::Error>;
	//
	// Debug("Some text here.");
	template <typename LevelType>
	auto Log = []<typename Message, typename... Value>(Message&& message, Value&&... values) {
		if (LevelType::Level >= Details::Level) {
			const auto output = std::vformat(std::forward<Message>(message), std::make_format_args(std::forward<Value>(values)...));
			for (const auto& logger : Details::Loggers) {
				logger(std::format("[{}] {}\n", LevelType::Name, output));
			}
		}
	};

	// Sets the minimum logging level. Messages below this level will not be logged.
	void SetLevel(int level);

	// Resets the logging configuration.
	void Reset();

} // namespace Log
