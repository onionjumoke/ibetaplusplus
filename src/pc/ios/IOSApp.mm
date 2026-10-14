// ─────────────────────────────────────────────────────────────────────────────
// IOSApp.mm — UIApplication delegate + the iosapp bridge implementation.
//
// Main thread: UIApplicationMain runloop, window/view lifecycle, asset
// bootstrap, text-input field.
// Game thread (spawned once): EAGL context ownership and the entire portable
// game loop. Touch events arrive from the main thread through the lwjgl
// input queues (mutex-protected std::queues inside Mouse.cpp/Keyboard.cpp).
//
// Compiled MRC (-fno-objc-arc is baked into the toolchain wrapper).
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

#import "EAGLView.h"
#import "IosBridge.h"
#import "IosInput.h"

#include "GLES11Shim.h"
#include "sdlcompat/sdlcompat.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <sys/stat.h>
#include <thread>
#include <pthread.h>
#include <sys/time.h>

static UIWindow *g_window = nil;
static EAGLView *g_view = nil;
static EAGLContext *g_glContext = nil; // owned by the game thread

static UITextField *g_textField = nil;

// Audio pause hook (implemented by sdlcompat_audio.cpp).
extern "C" void SDL_PauseAudioDevice(unsigned int dev, int pause_on);

static std::mutex g_viewMutex;
static std::condition_variable g_viewCv;
static bool g_viewReady = false;

static std::atomic<bool> g_appActive{true};
static std::atomic<bool> g_quitRequested{false};
static std::atomic<int> g_surfaceW{0};
static std::atomic<int> g_surfaceH{0};

// ═════════════════════════════════════════════════════════════════════════════
// Bridge implementation (see IosBridge.h)
// ═════════════════════════════════════════════════════════════════════════════

static bool bpDebugMarkerPresent();

namespace iosapp
{

void notifySurfaceResized(int width, int height)
{
	g_surfaceW = width;
	g_surfaceH = height;
}

void notifyAppActive(bool active)
{
	g_appActive = active;
	SDL_PauseAudioDevice(1, active ? 0 : 1);
}

void notifyQuitRequested()
{
	g_quitRequested = true;
}

bool waitForView(double timeoutSeconds)
{
	std::unique_lock<std::mutex> lock(g_viewMutex);
	return g_viewCv.wait_for(lock, std::chrono::duration<double>(timeoutSeconds),
	                         [] { return g_viewReady; });
}

bool isAppActive()
{
	return g_appActive;
}

bool isQuitRequested()
{
	return g_quitRequested;
}

// Called on the game thread. The EAGLContext is bound to whichever thread
// creates and makes it current — everything GL stays on this thread forever.
bool createGlContext()
{
	if (g_view == nil || g_glContext != nil)
		return false;

	EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	if (context == nil)
	{
		NSLog(@"IOSApp: device reports no OpenGLES 1.1 support");
		return false;
	}

	if (![EAGLContext setCurrentContext:context])
	{
		[context release];
		return false;
	}

	if (![g_view createFramebufferWithContext:context])
	{
		[EAGLContext setCurrentContext:nil];
		[context release];
		return false;
	}

	g_glContext = context; // transfer ownership (alloc'd above)
	return true;
}

void destroyGlContext()
{
	if (g_glContext == nil)
		return;
	[g_view destroyFramebuffer];
	[EAGLContext setCurrentContext:nil];
	[g_glContext release];
	g_glContext = nil;
}

namespace
{
std::mutex g_hudMutex;
std::string g_hudText;
}

void publishHud(const std::string &text)
{
	std::lock_guard<std::mutex> lock(g_hudMutex);
	g_hudText = text;
}

std::string hudText()
{
	std::lock_guard<std::mutex> lock(g_hudMutex);
	return g_hudText;
}

void presentFrame()
{
	static unsigned long frames = 0;
	static gles11shim::DebugStats prev = {};
	static double prevT = 0.0;

	if (g_view != nil && g_glContext != nil)
		[g_view presentFrame];

	// Profiling telemetry every 300 presents (~5 s at 60 fps): per-second
	// deltas of shim counters reveal where a frame's time goes.
	frames++;
	if ((frames % 300) != 0)
		return;

	struct timeval tv;
	gettimeofday(&tv, nullptr);
	double now = tv.tv_sec + tv.tv_usec * 1e-6;
	double dt = now - prevT;
	gles11shim::DebugStats s = gles11shim::debugStats();
	if (prevT <= 0.0 || dt <= 0)
	{
		prev = s;
		prevT = now;
		return;
	}
	GLint vp[4] = {0, 0, 0, 0};
	glGetIntegerv(GL_VIEWPORT, vp);
	GLenum err = glGetError();
	(void)0;
	prev = s;
	prevT = now;
}

int surfaceWidth() { return static_cast<int>(g_surfaceW) > 0 ? static_cast<int>(g_surfaceW) : 480; }
int surfaceHeight() { return static_cast<int>(g_surfaceH) > 0 ? static_cast<int>(g_surfaceH) : 320; }

} // namespace iosapp

