// ─────────────────────────────────────────────────────────────────────────────
// EAGLView.mm — GL surface + full touch→input mapping.
//
// Rendering notes (PORTING.md §3.7):
//   * RGB565 drawable halves fill bandwidth vs RGBA8888 — the SGX-era
//     optimisation; Minecraft's art dithers acceptably.
//   * 24-bit depth buffer (falls back to 16): 16-bit makes chunk culling
//     z-fight because Advanced OpenGL occlusion queries are fail-open here.
//   * contentScaleFactor forced to 1.0: native resolution would quadruple
//     fragment work on retina devices for no visible gain at this art style.
//
// Compiled MRC (-fno-objc-arc is baked into the toolchain wrapper).
#include "EAGLView.h"

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>

#include <cmath>

#include "lwjgl/Mouse.h"
#include "lwjgl/Keyboard.h"
#include "IosInput.h"
#include "IosBridge.h"
#include "net/minecraft/src/ScaledResolution.h"

@implementation EAGLView

+ (Class)layerClass
{
	return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if (self)
	{
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		eaglLayer.opaque = YES;
		// Crisp upscaled pixels (the earlier crash was corrupt saves, not this)
		eaglLayer.minificationFilter = kCAFilterNearest;
		eaglLayer.magnificationFilter = kCAFilterNearest;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
		                                    [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking,
		                                    kEAGLColorFormatRGB565, kEAGLDrawablePropertyColorFormat,
		                                    nil];

		self.multipleTouchEnabled = YES;
		self.userInteractionEnabled = YES;
		// Perf budget: downscale on the weak A5 iPhone; iPads (A6X) render at
		// point resolution (1024x768). CoreAnimation upscales the drawable.
		if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
			g_renderScale = 1.0f;
		else
			g_renderScale = 0.75f;
		self.contentScaleFactor = g_renderScale;
		eaglLayer.contentsScale = g_renderScale;

		_framebuffer = 0;
		_colorRenderbuffer = 0;
		_depthRenderbuffer = 0;
		_backingWidth = 0;
		_backingHeight = 0;

		[self setupOverlayButtons];
	}
	return self;
}

- (void)dealloc
{
	[self destroyFramebuffer];
	[super dealloc];
}

// ── Framebuffer management ───────────────────────────────────────────────────

- (BOOL)createFramebufferWithContext:(EAGLContext *)context
{
	if (_framebuffer != 0 && _backingWidth != 0)
	{
		// Already built; only rebuild if the layer size changed underneath us.
		CGSize size = self.layer.bounds.size;
		CGFloat scale = self.contentScaleFactor;
		int w = static_cast<int>(size.width * scale), h = static_cast<int>(size.height * scale);
		if (w == _backingWidth && h == _backingHeight)
			return YES;
		[self destroyFramebuffer];
	}

	_context = context;
	[EAGLContext setCurrentContext:_context];

	glGenFramebuffersOES(1, &_framebuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);

	glGenRenderbuffersOES(1, &_colorRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _colorRenderbuffer);
	if (![_context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer *)self.layer])
	{
		[self destroyFramebuffer];
		return NO;
	}

	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &_backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &_backingHeight);

	// Depth first at 24 bits, falling back to the ES minimum of 16.
	glGenRenderbuffersOES(1, &_depthRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT24_OES, _backingWidth, _backingHeight);
	if (glGetError() != GL_NO_ERROR)
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, _backingWidth, _backingHeight);

	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,
	                             GL_RENDERBUFFER_OES, _colorRenderbuffer);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES,
	                             GL_RENDERBUFFER_OES, _depthRenderbuffer);

	GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	if (status != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		NSLog(@"EAGLView: framebuffer incomplete: 0x%x", status);
		[self destroyFramebuffer];
		return NO;
	}

	// Leave the framebuffer bound: this is the only render target the game
	// ever uses, and ES has no default framebuffer behind FBO 0 — drawing
	// while unbound goes nowhere (black screen).
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);

	iosapp::notifySurfaceResized(_backingWidth, _backingHeight);
	return YES;
}

- (void)destroyFramebuffer
{
	if (!_context)
		return;
	[EAGLContext setCurrentContext:_context];
	if (_framebuffer)
	{
		glDeleteFramebuffersOES(1, &_framebuffer);
		_framebuffer = 0;
	}
	if (_colorRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &_colorRenderbuffer);
		_colorRenderbuffer = 0;
	}
	if (_depthRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &_depthRenderbuffer);
		_depthRenderbuffer = 0;
	}
	_backingWidth = 0;
	_backingHeight = 0;
	_context = nil;
}

- (void)presentFrame
{
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, _colorRenderbuffer);
	[_context presentRenderbuffer:GL_RENDERBUFFER_OES];

	// Re-arm for the next frame: presentRenderbuffer invalidates the
	// renderbuffer binding, and the game thread draws immediately after this
	// returns — the framebuffer must already be attached when it does.
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, _framebuffer);
}

