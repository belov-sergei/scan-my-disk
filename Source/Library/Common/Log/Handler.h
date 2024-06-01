// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Log::Handler {
	// Returns all log handlers.
	std::vector<std::function<void(std::string_view)>>& List();

	// Resets all handlers.
	void Reset();

	// Adds a new handler, for example, for logging to a file, a database, etc.
	template <typename T, typename... V>
	void Register(V&&... values) {
		auto& handlers = List();
		handlers.emplace_back(T(std::forward<V>(values)...));
	}

	// Handler for outputting log messages to the Windows Debug Output.
	struct WindowsDebugOutput final {
		void operator()(std::string_view value) const;
	};

	// Handler for outputting log messages to an output stream.
	struct OutputStream final {
		void operator()(std::string_view value) const;
	};

	// Handler for outputting log messages to a file stream.
	struct FileStream final {
		constexpr FileStream(std::string_view filename) noexcept
		    : _filename(filename) {}

		void operator()(std::string_view value) const;

	private:
		std::string_view _filename;
	};
} // namespace Log::Handler