// ═════════════════════════════════════════════════════════════════════════════
// Text input: UIAlertView with a plain text field. Two modes:
//  - CHATKEYS: inject typed chars + RETURN into lwjgl::Keyboard (KB button)
//  - USERNAME: stash the result for the game thread (options username editor)
// A bare UITextField on the window blew the main-thread stack inside
// QuartzCore layout recursion on iOS 5; alerts are the proven path here.
// The delegate is a process-wide singleton — releasing it from inside its own
// dismissal callback made later SHOW toggles dead.
// ═════════════════════════════════════════════════════════════════════════════

static UIAlertView *g_alert = nil;
static NSObject *g_alertDelegate = nil;

enum TextInputMode { INPUT_CHATKEYS = 0, INPUT_USERNAME = 1, INPUT_FIELD = 2 };
static TextInputMode g_inputMode = INPUT_CHATKEYS;
static std::string g_pendingText;
static std::atomic<bool> g_pendingFlag(false);
static std::atomic<int> g_overlayMode(0);

// Replacement-injection channel (CHATKEYS mode)
static std::string g_injectText;
static std::atomic<bool> g_injectFlag(false);
static std::atomic<bool> g_injectReturn(false);
static std::atomic<int> g_injectBackspaces(0);

// Active text source, published by the game thread.
static std::mutex g_activeMutex;
static std::string g_activeText;
static std::atomic<bool> g_inputTarget(false);
static std::atomic<bool> g_pendingShow(false);
static std::atomic<bool> g_blurField(false);

@interface AlertDelegate : NSObject<UIAlertViewDelegate>
@end

@implementation AlertDelegate

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	(void)alertView;
	g_blurField.store(true); // let the game drop field focus so a re-tap reopens
	if (buttonIndex == 0)
		return; // Cancel
	UITextField *f = [g_alert textFieldAtIndex:0];
	NSString *t = f.text ?: @"";
	if (g_inputMode == INPUT_USERNAME)
	{
		g_pendingText = std::string(t.UTF8String ?: "");
		g_pendingFlag.store(true);
		return;
	}
	g_injectText = std::string(t.UTF8String ?: "");
	g_injectFlag.store(true);
	g_injectReturn.store(g_inputMode == INPUT_CHATKEYS); // RETURN only closes chat

	// Do NOT wait for alertViewDidDismiss (unreliable on iOS 5): retire the
	// alert right here. Autorelease keeps it alive through UIKit's own
	// dismissal call stack.
	if (g_alert != nil)
	{
		UIAlertView *retiring = g_alert;
		[retiring retain];
		[retiring autorelease];
		g_alert = nil;
		(void)0;
	}
}