- (void)layoutSubviews
{
	[super layoutSubviews];
	[self positionOverlayButtons];
	// NOTE: no framebuffer rebuild here on purpose — layout runs on the main
	// thread and the EAGL context is owned by the game thread. Rotation is
	// disabled in Info.plist so the surface never changes under us.
}

// ── Overlay buttons ──────────────────────────────────────────────────────────

static const int KEY_ESC = 0x01;
static const int KEY_T = 0x14;
static const int KEY_E = 0x12;
static const int KEY_W = 0x11, KEY_A = 0x1E, KEY_S = 0x1F, KEY_D = 0x20;
static const int KEY_SPACE = 0x39;
static const int KEY_F3 = 0x3D;
static const int KEY_LSHIFT = 0x2A;
static const int KEY_DIGIT1 = 0x02; // .. through 0 at 0x0B
static const int KEY_G = 0x22;

// Touch coordinates arrive in view points; the GL surface is scaled from
// that by renderScale (0.75 on iPhone, 1.0 on iPad retina).
static CGFloat g_renderScale = 0.75f;
static inline int txX(CGFloat v) { return static_cast<int>(v * g_renderScale); }
static inline int txY(CGFloat v) { return static_cast<int>(v * g_renderScale); }

UIButton *_menuButton = nil;
UIButton *_invButton = nil;
UIButton *_jumpButton = nil;
UIButton *_dbgButton = nil;
UIButton *_chatButton = nil;
UIButton *_kbButton = nil;
UIButton *_crouchButton = nil;
UIButton *_chunkButton = nil;
UIButton *_hotbar[10] = {nil};
UILabel *_hudLabel = nil;
UIView *_stickPad = nil;
UIView *_stickKnob = nil;
UIButton *_hotbarHit[9] = {nil}; // invisible enlarged touch zones over cells
CGRect _stickFrame = CGRectZero;
bool _crouchOn = false;

// ── Minecraft-style widget look ─────────────────────────────────────────────
// The overlay used to be flat white-bordered rounded rectangles. Classic MC
// buttons are square, dark-outlined, with a light-to-dark beveled face and a
// pixel-ish monospace label with a hard shadow.
static UIColor *mcColor(float v, float a) { return [UIColor colorWithWhite:v alpha:a]; }

static UIFont *mcFont(void)
{
	return [UIFont boldSystemFontOfSize:11.0f];
}

static UIImage *mcButtonBackground(void)
{
	static UIImage *img = nil;
	if (img == nil)
	{
		CGSize size = CGSizeMake(1.0f, 20.0f);
		UIGraphicsBeginImageContext(size);
		CGContextRef ctx = UIGraphicsGetCurrentContext();
		CGColorSpaceRef space = CGColorSpaceCreateDeviceGray();
		CGFloat comps[4] = {0.60f, 0.60f, 0.34f, 0.34f}; // top light, bottom dark
		CGFloat locs[2] = {0.0f, 1.0f};
		CGGradientRef grad = CGGradientCreateWithColorComponents(space, comps, locs, 2);
		CGContextDrawLinearGradient(ctx, grad, CGPointMake(0.0f, 0.0f), CGPointMake(0.0f, 20.0f), 0);
		CGGradientRelease(grad);
		CGColorSpaceRelease(space);
		img = [[UIGraphicsGetImageFromCurrentImageContext() stretchableImageWithLeftCapWidth:0 topCapHeight:0] retain];
		UIGraphicsEndImageContext();
	}
	return img;
}

