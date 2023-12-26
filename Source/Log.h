#pragma once
#include <fmt/format.h>

#if defined(WINDOWS)
#include <Windows.h>
#endif

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
			const auto output = fmt::vformat(std::forward<Message>(message), fmt::make_format_args(std::forward<Value>(values)...));
			for (const auto& logger : Details::Loggers) {
				logger(fmt::format("[{}] {}\n", LevelType::Name, output));
			}
		}
	};

	// Sets the minimum logging level. Messages below this level will not be logged.
	void SetLevel(int level);

	// Resets the logging configuration.
	void Reset();

	struct WindowsDebugOutputLogger {
		void operator()(std::string_view string) const {
#if defined(WINDOWS)
			OutputDebugString(string.data());
#endif
		}
	};

	struct OutputStreamLogger {
		void operator()(std::string_view string) const {
			std::cout << string;
		}
	};

	struct FileStreamLogger final {
		FileStreamLogger(std::string_view file) {
			_file = file;
		}

		void operator()(std::string_view string) const {
			std::ofstream stream(_file);
			if (stream.is_open()) {
				stream << string;
			}
		}

	private:
		std::string _file;
	};

} // namespace Log
