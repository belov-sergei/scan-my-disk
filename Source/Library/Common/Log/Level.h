// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

// Standard logging levels. Each level defines its priority, name, and format.
// If the format is not specified, the default message format "{} <{}> {}\n" will be used,
// and the output will be something like "24.028-14:32:22 <Debug> Hello, World!".
//
// Users can define their own logging levels, for example:
// struct MyLevel {
//     uint8_t Level = 5;
//     std::string_view Name = "My Level";
//     std::string_view Format = R"(<Message Level="{1}" Time="{0}">{2}</Message>)\n";
// };

namespace Log::Level {
	// Sets the logging level. Any levels below this value will cease to be logged.
	void Set(uint8_t value);

	// Returns the current logging level.
	uint8_t Get();

	// Detailed debugging information during the development and testing of the application.
	struct Debug {
		constexpr static uint8_t Level = 1;
		constexpr static std::string_view Name = "Debug";
	};

	// General information about the program's execution progress.
	struct Message {
		constexpr static uint8_t Level = 2;
		constexpr static std::string_view Name = "Message";
	};

	// Possible issues or non-critical errors that do not lead to a critical failure of the application.
	struct Warning {
		constexpr static uint8_t Level = 3;
		constexpr static std::string_view Name = "Warning";
	};

	// Errors that lead to a failure or incorrect operation of the application.
	struct Error {
		constexpr static uint8_t Level = 4;
		constexpr static std::string_view Name = "Error";
	};
} // namespace Log::Level