static void styleMcButton(UIButton *b, bool slot)
{
	b.layer.cornerRadius = 0.0f;
	b.clipsToBounds = YES;
	b.layer.borderWidth = 2.0f;
	b.titleLabel.font = mcFont();
	[b setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
	[b setTitleShadowColor:mcColor(0.06f, 1.0f) forState:UIControlStateNormal];
	b.titleLabel.shadowOffset = CGSizeMake(1.0f, 1.0f);

	if (slot)
	{
		b.layer.borderColor = [mcColor(0.30f, 1.0f) CGColor];
		b.backgroundColor = mcColor(0.14f, 0.55f);
		[b setBackgroundImage:nil forState:UIControlStateNormal];
	}
	else
	{
		b.layer.borderColor = [mcColor(0.08f, 1.0f) CGColor];
		b.backgroundColor = mcColor(0.40f, 1.0f);
		[b setBackgroundImage:mcButtonBackground() forState:UIControlStateNormal];
	}
}

- (void)keyTap:(int)key
{
	lwjgl::Keyboard::detail::pushLwjglKey(key, true);
	lwjgl::Keyboard::detail::pushLwjglKey(key, false);
}

- (void)setupOverlayButtons
{
	_menuButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_menuButton setTitle:@"MENU" forState:UIControlStateNormal];
	_menuButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_invButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_invButton setTitle:@"INV" forState:UIControlStateNormal];
	_invButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_jumpButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_jumpButton setTitle:@"JUMP" forState:UIControlStateNormal];
	_jumpButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_dbgButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_dbgButton setTitle:@"DBG" forState:UIControlStateNormal];
	_dbgButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_chatButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_chatButton setTitle:@"CHAT" forState:UIControlStateNormal];
	_chatButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_kbButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_kbButton setTitle:@"KB" forState:UIControlStateNormal];
	_kbButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_crouchButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_crouchButton setTitle:@"CRCH" forState:UIControlStateNormal];
	_crouchButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	_chunkButton = [UIButton buttonWithType:UIButtonTypeCustom];
	[_chunkButton setTitle:@"CHNKS" forState:UIControlStateNormal];
	_chunkButton.titleLabel.font = [UIFont boldSystemFontOfSize:11];

	// Hotbar: 9 slot selectors + crafting ("…").
	for (int i = 0; i < 10; ++i)
	{
		_hotbar[i] = [UIButton buttonWithType:UIButtonTypeCustom];
		_hotbar[i].titleLabel.font = [UIFont boldSystemFontOfSize:11];
		_hotbar[i].tag = 100 + i;
		NSString *label = (i == 9) ? @"…" : [NSString stringWithFormat:@"%d", i + 1];
		[_hotbar[i] setTitle:label forState:UIControlStateNormal];
	}

	// Joystick visuals: pad + knob, NOT user-interactive (touches are handled
	// by EAGLView itself so the same gesture can also drive the knob).
	_stickPad = [[UIView alloc] initWithFrame:CGRectZero];
	_stickPad.backgroundColor = [UIColor colorWithWhite:0.22f alpha:0.55f];
	_stickPad.layer.cornerRadius = 6.0f;
	_stickPad.layer.borderWidth = 3.0f;
	_stickPad.layer.borderColor = [[UIColor colorWithWhite:0.05f alpha:0.85f] CGColor];
	_stickPad.userInteractionEnabled = NO;
	_stickPad.alpha = 0.7f;
	[self addSubview:_stickPad];
	[_stickPad release];

	_stickKnob = [[UIView alloc] initWithFrame:CGRectZero];
	_stickKnob.backgroundColor = [UIColor colorWithWhite:0.62f alpha:0.9f];
	_stickKnob.layer.cornerRadius = 5.0f;
	_stickKnob.layer.borderWidth = 2.0f;
	_stickKnob.layer.borderColor = [[UIColor colorWithWhite:0.08f alpha:0.9f] CGColor];
	_stickKnob.userInteractionEnabled = NO;
	[self addSubview:_stickKnob];
	[_stickKnob release];

	NSArray *buttons = [NSArray arrayWithObjects:_menuButton, _invButton, _jumpButton,
	                                   _dbgButton, _chatButton, _kbButton,
	                                   _crouchButton, _chunkButton,
	                                   _hotbar[0], _hotbar[1], _hotbar[2], _hotbar[3],
	                                   _hotbar[4], _hotbar[5], _hotbar[6], _hotbar[7],
	                                   _hotbar[8], _hotbar[9], nil];
	for (UIButton *b in buttons)
	{
		const bool slot = (b.tag >= 100 && b.tag <= 108);
		b.alpha = slot ? 0.5f : 0.8f;
		styleMcButton(b, slot);
		if (slot)
		{
			// Aligned hotbar cells: keep them subtle so the game's own drawn
			// hotbar stays readable; just a faint slot outline.
			b.backgroundColor = mcColor(1.0f, 0.06f);
			b.layer.borderWidth = 1.0f;
			b.layer.borderColor = [mcColor(0.0f, 0.35f) CGColor];
			b.titleLabel.font = [UIFont boldSystemFontOfSize:10.0f];
		}
		[b addTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
		if (b == _jumpButton)
		{
			// Only jump is a held key — every other control must not fire SPACE!
			[b addTarget:self action:@selector(jumpDown) forControlEvents:UIControlEventTouchDown];
			[b addTarget:self action:@selector(jumpUp) forControlEvents:UIControlEventTouchUpInside];
			[b addTarget:self action:@selector(jumpUp) forControlEvents:UIControlEventTouchUpOutside];
			[b addTarget:self action:@selector(jumpUp) forControlEvents:UIControlEventTouchCancel];
		}
		[self addSubview:b];
		[b release];
	}

	// Compact HUD (coordinates + fps), updated by the lifecycle timer.
	_hudLabel = [[UILabel alloc] initWithFrame:CGRectZero];
	_hudLabel.font = mcFont();
	_hudLabel.textColor = [UIColor colorWithWhite:1.0f alpha:0.95f];
	_hudLabel.backgroundColor = [UIColor colorWithWhite:0.0f alpha:0.45f];
	_hudLabel.textAlignment = UITextAlignmentLeft;
	_hudLabel.adjustsFontSizeToFitWidth = YES;
	_hudLabel.minimumFontSize = 8.0f;
	_hudLabel.numberOfLines = 1;
	_hudLabel.userInteractionEnabled = NO;
	_hudLabel.hidden = YES;
	[self addSubview:_hudLabel];
	[_hudLabel release];

	// Invisible enlarged hit zones for the tiny aligned hotbar cells.
	for (int i = 0; i < 9; ++i)
	{
		_hotbarHit[i] = [UIButton buttonWithType:UIButtonTypeCustom];
		_hotbarHit[i].tag = 100 + i;
		_hotbarHit[i].backgroundColor = [UIColor clearColor];
		[_hotbarHit[i] addTarget:self action:@selector(buttonPressed:)
			    forControlEvents:UIControlEventTouchUpInside];
		[self addSubview:_hotbarHit[i]];
		[_hotbarHit[i] release];
	}

	_menuButton.tag = 1;
	_invButton.tag = 2;
	_jumpButton.tag = 3;
	_dbgButton.tag = 4;
	_chatButton.tag = 5;
	_kbButton.tag = 6;
	_crouchButton.tag = 7;
	_chunkButton.tag = 8;

	// Overlay visibility follows the game state (menu / world / screen open).
	[NSTimer scheduledTimerWithTimeInterval:0.25
	                                 target:self
	                               selector:@selector(syncOverlayVisibility)
	                               userInfo:nil
	                                repeats:YES];

	// Jump must behave like a held key, not a tap that also fires buttonPressed.
	[_jumpButton removeTarget:self action:@selector(buttonPressed:) forControlEvents:UIControlEventTouchUpInside];
	// The view's touch pipeline owns the jump button (see touchesBegan), so
	// disable its own hit-testing; the visuals stay identical.
	_jumpButton.userInteractionEnabled = NO;

	// Hold-to-mine: a finger that lingers on the camera side must start
	// breaking even if it never moves. touchesMoved cannot see a still
	// finger, so poll the slots on a timer instead.
	[NSTimer scheduledTimerWithTimeInterval:0.08
	                                 target:self
	                               selector:@selector(serviceHoldToMine)
	                               userInfo:nil
	                                repeats:YES];
}

