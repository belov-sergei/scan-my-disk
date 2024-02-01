// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <fmt/format.h>

namespace Debug {
	// This template struct is designed to be embedded as a member in other classes
	// in order to track the number of copy and move operations for tests.
	template <typename ValueType>
	struct CopyMoveData final {
		CopyMoveData() = default;
		~CopyMoveData() = default;

		template <typename... ArgumentTypes, std::enable_if_t<std::is_constructible_v<ValueType, ArgumentTypes...>, bool> = true>
		CopyMoveData(ArgumentTypes... arguments)
			: value(std::forward<ArgumentTypes>(arguments)...) { }

		CopyMoveData(const CopyMoveData& other) {
			this->value = other.value;

			this->moved = other.moved;
			this->copied = other.copied + 1;
		}

		CopyMoveData(CopyMoveData&& other) noexcept {
			this->value = std::move(other.value);

			this->moved = other.moved + 1;
			this->copied = other.copied;
		}

		CopyMoveData& operator=(const CopyMoveData& other) {
			this->value = other.value;

			this->moved = other.moved;
			this->copied = other.copied + 1;

			return *this;
		}

		CopyMoveData& operator=(CopyMoveData&& other) noexcept {
			this->value = std::move(other.value);

			this->moved = other.moved + 1;
			this->copied = other.copied;

			return *this;
		}

		size_t moved = 0;
		size_t copied = 0;

		ValueType value = {};
	};
} // namespace Debug

template <typename ValueType>
struct fmt::formatter<Debug::CopyMoveData<ValueType>> : fmt::formatter<std::string_view> {
	auto format(const Debug::CopyMoveData<ValueType>& value, fmt::format_context& context) const {
		std::string result;

		fmt::format_to(std::back_inserter(result), "Moved= \"{}\" ", value.moved);
		fmt::format_to(std::back_inserter(result), "Copied=\"{}\" ", value.copied);
		fmt::format_to(std::back_inserter(result), "Value= \"{}\" ", value.value);

		return fmt::formatter<std::string_view>::format(result, context);
	}
};
