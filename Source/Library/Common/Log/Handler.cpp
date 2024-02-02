// Copyright ❤️ 2023-2024, Sergei Belov

#include "Handler.h"

namespace Log::Handler {
	namespace Detail {
		std::vector<std::function<void(std::string_view)>> Handlers;
	}

	std::vector<std::function<void(std::string_view)>>& List() {
		return Detail::Handlers;
	}

	void Reset() {
		Detail::Handlers = {};
	}

	void OutputStream::operator()(std::string_view value) const {
		std::cout << value;
	}

	void FileStream::operator()(std::string_view value) const {
		std::ofstream stream(_filename.data(), std::ios::app);
		if (stream.is_open()) {
			stream << value;
		}
	}
}