- (void)buttonPressed:(UIButton *)sender
{
	switch (sender.tag)
	{
		case 1: [self keyTap:KEY_ESC]; break;
		case 2: [self keyTap:KEY_E]; break;
		case 4: [self keyTap:KEY_F3]; break;
		case 5: [self keyTap:KEY_T]; break;
		case 6: iosapp::toggleKeyboard(); break;
		case 8: [self toggleChunkBorders]; break;
		case 7:
			_crouchOn = !_crouchOn;
			lwjgl::Keyboard::detail::pushLwjglKey(KEY_LSHIFT, _crouchOn);
			_crouchButton.backgroundColor = _crouchOn
			    ? [UIColor colorWithRed:0.9f green:0.6f blue:0.1f alpha:0.85f]
			    : [UIColor colorWithWhite:0.2f alpha:0.6f];
			break;
		default:
			if (sender.tag >= 100 && sender.tag <= 108)
				[self keyTap:KEY_DIGIT1 + (sender.tag - 100)];
			else if (sender.tag == 109)
				[self keyTap:KEY_E]; // "…": inventory with the 2x2 crafting grid
			break;
	}
}

- (void)toggleChunkBorders
{
	// Emulate F3+G: press F3, tap G while held, release F3. The game's input
	// handler recognizes the combo and toggles the chunk-border overlay
	// without opening the F3 debug screen.
	lwjgl::Keyboard::detail::pushLwjglKey(KEY_F3, true);
	lwjgl::Keyboard::detail::pushLwjglKey(KEY_G, true);
	lwjgl::Keyboard::detail::pushLwjglKey(KEY_G, false);
	lwjgl::Keyboard::detail::pushLwjglKey(KEY_F3, false);
}

- (void)jumpDown
{
	(void)0;
	lwjgl::Keyboard::detail::pushLwjglKey(KEY_SPACE, true);
}

- (void)jumpUp
{
	(void)0;
	lwjgl::Keyboard::detail::pushLwjglKey(KEY_SPACE, false);
}

