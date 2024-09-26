// Copyright ❤️ 2023-2024, Sergei Belov

#include <Filesystem.h>
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

namespace Filesystem {
	std::vector<VolumeData> GetVolumesData() {
		std::vector<VolumeData> result;
		
		NSArray* propertyKeys = @[
			NSURLVolumeNameKey,
			NSURLVolumeTotalCapacityKey,
			NSURLVolumeAvailableCapacityKey,
			NSURLVolumeIsBrowsableKey
		];
		
		NSArray* mountedVolumes = [[NSFileManager defaultManager]
			mountedVolumeURLsIncludingResourceValuesForKeys:propertyKeys
			options:0
		];
		
		for(NSURL* volumeURL in mountedVolumes) {
			NSDictionary* propertyValues = [volumeURL resourceValuesForKeys:propertyKeys error:nil];
			
			if (![propertyValues[NSURLVolumeIsBrowsableKey] boolValue]) {
				continue;
			}
			
			VolumeData& volumeData = result.emplace_back();
			volumeData.rootPath = [volumeURL.path cStringUsingEncoding:NSUTF8StringEncoding];
			
			volumeData.name = [propertyValues[NSURLVolumeNameKey] cStringUsingEncoding:NSUTF8StringEncoding];
			
			volumeData.bytesFree = [propertyValues[NSURLVolumeAvailableCapacityKey] unsignedLongLongValue];
			volumeData.bytesTotal = [propertyValues[NSURLVolumeTotalCapacityKey] unsignedLongLongValue];
		}
		
		return result;
	}
 
	void OpenSystemPath(const std::filesystem::path& value) {
		system(fmt::format("open \"{}\"", value.native()).c_str());
	}

	std::string GetLocalSettingsPath() {
		return fmt::format("{}/{}", getenv("HOME"), "Library/Application Support/Scan My Disk/Settings.xml");
	}

	bool Exists(std::string path) {
		return std::filesystem::exists(path);
	}

	std::vector<Node*> EnumerateDirectory(Node* pathNode, std::atomic<size_t>& progress) {
		thread_local std::vector<Node*> newTasks;
		newTasks.clear();

		const auto nodePath = pathNode->GetFullPath();

		NSURL* directoryURL = [NSURL fileURLWithPath:
			[NSString stringWithCString:nodePath.c_str() encoding:NSUTF8StringEncoding]
		];
		
		NSArray* propertyKeys = @[
			NSURLIsDirectoryKey,
			NSURLIsSymbolicLinkKey,
			NSURLIsVolumeKey,
			NSURLVolumeIsBrowsableKey,
			NSURLFileSizeKey
		];
		
		NSArray* directoryContents = [[NSFileManager defaultManager]
			contentsOfDirectoryAtURL:directoryURL
			includingPropertiesForKeys:propertyKeys
			options:0
			error:nil
		];

		size_t totalSize = 0;

		for (NSURL* entryURL in directoryContents) {
			NSDictionary* propertyValues = [entryURL resourceValuesForKeys:propertyKeys error:nil];
			
			if ([propertyValues[NSURLIsSymbolicLinkKey] boolValue]) {
				continue;
			}
			else if ([propertyValues[NSURLIsVolumeKey] boolValue]) {
				if ([propertyValues[NSURLVolumeIsBrowsableKey] boolValue]) {
					continue;
				}
			}

			Node& newNode = pathNode->CreateChild();
			newNode.SetPath([[entryURL lastPathComponent] cStringUsingEncoding:NSUTF8StringEncoding]);
			
			if ([propertyValues[NSURLIsDirectoryKey] boolValue]) {
				newTasks.emplace_back(&newNode);
			}
			else {
				newNode.SetSize([propertyValues[NSURLFileSizeKey] unsignedLongLongValue]);
				totalSize += newNode.GetSize();
			}
		}

		progress += totalSize;
		
		return newTasks;
	}

	std::string OpenSelectFolderDialog() {
		std::string result;
		
		NSOpenPanel* openPanel = [NSOpenPanel openPanel];
		[openPanel setCanChooseFiles:NO];
		[openPanel setCanChooseDirectories:YES];

		if ([openPanel runModal] == NSModalResponseOK)
		{
			NSString* path = [[openPanel URL] path];
			result = [path cStringUsingEncoding:NSUTF8StringEncoding];
		}
		
		return result + "/";
	}

	std::string BytesToString(size_t value) {
		constexpr std::array units = {"B", "KB", "MB", "GB", "TB", "PB"};

		auto size = static_cast<double>(value);

		size_t unit = 0;
		while (size >= 1000 && unit < units.size()) {
			size /= 1000;
			unit++;
		}

		return fmt::format("{:.2f} {}", size, units[unit]);
	}
}
