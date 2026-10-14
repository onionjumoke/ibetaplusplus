// sdlcompat_app.mm — bundle/HOME paths, clipboard, URL opening and the crash
// alert. Objective-C is required here; compiled MRC (the toolchain wrapper
// builds everything with -fno-objc-arc, libarclite does not exist for iOS 5).
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "sdlcompat/sdlcompat.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ── Paths ────────────────────────────────────────────────────────────────────
// Desktop SDL returns the executable's directory and the port derives BOTH
// asset paths (<exe>/assets) and data paths (<exe>/.minecraft) from it. On iOS
// we point that root at the app's Documents folder so the user-supplied assets
// (and the saves the game writes) are visible over iTunes File Sharing:
//   Documents/assets      — required, NOT bundled (Mojang copyright)
//   Documents/resources   — optional sound/music
//   Documents/.minecraft  — world saves
// NSHomeDirectory is valid by the time any SDL call runs.

extern "C" char *SDL_GetBasePath(void)
{
	NSString *home = NSHomeDirectory();
	if (home.length == 0)
		home = @"/var/mobile";
	NSString *dir = [home stringByAppendingPathComponent:@"Documents/"];
	return strdup(dir.UTF8String);
}

extern "C" void SDL_free(void *p)
{
	free(p);
}

// ── Clipboard ────────────────────────────────────────────────────────────────

extern "C" char *SDL_GetClipboardText(void)
{
	NSString *text = [[UIPasteboard generalPasteboard] string];
	if (text == nil)
		text = @"";
	return strdup(text.UTF8String);
}

// ── URL opening ──────────────────────────────────────────────────────────────

extern "C" int SDL_OpenURL(const char *url)
{
	if (url == nullptr)
		return -1;
	NSString *ns = [NSString stringWithUTF8String:url];
	NSURL *nsurl = [NSURL URLWithString:ns];
	if (nsurl == nil)
		return -1;
	dispatch_async(dispatch_get_main_queue(), ^{
		[[UIApplication sharedApplication] openURL:nsurl];
	});
	return 0;
}

// ── Crash message box ────────────────────────────────────────────────────────

@interface SdlAlertDelegate : NSObject<UIAlertViewDelegate>
{
@public
	bool *doneFlag;
}
@end

@implementation SdlAlertDelegate

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	(void)alertView;
	(void)buttonIndex;
	if (doneFlag != nullptr)
		*doneFlag = true;
}

@end

static void showAlertOnMainThread(const char *title, const char *message, bool *done)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSString *nsTitle = [NSString stringWithUTF8String:title ? title : ""];
	NSString *nsMessage = [NSString stringWithUTF8String:message ? message : ""];
	SdlAlertDelegate *delegate = [[SdlAlertDelegate alloc] init]; // leaked on purpose: crash path
	delegate->doneFlag = done;

	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:nsTitle
	                                                message:nsMessage
	                                               delegate:delegate
	                                      cancelButtonTitle:@"OK"
	                                      otherButtonTitles:nil];
	[alert show];
	[alert release]; // alert retained by the view hierarchy while visible

	[pool drain];
}

extern "C" int SDL_ShowSimpleMessageBox(Uint32 flags, const char *title, const char *message, void *window)
{
	(void)flags;
	(void)window;

	fprintf(stderr, "[Crash] %s: %s\n", title ? title : "", message ? message : "");

	bool done = false;
	if ([NSThread isMainThread])
	{
		showAlertOnMainThread(title, message, &done);
		while (!done)
			[[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
			                         beforeDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
	}
	else
	{
		// Called from the game thread: run the alert on the main thread and
		// poll until dismissed.
		showAlertOnMainThread(title, message, &done);
		while (!done)
			usleep(50000);
	}
	return 0;
}
