// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "ThreadAllocatorPool.h"

#include <fmt/format.h>

class Node final : public ThreadAllocatorPool<Node> {
public:
	Node() noexcept;
	~Node();

	Node(const Node&) noexcept = delete;
	Node& operator=(const Node&) noexcept = delete;

	Node(Node&&) noexcept = delete;
	Node& operator=(Node&&) noexcept = delete;

	bool HasParent() const;
	Node& GetParent();
	const Node& GetParent() const;

	bool HasChildren() const;

	std::vector<Node*> GetChildren();
	std::vector<const Node*> GetChildren() const;

	size_t GetSize() const;
	void SetSize(size_t newSize);

	const std::string& GetPath() const;
	void SetPath(std::string newPath);

	size_t GetDepth() const;
	std::string GetFullPath() const;

	Node& CreateChild();

private:
	Node* parentNode = nullptr;

	Node* lastChild = nullptr;
	Node* nextChild = nullptr;

	std::string nodePath;
	std::size_t nodeSize = 0;
};

template <>
struct fmt::formatter<Node> : fmt::formatter<std::string_view> {
	auto format(const Node& value, fmt::format_context& context) const {
		std::stack<const Node*> pending;
		pending.emplace(&value);

		std::string result = "<Tree>";

		while (!pending.empty()) {
			if (pending.top() == nullptr) {
				result += "</Node>";

				pending.pop();
				continue;
			}

			const auto& current = *pending.top();
			pending.pop();

			result += fmt::format("<Node Path=\"{}\" Size=\"{}\" Depth=\"{}\">", current.GetPath(), current.GetSize(), current.GetDepth());

			if (current.HasChildren()) {
				// Close node later, after all children have been processed.
				pending.emplace(nullptr);
			} else {
				result += "</Node>";
			}

			for (const Node* child : current.GetChildren()) {
				pending.emplace(child);
			}
		}

		return fmt::formatter<std::string_view>::format(result + "</Tree>", context);
	}
};

inline bool IsEqual(const Node& node, std::string xmlString) {
	std::string xmlNodeString = fmt::format("{}", node);

	{
		auto& string = xmlNodeString;
		string.erase(std::remove_if(string.begin(), string.end(), ::isspace), string.end());
	}

	{
		auto& string = xmlString;
		string.erase(std::remove_if(string.begin(), string.end(), ::isspace), string.end());
	}

	return xmlNodeString == xmlString;
}