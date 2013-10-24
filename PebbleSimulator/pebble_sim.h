//
//  pebble_sim.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 14-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include "pebble_os.h"

const CFStringRef kSRPebbleButtonDown;
const CFStringRef kSRPebbleButtonUp;

typedef struct _SimulatorGContext {
    CGContextRef coreGraphicsContext;
    GCompOp compositingMode;
} SimulatorGContext;

typedef struct _SimulatorParams
{
    PebbleAppHandlers * handlers;
    CFMutableArrayRef windowStack;
    CFDictionaryRef resourceManifest;
    CFMutableDictionaryRef animationCollection;
    CFMutableDictionaryRef appTimers;
    
    SimulatorGContext graphicsContext;
    CFRunLoopRef runLoop;
    bool displayDirty;
    void * backButton;
    void * upButton;
    void * selectButton;
    void * downButton;
    void (*redisplay)();
    
    ClickConfig backConfig;
    ClickConfig upConfig;
    ClickConfig selectConfig;
    ClickConfig downConfig;
    
    uint32_t uptime_ms;
    PblTm pblNow;
} SimulatorParams;

typedef struct _ClickRecognizer
{
    ButtonId buttonID;
    uint8_t clickCount;
} ClickRecognizer;