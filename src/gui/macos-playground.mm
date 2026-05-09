/*
 * This file is part of MPE Emulator
 * Copyright (C) 2026  Attila M. Magyar
 *
 * MPE Emulator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPE Emulator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#import <Cocoa/Cocoa.h>

#include "gui/macos.hpp"

#include "proxy.hpp"


@interface GUIPlaygroundAppDelegate : NSObject<NSApplicationDelegate>
    {
        MpeEmulator::GUI* gui;
        MpeEmulator::Proxy* proxy;
    }

    @property (strong) NSWindow* window;
    @property (strong) NSTimer* idle_timer;
@end


@implementation GUIPlaygroundAppDelegate

    - (id) init
    {
        if (!(self = [super init])) {
            return nil;
        }

        self->gui = NULL;
        self->proxy = NULL;

        return self;
    }

    - (void) applicationDidFinishLaunching:(NSNotification*)notification
    {
        NSRect frame = NSMakeRect(0.0, 0.0, 1020.0, 640.0);

        self.window = [
            [NSWindow alloc]
            initWithContentRect:frame
            styleMask:(
                NSWindowStyleMaskTitled
                | NSWindowStyleMaskClosable
                | NSWindowStyleMaskResizable
                | NSWindowStyleMaskMiniaturizable
            )
            backing:NSBackingStoreBuffered
            defer:NO
        ];

        [self.window setTitle:@"MPE Emulator GUI Playground"];
        [self.window center];

        NSView* content_view = [[NSView alloc] initWithFrame:frame];
        self.window.contentView = content_view;

        proxy = new MpeEmulator::Proxy();

        gui = new MpeEmulator::GUI(
            NULL,
            NULL,
            (__bridge MpeEmulator::GUI::PlatformWidget)content_view,
            *proxy,
            true
        );
        gui->show();

        [self.window makeKeyAndOrderFront:nil];

        self.idle_timer = [
            NSTimer
            scheduledTimerWithTimeInterval:0.1
            repeats:YES
            block:^(NSTimer* timer) {
                if (proxy != NULL) {
                    proxy->process_messages();
                }
            }
        ];
    }

    - (void) applicationWillTerminate:(NSNotification*)notification
    {
        [self.idle_timer invalidate];

        if (gui != NULL) {
            delete gui;

            gui = NULL;
        }

        if (proxy != NULL) {
            delete proxy;

            proxy = NULL;
        }
    }

    - (BOOL) applicationShouldTerminateAfterLastWindowClosed
        :(NSApplication*)sender
    {
        return YES;
    }

@end


extern "C" void run_gui_playground()
{
    @autoreleasepool {
        NSApplication* app = [NSApplication sharedApplication];
        GUIPlaygroundAppDelegate* delegate = [GUIPlaygroundAppDelegate new];
        [app setDelegate:delegate];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app run];
    }
}
