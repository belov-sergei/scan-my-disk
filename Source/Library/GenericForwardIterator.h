// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

// Represents a generic forward iterator. It is a base class for creating custom forward iterators.
// It uses CRTP to provide basic iterator functionalities.
//
// The derived class must implement the following methods:
// - bool Equal(const DerivedType&) const;
// - DerivedType& Increment();
// - ValueType& Dereference()
template <typename DerivedType, typename ValueType>
struct GenericForwardIterator {
	using iterator_category = std::forward_iterator_tag;
	using value_type = ValueType;
	using difference_type = std::ptrdiff_t;
	using pointer = ValueType*;
	using reference = ValueType&;

	[[nodiscard]] bool operator==(const DerivedType& other) const {
		return static_cast<const DerivedType*>(this)->Equal(other);
	}

	[[nodiscard]] bool operator!=(const DerivedType& other) const {
		return !operator==(other);
	}

	DerivedType& operator++() {
		static_cast<DerivedType*>(this)->Increment();
		return *static_cast<DerivedType*>(this);
	}

	DerivedType operator++(int) {
		DerivedType value = *static_cast<DerivedType*>(this);
		operator++();

		return value;
	}

	[[nodiscard]] ValueType& operator*() {
		return static_cast<DerivedType*>(this)->Dereference();
	}

	[[nodiscard]] ValueType* operator->() {
		return &static_cast<DerivedType*>(this)->Dereference();
	}

	[[nodiscard]] operator bool() {
		return static_cast<DerivedType&>(*this) != DerivedType();
	}
};
