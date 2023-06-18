#pragma once

namespace Debug
{
	template <typename ValueType>
	struct CopyMoveCounter final
	{
		CopyMoveCounter() = default;
		~CopyMoveCounter() = default;

		template <typename... Types>
		CopyMoveCounter(Types... values) : value(std::forward<Types>(values)...)
		{
		}

		CopyMoveCounter(const CopyMoveCounter& other)
		{
			value = other.value;

			moved = other.moved;
			copied = other.copied + 1;
		}

		CopyMoveCounter(CopyMoveCounter&& other) noexcept
		{
			value = std::move(other.value);

			moved = other.moved + 1;
			copied = other.copied;
		}

		CopyMoveCounter& operator=(const CopyMoveCounter& other)
		{
			value = other.value;

			moved = other.moved;
			copied = other.copied + 1;

			return *this;
		}

		CopyMoveCounter& operator=(CopyMoveCounter&& other) noexcept
		{
			value = std::move(other.value);

			moved = other.moved + 1;
			copied = other.copied;

			return *this;
		}

		size_t moved = 0;
		size_t copied = 0;

		ValueType value = {};
	};
}