- (void)positionOverlayButtons
{
	// Layout follows the hand-drawn spec:
	//   top: KB + CHAT center, pause right | right edge: crouch above jump
	//   bottom-left: square joystick | bottom-center-right: 10-slot bar
	CGRect b = self.bounds;
	CGFloat m = 8.0f, s = 44.0f, h = 28.0f;

	_menuButton.frame = CGRectMake(b.size.width - s - m, m, s, h);
	_dbgButton.frame = CGRectMake(b.size.width - s - m, m + h + 6.0f, s, h);
	_invButton.frame = CGRectMake(b.size.width - s - m, m + (h + 6.0f) * 2, s, h);
	_chunkButton.frame = CGRectMake(b.size.width - s - m, m + (h + 6.0f) * 3, s, h);

	_kbButton.frame = CGRectMake(b.size.width * 0.40f, m, s, h);
	_chatButton.frame = CGRectMake(b.size.width * 0.40f + s + 6.0f, m, s, h);

	_jumpButton.frame = CGRectMake(b.size.width - 56.0f - m, b.size.height - 56.0f - m, 56.0f, 56.0f);
	_crouchButton.frame = CGRectMake(b.size.width - 56.0f - m,
	                                b.size.height - 56.0f - m - 56.0f - 6.0f, 56.0f, 56.0f);

	// Joystick: square pad, bottom-left.
	CGFloat stick = 124.0f;
	_stickFrame = CGRectMake(m + 4.0f, b.size.height - stick - m - 4.0f, stick, stick);
	_stickPad.frame = _stickFrame;
	[self recenterStickKnob];

	// Hotbar: align 1:1 with the vanilla-rendered GUI bar. Beta renders it
	// 182x22 GUI-units wide, bottom-center, with 20x22 cells starting 1px in
	// (1px border + 9*20 + 1px). Engine space is the BACKING size (half of
	// view points since the half-res change); convert back via pxToPt.
	CGFloat pxToPt = (_backingWidth > 0 && b.size.width > 0)
	    ? b.size.width / (CGFloat)_backingWidth : 1.0f;
	CGFloat gw = (_backingWidth > 0) ? (CGFloat)_backingWidth : b.size.width;
	CGFloat gh = (_backingHeight > 0) ? (CGFloat)_backingHeight : b.size.height;
	// Must mirror ScaledResolution's auto rule incl. the touch-build floor.
	CGFloat guiScale = 1.0f;
	while (guiScale < 4.0f && gw / (guiScale + 1.0f) >= 280.0f &&
	       gh / (guiScale + 1.0f) >= 158.0f)
		guiScale += 1.0f;
	CGFloat uiw = gw / guiScale, uih = gh / guiScale;
	CGFloat barX = floorf((uiw - 182.0f) / 2.0f);
	CGFloat barY = uih - 22.0f;
	CGFloat gs = guiScale * pxToPt; // GUI units -> view points
	for (int i = 0; i < 9; ++i)
	{
		CGRect cell = CGRectMake((barX + 1.0f + i * 20.0f) * gs, barY * gs,
		                         20.0f * gs, 22.0f * gs);
		_hotbar[i].frame = cell;
		// thumb-sized invisible zone centered on the cell
		CGFloat hw = cell.size.width + 12.0f, hh = cell.size.height + 24.0f;
		_hotbarHit[i].frame = CGRectMake(cell.origin.x + cell.size.width / 2 - hw / 2,
		                                 cell.origin.y + cell.size.height / 2 - hh / 2,
		                                 hw, hh);
	}
	// Crafting "…": its own button right of the bar, thumb-sized.
	_hotbar[9].frame = CGRectMake((barX + 183.0f) * gs + 6.0f,
	                              barY * gs + 11.0f * gs - 20.0f, 40.0f, 40.0f);

	(void)0;
}

- (void)recenterStickKnob
{
	CGRect k = CGRectMake(0, 0, 48.0f, 48.0f);
	k.origin.x = _stickFrame.origin.x + (_stickFrame.size.width - k.size.width) / 2.0f;
	k.origin.y = _stickFrame.origin.y + (_stickFrame.size.height - k.size.height) / 2.0f;
	_stickKnob.frame = k;
}

- (void)setOverlayVisible:(BOOL)visible
{
	float alpha = visible ? 0.45f : 0.0f;
	NSArray *buttons = [NSArray arrayWithObjects:_menuButton, _invButton, _jumpButton,
	                                   _dbgButton, _chatButton, _kbButton,
	                                   _crouchButton, _chunkButton,
	                                   _hotbar[0], _hotbar[1], _hotbar[2], _hotbar[3],
	                                   _hotbar[4], _hotbar[5], _hotbar[6], _hotbar[7],
	                                   _hotbar[8], _hotbar[9], nil];
	for (UIButton *b in buttons)
	{
		// Jump is driven by the view's touch pipeline (reliable release);
		// leave its interaction off so it never swallows those touches.
		if (b != _jumpButton)
			b.userInteractionEnabled = visible;
		b.alpha = alpha;
	}
	_stickPad.alpha = alpha;
	_stickKnob.alpha = visible ? 0.8f : 0.0f;
	_stickPad.userInteractionEnabled = NO;
	_stickKnob.userInteractionEnabled = NO;
}

- (void)syncOverlayVisibility
{
	int mode = iosapp::overlayMode(); // 0 menu, 1 world, 2 screen open
	BOOL world = (mode != 0);
	BOOL screenOpen = (mode == 2);

	// MENU (ESC) and KB stay available everywhere — the title screen needs
	// ESC for the options row and KB for text entry.
	_menuButton.hidden = NO;
	_chatButton.hidden = !world;
	_kbButton.hidden   = NO;
	_invButton.hidden  = !(world && !screenOpen);
	_dbgButton.hidden  = !(world && !screenOpen);
	_chunkButton.hidden = !(world && !screenOpen);
	_jumpButton.hidden = !(mode == 1);
	_crouchButton.hidden = !(mode == 1);
	for (int i = 0; i < 9; ++i)
	{
		_hotbar[i].hidden = !(mode == 1);
		_hotbarHit[i].hidden = !(mode == 1);
	}
	_hotbar[9].hidden = !(mode == 1);
	_stickPad.hidden = !(mode == 1);
	_stickKnob.hidden = !(mode == 1);

	// HUD text is published by the game thread each frame.
	{
		std::string hud = iosapp::hudText();
		_hudLabel.hidden = !world || hud.empty();
		if (!_hudLabel.hidden)
		{
			_hudLabel.text = [NSString stringWithUTF8String:hud.c_str()];
			CGSize fit = [_hudLabel sizeThatFits:CGSizeMake(self.bounds.size.width - 16.0f, 20.0f)];
			_hudLabel.frame = CGRectMake(6.0f, 6.0f, fit.width + 10.0f, 18.0f);
		}
	}
	if (!world && _crouchOn)
	{
		_crouchOn = false;
		lwjgl::Keyboard::detail::pushLwjglKey(KEY_LSHIFT, false);
	}
}