- (void)alertFieldChanged:(UITextField *)sender
{
	// Only plain text fields stream live (chat/username commit on the button, so
	// their key-replay path is not spammed once per keystroke).
	if (g_inputMode != INPUT_FIELD || sender == nil)
		return;
	NSString *t = sender.text ?: @"";
	g_injectText = std::string(t.UTF8String ?: "");
	g_injectBackspaces.store(0);
	g_injectReturn.store(false);
	g_injectFlag.store(true);
}

- (void)alertViewDidDismissWithButtonIndex:(NSInteger)buttonIndex
{
	(void)buttonIndex;
	// Lifecycle is handled in clickedButtonAtIndex / hideTextInput; nothing to
	// do here besides logging.
	(void)0;
}

@end

namespace iosapp
{

static void showAlert(const char *title, const char *initial, TextInputMode mode)
{
	// Copy anything the caller passes into owned storage BEFORE the async block:
	// callers may hand us a temporary std::string's c_str() (e.g. the focused
	// text field's current text), which would dangle by the time this runs.
	std::string titleCopy = (title != nullptr) ? std::string(title) : std::string("Edit");
	const bool hasInitial = (initial != nullptr && initial[0] != '\0');
	std::string initialCopy = hasInitial ? std::string(initial) : std::string();

	dispatch_async(dispatch_get_main_queue(), ^{
		if (g_alert != nil)
		{
			// Self-heal: a stale alert (dismiss callback lost) must not wedge
			// the toggle forever — force it down and build a fresh one.
			(void)0;
			[g_alert dismissWithClickedButtonIndex:g_alert.cancelButtonIndex animated:NO];
			[g_alert release];
			g_alert = nil;
		}
		if (g_alertDelegate == nil)
			g_alertDelegate = [[AlertDelegate alloc] init];
		g_inputMode = mode;

		g_alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:titleCopy.c_str()]
		                                    message:nil
		                                   delegate:g_alertDelegate
		                          cancelButtonTitle:@"Cancel"
		                          otherButtonTitles:@"Send", nil];
		g_alert.alertViewStyle = UIAlertViewStylePlainTextInput;
		UITextField *f = [g_alert textFieldAtIndex:0];
		f.autocorrectionType = UITextAutocorrectionTypeNo;
		f.autocapitalizationType = UITextAutocapitalizationTypeNone;
		f.returnKeyType = UIReturnKeySend;
		std::string initialStr = hasInitial ? initialCopy : activeInputText();
		NSString *nsInitial = [NSString stringWithUTF8String:initialStr.c_str()];
		f.text = nsInitial;
		[f addTarget:g_alertDelegate action:@selector(alertFieldChanged:)
		    forControlEvents:UIControlEventEditingChanged];
		g_injectBackspaces.store((mode == INPUT_FIELD) ? 0 : (NSInteger)initialStr.size());
		(void)0;
		[g_alert show];
	});
}

void showTextInput(const char *initial)
{
	showAlert("Type message:", initial, INPUT_CHATKEYS);
}

void showFieldInput(const char *initial)
{
	showAlert("Edit", initial, INPUT_FIELD);
}

void requestTextInput(const char *title, const char *initial)
{
	showAlert(title, initial, INPUT_USERNAME);
}

bool consumePendingTextInput(std::string &out)
{
	if (!g_pendingFlag.exchange(false))
		return false;
	out = g_pendingText;
	g_pendingText.clear();
	return true;
}

std::string currentUsername()
{
	NSString *path = [[NSHomeDirectory() stringByAppendingPathComponent:@"Library"]
	    stringByAppendingPathComponent:@"username.txt"];
	NSString *raw = [NSString stringWithContentsOfFile:path
	                                          encoding:NSUTF8StringEncoding
	                                             error:nil] ?: @"";
	NSString *trim = [raw stringByTrimmingCharactersInSet:
	                           [NSCharacterSet whitespaceAndNewlineCharacterSet]];
	return trim.length > 0 ? std::string(trim.UTF8String) : std::string("Player");
}

