#include <Filesystem.h>

namespace Filesystem
{
	std::vector<std::string> GetLogicalDrives()
	{
		std::vector<std::string> logicalDrives;

		DWORD availableDrivesBitmask = ::GetLogicalDrives();
		for (auto driveLetter = 'A'; driveLetter <= 'Z'; driveLetter++)
		{
			if (availableDrivesBitmask & 1)
			{
				logicalDrives.emplace_back(std::format("{}://", driveLetter));
			}

			availableDrivesBitmask >>= 1;
		}

		return logicalDrives;
	}

	std::pair<ULONGLONG, ULONGLONG> GetDriveSpace(std::string_view driveLetter)
	{
		ULARGE_INTEGER bytesTotal, bytesFree;
		::GetDiskFreeSpaceEx(driveLetter.data(), nullptr, &bytesTotal, &bytesFree);

		return std::make_pair(bytesTotal.QuadPart, bytesFree.QuadPart);
	}

	Tree::Node<Entry> BuildTree(const std::filesystem::path& rootPath)
	{
		Tree::Node<Entry> filesystemTree;

		auto& rootNode = filesystemTree;
		rootNode->path = rootPath;

		std::stack<decltype(&rootNode)> stack;
		stack.emplace(&rootNode);

		std::error_code errorCode;
		while (!stack.empty())
		{
			auto* nextNode = stack.top();
			stack.pop();

			for (const auto& entry : std::filesystem::directory_iterator((*nextNode)->path, errorCode))
			{
				auto& childNode = nextNode->Emplace(entry.path());
				if (std::filesystem::is_directory(entry, errorCode))
				{
					stack.emplace(&childNode);
				}
			}
		}

		return filesystemTree;
	}
}