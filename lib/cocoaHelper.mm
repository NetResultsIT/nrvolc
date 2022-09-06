#include "cocoaHelper.h"

#import <foundation/foundation.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSImage.h>
#import <AppKit/NSCursor.h>
#import <AppKit/NSScreen.h>
#import <corefoundation/CFString.h>
#import <foundation/NSString.h>
#import <ApplicationServices/ApplicationServices.h>

@implementation CocoaHelper
- (NSAppleEventDescriptor *) cocoaCallAppleScript: (NSString *) actualScript
{
    NSAppleScript *script = [[NSAppleScript alloc] initWithSource:actualScript];
    NSDictionary **anErr = NULL;
    NSAppleEventDescriptor *descriptor = [script executeAndReturnError:anErr];
    /*if (!anErr)
        NSLog(@"Error calling Applescript: %@", anErr);*/
    [script release];
    return descriptor;
}

-(int) cocoaInc: (int) base
{
    return base+1;
}

@end