// ── Touch state ──────────────────────────────────────────────────────────────

enum TouchRole
{
	ROLE_NONE = 0,
	ROLE_MENU,     // ungrabbed: direct cursor emulation
	ROLE_JOYSTICK, // grabbed: left-third virtual stick → WASD
	ROLE_LOOK,     // grabbed: camera deltas (+hold-to-mine / tap-to-use)
	ROLE_SECOND,   // grabbed: wheel drag or quick attack tap
	ROLE_JUMP      // grabbed: jump button, owned by the view touch pipeline
};

struct TouchSlot
{
	UITouch *touch;
	int role;
	double startTime;
	CGPoint origin;
	CGPoint last;
	double travel;
	bool miningHeld;

	// Joystick state
	unsigned char stickKeys[4]; // W A S D

	// Second-finger wheel state
	double wheelAccum;
	bool wheelMode;

	// Menu gesture state (inventory/chest GUIs)
	bool shiftMode;       // double-tap -> quick move (shift-click)
	bool rightClickSent;  // long-press -> one right-click
};

static TouchSlot g_slots[4];

// Menu double-tap tracking (second tap within the window = shift-click).
static double g_lastMenuTapTime = 0.0;
static CGPoint g_lastMenuTapPos = {0.0, 0.0};

static double nowSeconds()
{
	return CACurrentMediaTime();
}

static TouchSlot *slotFor(UITouch *t, bool create)
{
	for (int i = 0; i < 4; ++i)
		if (g_slots[i].touch == t)
			return &g_slots[i];
	if (!create)
		return nullptr;
	for (int i = 0; i < 4; ++i)
	{
		if (g_slots[i].touch == nullptr)
		{
			g_slots[i] = TouchSlot{};
			g_slots[i].touch = t;
			return &g_slots[i];
		}
	}
	return nullptr;
}

static void releaseSlot(UITouch *t)
{
	for (int i = 0; i < 4; ++i)
		if (g_slots[i].touch == t)
			g_slots[i] = TouchSlot{};
}

// ── Virtual joystick ─────────────────────────────────────────────────────────

static const CGFloat kJoystickDeadzone = 14.0f;
static const int kStickKeyOrder[4] = {KEY_W, KEY_A, KEY_S, KEY_D};

// Absolute stick: direction = touch position relative to the PAD CENTER.
// The knob visual is clamped to the pad; keys toggle on zone crossings.
static void updateJoystick(TouchSlot &s, CGPoint pos)
{
	float cx = _stickFrame.origin.x + _stickFrame.size.width / 2.0f;
	float cy = _stickFrame.origin.y + _stickFrame.size.height / 2.0f;
	float dx = static_cast<float>(pos.x - cx);
	float dy = static_cast<float>(pos.y - cy);

	// Clamp the knob to the pad bounds.
	CGRect k = _stickKnob.frame;
	CGFloat half = k.size.width / 2.0f;
	CGFloat kx = pos.x - half, ky = pos.y - half;
	CGFloat maxX = _stickFrame.origin.x + _stickFrame.size.width - k.size.width;
	CGFloat maxY = _stickFrame.origin.y + _stickFrame.size.height - k.size.height;
	k.origin.x = kx < _stickFrame.origin.x ? _stickFrame.origin.x : (kx > maxX ? maxX : kx);
	k.origin.y = ky < _stickFrame.origin.y ? _stickFrame.origin.y : (ky > maxY ? maxY : ky);
	_stickKnob.frame = k;

	const bool want[4] = {
		dy < -kJoystickDeadzone, // up    → forward
		dx < -kJoystickDeadzone, // left  → strafe left
		dy >  kJoystickDeadzone, // down  → backward
		dx >  kJoystickDeadzone  // right → strafe right
	};

	for (int i = 0; i < 4; ++i)
	{
		if (want[i] != (s.stickKeys[i] != 0))
		{
			s.stickKeys[i] = want[i] ? 1 : 0;
			lwjgl::Keyboard::detail::pushLwjglKey(kStickKeyOrder[i], want[i]);
		}
	}
}

static void releaseJoystick(TouchSlot &s)
{
	for (int i = 0; i < 4; ++i)
	{
		if (s.stickKeys[i])
		{
			s.stickKeys[i] = 0;
			lwjgl::Keyboard::detail::pushLwjglKey(kStickKeyOrder[i], false);
		}
	}
}

