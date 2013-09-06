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

typedef struct _SimulatorGContext {
    CGContextRef coreGraphicsContext;
    GCompOp compositingMode;
} SimulatorGContext;

typedef struct _SimulatorParams
{
    void (*setGraphicsContext)(SimulatorGContext * context);
    void (*setAppHandlers)(PebbleAppHandlers * handlers);
    void (*setWindowStack)(CFMutableArrayRef windowStack);
    CFMutableDictionaryRef (*requestResourceManifest)(void);
    void (*setclickConfigs)(ClickConfig** clickConfigs);
    PebbleAppHandlers * handlers;
    CFRunLoopRef runLoop;
} SimulatorParams;

typedef struct _ClickRecognizer
{
    ButtonId buttonID;
    uint8_t clickCount;
} ClickRecognizer;