void hideTextInput()
{
	dispatch_async(dispatch_get_main_queue(), ^{
		if (g_alert == nil)
			return;
		UIAlertView *retiring = g_alert;
		[retiring retain];
		[retiring autorelease];
		g_alert = nil;
		g_blurField.store(true);
		[retiring dismissWithClickedButtonIndex:retiring.cancelButtonIndex animated:NO];
	});
}

bool isTextInputActive()
{
	return g_alert != nil;
}

void toggleKeyboard()
{
	bool hidden = (g_alert == nil);
	(void)0;
	if (hidden)
	{
		if (overlayMode() == 1 && !inputTargetExists())
		{
			// Targetless in-world tap: open chat, then let the game thread
			// flash the alert once it has fresh text published (avoids the
			// stale-empty prefill race at low fps).
			(void)0;
			lwjgl::Keyboard::detail::pushLwjglKey(0x14, true);  // KEY_T
			lwjgl::Keyboard::detail::pushLwjglKey(0x14, false);
			g_pendingShow.store(true);
		}
		else
			showTextInput(nullptr);
	}
	else
		hideTextInput();
}

bool consumePendingTextInject(std::string &out, int &backspaces, bool &withReturn)
{
	if (!g_injectFlag.exchange(false))
		return false;
	out = g_injectText;
	g_injectText.clear();
	backspaces = g_injectBackspaces.load();
	withReturn = g_injectReturn.load();
	return true;
}

void publishTextInput(const std::string &text)
{
	std::lock_guard<std::mutex> lock(g_activeMutex);
	g_activeText = text;
}

void publishInputTarget(bool exists) { g_inputTarget.store(exists); }
bool inputTargetExists() { return g_inputTarget.load(); }

std::string activeInputText()
{
	std::lock_guard<std::mutex> lock(g_activeMutex);
	return g_activeText;
}

bool takePendingKeyboardShow()
{
	return g_pendingShow.exchange(false);
}

bool takePendingFieldBlur()
{
	return g_blurField.exchange(false);
}

void setOverlayMode(int mode) { g_overlayMode.store(mode); }
int overlayMode() { return g_overlayMode.load(); }

} // namespace iosapp

// Marker-file debug switch shared with Main_ios.mm: <TMPDIR>/bp.debug enables
// stderr logging and headless game-thread autostart. Falls back to HOME when
// TMPDIR is absent (SSH launches).
static bool bpDebugMarkerPresent()
{
	const char *dir = ::getenv("TMPDIR");
	if (dir == nullptr || dir[0] == '\0')
		dir = ::getenv("HOME");
	if (dir == nullptr || dir[0] == '\0')
		return false;
	std::string marker = std::string(dir) + "/bp.debug";
	struct stat st;
	return ::stat(marker.c_str(), &st) == 0;
}

// ═════════════════════════════════════════════════════════════════════════════
// Assets are NOT bundled: the game is an independent C++ re-implementation, but
// the Minecraft assets are copyright Mojang. The user supplies them in the app's
// Documents folder (iTunes File Sharing):
//   Documents/assets/      (required)
//   Documents/resources/   (optional: sound/music)
//   Documents/.minecraft/  (world saves, written by the game)
// If the assets are absent the game cannot run, so tell the user and quit.
// ═════════════════════════════════════════════════════════════════════════════

static bool assetsPresent()
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *documents = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"];
	NSString *font = [documents stringByAppendingPathComponent:@"assets/font.txt"];
	BOOL isDir = NO;
	NSString *resources = [documents stringByAppendingPathComponent:@"resources"];
	return [fm fileExistsAtPath:font] &&
	       [fm fileExistsAtPath:resources isDirectory:&isDir] && isDir;
}

// A *debug* build may ship assets/resources inside the .app (see
// ios/make-debug-ipa.sh). Seed them into Documents on first launch so the game
// reads from the single Documents root, exactly like a user-supplied copy.
static void seedFolderIfMissing(NSFileManager *fm, NSString *documents, NSString *name)
{
	NSString *dst = [documents stringByAppendingPathComponent:name];
	if ([fm fileExistsAtPath:dst])
		return;
	NSString *src = [[NSBundle mainBundle] pathForResource:name ofType:nil];
	if (src != nil && ![fm copyItemAtPath:src toPath:dst error:nil])
		NSLog(@"iB++: failed to seed bundled %@ into Documents", name);
}

