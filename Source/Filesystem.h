#pragma once

#include <Windows.h>
#include <Tree/Node.h>

namespace Filesystem
{
	struct Entry
	{
		std::filesystem::path path;
	};

	std::vector<std::string> GetLogicalDrives();
	std::pair<ULONGLONG, ULONGLONG> GetDriveSpace(std::string_view driveLetter);

	Tree::Node<Entry> BuildTree(const std::filesystem::path& rootPath);
}