// Copyright ❤️ 2023-2024, Sergei Belov

#include "Components/SystemFontComponent.h"

#import <Cocoa/Cocoa.h>

FontCollection::BinaryFontDataType SystemFontComponent::GetBinaryFontData() {
	NSFont* systemFont = [NSFont systemFontOfSize:18];
	
	CTFontRef ctFont = (__bridge CTFontRef)systemFont;
	CFTypeRef urlAttribute = CTFontCopyAttribute(ctFont, kCTFontURLAttribute);
	
	NSData* contentData = [NSData dataWithContentsOfURL:(__bridge NSURL *)urlAttribute];
	CFRelease(urlAttribute);
	
	const uint8_t* binaryData = (const uint8_t*)[contentData bytes];

	return { binaryData, binaryData + [contentData length] };
}