static void requireAssetsOrExit()
{
	NSFileManager *fm = [NSFileManager defaultManager];
	NSString *documents = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents"];
	[fm createDirectoryAtPath:documents withIntermediateDirectories:YES attributes:nil error:nil];

	seedFolderIfMissing(fm, documents, @"assets");
	seedFolderIfMissing(fm, documents, @"resources");

	if (assetsPresent())
		return;

	SDL_ShowSimpleMessageBox(0,
	    "Missing files!",
	    "You're missing the files! please drop your 'assets' and 'resources' folder "
	    "to this app through iTunes",
	    nullptr);
	::exit(0);
}

// ═════════════════════════════════════════════════════════════════════════════
// Root view controller: pins landscape on iOS ≤5 (no rotation rebuilds)
// ═════════════════════════════════════════════════════════════════════════════

@interface RootViewController : UIViewController
@end

@implementation RootViewController

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return interfaceOrientation == UIInterfaceOrientationLandscapeLeft ||
	       interfaceOrientation == UIInterfaceOrientationLandscapeRight;
}

@end

// ═════════════════════════════════════════════════════════════════════════════
// App delegate
// ═════════════════════════════════════════════════════════════════════════════

@interface IOSAppDelegate : NSObject<UIApplicationDelegate, UIAlertViewDelegate>
{
	BOOL _gameStarted;
	BOOL _askingUsername;
}
@property(nonatomic, retain) UIWindow *window;
@end

@implementation IOSAppDelegate

@synthesize window = _window;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	(void)application;
	(void)launchOptions;

	[[UIApplication sharedApplication] setStatusBarHidden:YES animated:NO];
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES]; // no screen sleep mid-mine

	requireAssetsOrExit();

	UIWindow *window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	self.window = window; // property retains
	[window release];     // ownership now solely with the property

	RootViewController *root = [[RootViewController alloc] init];

	CGRect frame = [[UIScreen mainScreen] applicationFrame];
	frame.origin.x = 0;
	frame.origin.y = 0;

	EAGLView *view = [[EAGLView alloc] initWithFrame:frame];
	view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	root.view = view; // retains
	[view release];

	g_window = window; // raw aliases: both objects live for the app lifetime
	g_view = view;

	_window.rootViewController = root; // available since iOS 4
	[root release];

	[_window makeKeyAndVisible];

	CGSize size = _window.bounds.size;
	iosapp::notifySurfaceResized(static_cast<int>(size.width), static_cast<int>(size.height));

	{
		std::lock_guard<std::mutex> lock(g_viewMutex);
		g_viewReady = true;
	}
	g_viewCv.notify_all();

	// Headless/CI launches (SSH, apptool while locked) never reach
	// applicationDidBecomeActive; a $HOME/bp.debug marker begins the game
	// thread right after the view exists so the GL path can be exercised
	// off-screen. Same marker enables stderr logging (see Main_ios.mm).
	if (bpDebugMarkerPresent())
		[self startGameThread];
	return YES;
}

static void *gameThreadEntry(void *)
{
	iosapp::runGameOnThisThread();
	return nullptr;
}

static void spawnGameThreadWithBigStack()
{
	// std::thread uses the default pthread stack (~512 KB on iOS 5), which
	// the chunk-render/worldgen call chain overflows -> silent death (the
	// signal handler cannot run on the exhausted stack). Give the game
	// thread an explicit large stack.
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 16 * 1024 * 1024);
	pthread_t th;
	if (pthread_create(&th, &attr, gameThreadEntry, nullptr) == 0)
		pthread_detach(th);
	pthread_attr_destroy(&attr);
}

