// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

#include <fmt/format.h>

namespace Log {
	namespace TypeTraits {
		template <typename T, typename = void>
		constexpr bool CustomFormat = false;

		template <typename T>
		constexpr bool CustomFormat<T, std::void_t<
			decltype(std::declval<T>().Format)
		>> = true;
	}

	// Provides shared buffers for more efficient memory usage in Logger<T> instances.
	// It allows reusing existing resources for constructing and formatting log messages.
	struct SharedLoggerBuffer {
	protected:
		// Final formatted log entry.
		inline static thread_local std::string _result;

		// Temporary buffer for constructing log messages.
		inline static thread_local std::string _buffer;
	};

	// The main logging component. Important! Do not use it directly.
	// To define custom logging levels, create instances of Logger with the desired Log::Level.
	//
	// Example:
	// auto Debug = Log::Logger<Log::Level::Debug>();
	// auto Error = Log::Logger<Log::Level::Error>();
	//
	// Usage:
	// Debug("Some text here.");
	template <typename T>
	struct Logger final : SharedLoggerBuffer {
		Logger() {
			// Set custom format if applicable, based on the type traits.
			if constexpr (TypeTraits::CustomFormat<T>) {
				_format = T::Format;
			}
		}

		// Log message.
		template <typename M, typename... V>
		void operator()(M&& message, V&&... values) {
			if (T::Level < Level::Get()) {
				return;
			}
			
			_buffer.clear();
			// Format timestamp, log level prefix, and the actual message.
			fmt::format_to(std::back_inserter(_buffer), fmt::runtime(_format), Time::Get(), T::Name, std::forward<M>(message));

			_result.clear();
			// Format the complete log message with provided values.
			fmt::format_to(std::back_inserter(_result), fmt::runtime(_buffer), std::forward<V>(values)...);

			for (auto& handler : Handler::List()) {
				handler(_result);
			}
		}

	private:
		// Default log message format.
		std::string_view _format = "{} <{}> {}\n";
	};
}
