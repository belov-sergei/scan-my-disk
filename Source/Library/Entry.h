// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Allocator.h"

class Entry final {
public:
	Entry() noexcept;
	~Entry();

	Entry(const Entry&) noexcept = delete;
	Entry& operator=(const Entry&) noexcept = delete;

	Entry(Entry&&) noexcept = delete;
	Entry& operator=(Entry&&) noexcept = delete;

	bool HasParent() const;
	Entry& GetParent();
	const Entry& GetParent() const;

	bool HasChildren() const;

	size_t GetSize() const;
	void SetSize(size_t newSize);

	const std::string& GetPath() const;
	void SetPath(std::string newPath);

	size_t GetDepth() const;
	std::string GetFullPath() const;

	Entry& CreateChild();

private:
	Entry* parent = nullptr;

	Entry* lastChild = nullptr;
	Entry* nextChild = nullptr;

	std::string path;
	std::atomic_size_t size = 0;

	inline static thread_local Allocator<Entry> allocator;
};