- (NSString *)usernameFilePath
{
	return [[NSHomeDirectory() stringByAppendingPathComponent:@"Library"]
	    stringByAppendingPathComponent:@"username.txt"];
}

- (void)startGameThread
{
	if (_gameStarted)
		return;

	// First launch: ask for the player name before the game reads it. The
	// game thread starts once the alert is answered (or immediately when the
	// file already exists).
	NSString *namePath = [self usernameFilePath];
	NSString *existing = [NSString stringWithContentsOfFile:namePath
	                                               encoding:NSUTF8StringEncoding
	                                                  error:nil];
	if (existing == nil || [existing stringByTrimmingCharactersInSet:
	                                   [NSCharacterSet whitespaceAndNewlineCharacterSet]]
	                           .length == 0)
	{
		if (!_askingUsername)
		{
			_askingUsername = YES;
			UIAlertView *alert = [[UIAlertView alloc]
			    initWithTitle:@"Player name"
			    message:@"Enter the name other players will see:"
			    delegate:self
			    cancelButtonTitle:nil
			    otherButtonTitles:@"Play", nil];
			alert.alertViewStyle = UIAlertViewStylePlainTextInput;
			[alert textFieldAtIndex:0].autocorrectionType = UITextAutocorrectionTypeNo;
			[alert textFieldAtIndex:0].autocapitalizationType = UITextAutocapitalizationTypeNone;
			[alert show];
			[alert release];
			return; // spawn happens in alertViewClickedButtonAtIndex
		}
		return;
	}

	_gameStarted = YES;
	spawnGameThreadWithBigStack();
}

- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
	// Modern iOS 5 selector — the legacy single-parameter variant below is
	// never invoked by iOS 5, which stranded first-launch name entry.
	[self alertViewClickedButtonAtIndex:buttonIndex field:[alertView textFieldAtIndex:0]];
}

- (void)alertViewClickedButtonAtIndex:(NSInteger)buttonIndex
{
	[self alertViewClickedButtonAtIndex:buttonIndex field:nil];
}

- (void)alertViewClickedButtonAtIndex:(NSInteger)buttonIndex field:(UITextField *)fieldIn
{
	if (buttonIndex < 0)
		return;
	NSString *path = [self usernameFilePath];
	UITextField *field = fieldIn;
	if (field == nil)
	{
		// Fallback: reach the visible alert through its presentation window.
		for (UIWindow *w in [[UIApplication sharedApplication] windows])
			for (UIView *v in w.subviews)
				if ([v isKindOfClass:[UIAlertView class]])
					field = [(UIAlertView *)v textFieldAtIndex:0];
	}

	NSString *name = @"Player";
	NSString *typed = field.text ?: @"";
	// Keep only [A-Za-z0-9_], like the vanilla login does.
	NSCharacterSet *allowed = [NSCharacterSet
	    characterSetWithCharactersInString:
	        @"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"];
	NSMutableString *clean = [NSMutableString string];
	for (NSUInteger ci = 0; ci < typed.length; ci++)
	{
		unichar ch = [typed characterAtIndex:ci];
		if ([allowed characterIsMember:ch])
			[clean appendFormat:@"%C", ch];
	}
	NSString *finalName = [clean stringByTrimmingCharactersInSet:
	                                  [NSCharacterSet whitespaceCharacterSet]];
	if (finalName.length > 0 && finalName.length <= 16)
		name = finalName;

	[name writeToFile:path atomically:YES encoding:NSUTF8StringEncoding error:nil];
	_askingUsername = NO;

	if (!_gameStarted)
	{
		_gameStarted = YES;
		spawnGameThreadWithBigStack();
	}
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
	(void)application;
	iosapp::notifyAppActive(true);
	[self startGameThread];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	(void)application;
	iosapp::notifyAppActive(false);
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	(void)application;
	iosapp::notifyQuitRequested();
	// Give the game loop a moment to unwind before the process dies.
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

- (void)dealloc
{
	[_window release];
	[super dealloc];
}

@end
