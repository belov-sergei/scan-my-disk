// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

template <typename IdentifiersGroup = void, typename InternalType = size_t>
struct GenericIdentifier final {
	struct Hash {
		std::size_t operator()(const GenericIdentifier& identifier) const {
			return std::hash<InternalType>()(identifier.value);
		}
	};

	static GenericIdentifier Next() {
		static InternalType value = 0;
		return { value++ };
	}

	bool operator<(GenericIdentifier other) const {
		return value < other.value;
	}

	bool operator==(GenericIdentifier other) const {
		return value == other.value;
	}

private:
	GenericIdentifier(InternalType value)
	    : value(value) {}

private:
	InternalType value;
};

using Identifier = GenericIdentifier<>;
