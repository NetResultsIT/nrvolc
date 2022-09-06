#ifndef VOL_COCOA_STUFF_H
#define VOL_COCOA_STUFF_H

#import <foundation/foundation.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSScreen.h>
#import <corefoundation/CFString.h>
#import <foundation/NSString.h>
#import <ApplicationServices/ApplicationServices.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@interface CocoaHelper:NSObject
- (NSAppleEventDescriptor *) cocoaCallAppleScript: (NSString*) script;
- (int) cocoaInc: (int) base;
@end

#endif // GETCURSOR_H
