#pragma once

namespace Debug
{
	template <typename ValueType>
	struct CopyMoveData final
	{
		CopyMoveData() = default;
		~CopyMoveData() = default;

		template <typename... ArgumentTypes> requires std::is_constructible_v<ValueType, ArgumentTypes...>
		CopyMoveData(ArgumentTypes... arguments) : Value(std::forward<ArgumentTypes>(arguments)...)
		{
		}

		CopyMoveData(const CopyMoveData& other)
		{
			this->Value = other.Value;

			this->Moved = other.Moved;
			this->Copied = other.Copied + 1;
		}

		CopyMoveData(CopyMoveData&& other) noexcept
		{
			this->Value = std::move(other.Value);

			this->Moved = other.Moved + 1;
			this->Copied = other.Copied;
		}

		CopyMoveData& operator=(const CopyMoveData& other)
		{
			this->Value = other.Value;

			this->Moved = other.Moved;
			this->Copied = other.Copied + 1;

			return *this;
		}

		CopyMoveData& operator=(CopyMoveData&& other) noexcept
		{
			this->Value = std::move(other.Value);

			this->Moved = other.Moved + 1;
			this->Copied = other.Copied;

			return *this;
		}

		size_t Moved = 0;
		size_t Copied = 0;

		ValueType Value = {};
	};
}
