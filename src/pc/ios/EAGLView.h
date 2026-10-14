#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <QuartzCore/CAEAGLLayer.h>

// ─────────────────────────────────────────────────────────────────────────────
// EAGLView — the GL surface and the entire touch→input mapping.
//
// Rendering: CAEAGLLayer drawable in RGB565 (half the fill bandwidth of
// RGBA8888 — the classic SGX-era optimisation, see PORTING.md §3.7) with a
// 24-bit depth buffer (16-bit makes chunk culling z-fight).
//
// Controls:
//   menus (not grabbed):
//     tap                → left click at position
//   gameplay (grabbed):
//     left-third drag    → virtual joystick → W/A/S/D
//     right area drag    → camera look deltas
//     short tap          → use/place (right click)
//     hold ≥300ms        → mine/attack (left button held)
//     second finger      → hotbar scroll via vertical drag / quick attack tap
//   overlay buttons: MENU(ESC), INV(E), JUMP(SPACE), DBG(F3) are UIKit views.
// ─────────────────────────────────────────────────────────────────────────────

@class EAGLContext;

@interface EAGLView : UIView
{
@public
	EAGLContext *_context;

	GLuint _framebuffer;
	GLuint _colorRenderbuffer;
	GLuint _depthRenderbuffer;
	GLint _backingWidth;
	GLint _backingHeight;
}

- (BOOL)createFramebufferWithContext:(EAGLContext *)context;
- (void)destroyFramebuffer;
- (void)presentFrame;

@end