// ── Event conversion helpers ─────────────────────────────────────────────────

static CGPoint pointInSelf(UITouch *t, UIView *view)
{
	CGPoint p = [t locationInView:view];
	if (p.x < 0) p.x = 0;
	if (p.y < 0) p.y = 0;
	if (p.x > view.bounds.size.width) p.x = view.bounds.size.width;
	if (p.y > view.bounds.size.height) p.y = view.bounds.size.height;
	return p;
}

// ── Touch handlers ───────────────────────────────────────────────────────────

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	(void)event;
	BOOL grabbed = lwjgl::Mouse::isGrabbed() ? YES : NO;

	for (UITouch *t in touches)
	{
		TouchSlot *slot = slotFor(t, true);
		if (!slot)
			continue;

		CGPoint p = pointInSelf(t, self);
		slot->startTime = nowSeconds();
		slot->origin = p;
		slot->last = p;
		slot->travel = 0.0;
		slot->miningHeld = false;
		slot->wheelMode = false;
		slot->wheelAccum = 0.0;
		slot->shiftMode = false;
		slot->rightClickSent = false;
		for (int i = 0; i < 4; ++i)
			slot->stickKeys[i] = 0;

		if (!grabbed)
		{
			// Menus: emulate an absolute pointing device.
			BOOL haveMenuTouch = NO;
			for (int i = 0; i < 4; ++i)
				if (g_slots[i].role == ROLE_MENU && g_slots[i].touch != t)
					haveMenuTouch = YES;
			if (haveMenuTouch)
			{
				slot->role = ROLE_SECOND; // ignore extra fingers in menus
				continue;
			}
			slot->role = ROLE_MENU;
			// Double-tap = shift-click (quick move stack between
			// inventory/hotbar). The shift state must be down before the
			// button event reaches GuiContainer.mouseClicked.
			double nowT = nowSeconds();
			double tdx = p.x - g_lastMenuTapPos.x;
			double tdy = p.y - g_lastMenuTapPos.y;
			slot->shiftMode = (nowT - g_lastMenuTapTime < 0.35) &&
			                  (tdx * tdx + tdy * tdy < 900.0);
			g_lastMenuTapTime = 0.0; // consumed; re-armed on release
			if (slot->shiftMode)
				lwjgl::Keyboard::detail::pushLwjglKey(KEY_LSHIFT, true);
			lwjgl::Mouse::detail::pushScreenMotion(txX(p.x), txY(p.y), 0, 0);
			lwjgl::Mouse::detail::pushScreenButton(0, true, txX(p.x), txY(p.y));
			continue;
		}

		// Grabbed gameplay roles. The visible pad claims its touch first;
		// everything else on the left is also stick (muscle memory), the
		// right side is camera.
		// Jump is claimed here (not by the UIButton) so touchesEnded /
		// touchesCancelled always release SPACE — UIButton up-events were
		// never firing, leaving the player permanently jumping.
		if (!_jumpButton.hidden && _jumpButton.frame.size.width > 0 &&
		    CGRectContainsPoint(_jumpButton.frame, p))
		{
			slot->role = ROLE_JUMP;
			[self jumpDown];
			continue;
		}

		BOOL inStick = (_stickFrame.size.width > 0) &&
		    (p.x >= _stickFrame.origin.x - 8.0 && p.y >= _stickFrame.origin.y - 8.0 &&
		     p.x <= _stickFrame.origin.x + _stickFrame.size.width + 8.0 &&
		     p.y <= _stickFrame.origin.y + _stickFrame.size.height + 8.0);
		if (inStick || (p.x < self.bounds.size.width / 3.0 &&
		                p.y > self.bounds.size.height * 0.35))
		{
			slot->role = ROLE_JOYSTICK;
			updateJoystick(*slot, p); // absolute: jumping straight onto a side works
			continue;
		}

		BOOL haveLook = NO;
		for (int i = 0; i < 4; ++i)
			if (g_slots[i].role == ROLE_LOOK && g_slots[i].touch != t)
				haveLook = YES;

		if (!haveLook)
			slot->role = ROLE_LOOK;
		else
			slot->role = ROLE_SECOND;
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	(void)event;
	for (UITouch *t in touches)
	{
		TouchSlot *s = slotFor(t, false);
		if (!s || s->role == ROLE_NONE)
			continue;

		CGPoint p = pointInSelf(t, self);
		double dx = p.x - s->last.x;
		double dy = p.y - s->last.y;
		s->travel += sqrt(dx * dx + dy * dy);

		switch (s->role)
		{
			case ROLE_MENU:
				lwjgl::Mouse::detail::pushScreenMotion(txX(p.x), txY(p.y),
				                                       static_cast<int>(dx), static_cast<int>(dy));
				break;

			case ROLE_JOYSTICK:
				updateJoystick(*s, p);
				break;

			case ROLE_LOOK:
			{
				// Camera deltas, slightly amplified for finger precision.
				lwjgl::Mouse::detail::pushScreenMotion(txX(p.x), txY(p.y),
				                                       static_cast<int>(dx * 1.5), static_cast<int>(dy * 1.5));

				// Hold-to-mine kicks in once the finger has lingered.
				if (!s->miningHeld && s->travel < 24.0 &&
				    nowSeconds() - s->startTime >= 0.30)
				{
					lwjgl::Mouse::detail::pushScreenButton(0, true, txX(p.x), txY(p.y));
					s->miningHeld = true;
				}
				break;
			}

			case ROLE_SECOND:
			{
				if (s->wheelMode || fabs(s->wheelAccum) > 25.0 ||
				    fabs(dy) > fabs(dx) * 1.5)
				{
					s->wheelMode = true;
					s->wheelAccum += dy;
					while (fabs(s->wheelAccum) >= 18.0)
					{
						int dir = s->wheelAccum > 0 ? -1 : 1; // drag down → previous slot
						lwjgl::Mouse::detail::pushWheelDelta(dir);
						s->wheelAccum -= dir > 0 ? -18.0 : 18.0;
					}
				}
				break;
			}

			default:
				break;
		}

		s->last = p;
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	(void)event;
	for (UITouch *t in touches)
	{
		TouchSlot *s = slotFor(t, false);
		if (!s || s->role == ROLE_NONE)
		{
			releaseSlot(t);
			continue;
		}

		CGPoint p = pointInSelf(t, self);
		double duration = nowSeconds() - s->startTime;

		switch (s->role)
		{
			case ROLE_MENU:
				lwjgl::Mouse::detail::pushScreenButton(0, false, txX(p.x), txY(p.y));
				if (s->shiftMode)
					lwjgl::Keyboard::detail::pushLwjglKey(KEY_LSHIFT, false);
				// Arm the double-tap window for a following second tap.
				g_lastMenuTapTime = nowSeconds();
				g_lastMenuTapPos = p;
				break;

			case ROLE_JOYSTICK:
				releaseJoystick(*s);
				[self recenterStickKnob];
				break;

			case ROLE_JUMP:
				[self jumpUp];
				break;

			case ROLE_LOOK:
				if (s->miningHeld)
				{
					lwjgl::Mouse::detail::pushScreenButton(0, false, txX(p.x), txY(p.y));
				}
				else if (duration < 0.20 && s->travel < 12.0)
				{
					// Quick stationary tap → use/place (right-click equivalent).
					lwjgl::Mouse::detail::pushScreenButton(1, true, txX(p.x), txY(p.y));
					lwjgl::Mouse::detail::pushScreenButton(1, false, txX(p.x), txY(p.y));
				}
				break;

			case ROLE_SECOND:
				if (!s->wheelMode && duration < 0.25 && s->travel < 14.0)
				{
					// Quick second-finger tap → attack/mine pulse.
					lwjgl::Mouse::detail::pushScreenButton(0, true, txX(p.x), txY(p.y));
					lwjgl::Mouse::detail::pushScreenButton(0, false, txX(p.x), txY(p.y));
				}
				break;

			default:
				break;
		}

		releaseSlot(t);
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	(void)event;
	for (UITouch *t in touches)
	{
		TouchSlot *s = slotFor(t, false);
		if (!s)
			continue;
		if (s->role == ROLE_MENU)
			lwjgl::Mouse::detail::pushScreenButton(0, false, txX(s->last.x), txY(s->last.y));
		else if (s->role == ROLE_JOYSTICK)
		{
			releaseJoystick(*s);
			[self recenterStickKnob];
		}
		else if (s->role == ROLE_JUMP)
			[self jumpUp];
		else if (s->miningHeld)
			lwjgl::Mouse::detail::pushScreenButton(0, false, txX(s->last.x), txY(s->last.y));
		releaseSlot(t);
	}
}

// Hold-to-mine: a finger that lingers on the camera side must start breaking
// even if it never moves. touchesMoved cannot see a still finger, so this is
// polled from a timer instead.
- (void)serviceHoldToMine
{
	for (int i = 0; i < 4; ++i)
	{
		TouchSlot &s = g_slots[i];
		if (s.touch != nullptr && s.role == ROLE_MENU && !s.rightClickSent &&
		    s.travel < 12.0 && nowSeconds() - s.startTime >= 0.55)
		{
			// Long-press in a GUI = right-click (split stack / place one).
			// Cancel the held left button first so we do not left-click too.
			lwjgl::Mouse::detail::pushScreenButton(0, false, txX(s.last.x), txY(s.last.y));
			lwjgl::Mouse::detail::pushScreenButton(1, true, txX(s.last.x), txY(s.last.y));
			lwjgl::Mouse::detail::pushScreenButton(1, false, txX(s.last.x), txY(s.last.y));
			s.rightClickSent = true;
		}
		if (s.touch == nullptr || s.role != ROLE_LOOK || s.miningHeld)
			continue;
		if (s.travel < 24.0 && nowSeconds() - s.startTime >= 0.30)
		{
			lwjgl::Mouse::detail::pushScreenButton(0, true, txX(s.last.x), txY(s.last.y));
			s.miningHeld = true;
		}
	}
}

@end
