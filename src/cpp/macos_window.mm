#include <QQuickWindow>
#include <macos_window.h>

#import <AppKit/NSEvent.h>
#import <AppKit/NSWindow.h>

static id s_dragMonitor = nil;

void makeTitleBarTransparent(QQuickWindow *window)
{
    NSView   *view     = (__bridge NSView *)window->winId();
    NSWindow *nswindow = [view window];
    [nswindow setTitlebarAppearsTransparent:YES];
    [nswindow setStyleMask:[nswindow styleMask] | NSWindowStyleMaskFullSizeContentView];
    [nswindow setMovableByWindowBackground:YES];

    if (s_dragMonitor)
        return;

    s_dragMonitor =
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskLeftMouseDown
                                              handler:^(NSEvent *event) {
                                                NSWindow *target = [event window];
                                                if (target != nswindow)
                                                    return event;

                                                NSPoint loc     = [event locationInWindow];
                                                NSRect  bounds  = [[target contentView] bounds];
                                                CGFloat fromTop = bounds.size.height - loc.y;

                                                // Only the top 38 pixels, and avoid the
                                                // traffic-light region (left ~70 px)
                                                if (fromTop <= 38.0 && loc.x > 72.0)
                                                {
                                                    [target performWindowDragWithEvent:event];
                                                    return (NSEvent *)nil;
                                                }
                                                return event;
                                              }];
}
