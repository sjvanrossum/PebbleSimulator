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

typedef struct _SimulatorBitmap
{
    uint16_t rowSizeBytes;
    uint16_t infoFlags;
    GRect bounds;
    uint8_t data[0];
} SimulatorBitmap;

typedef struct _SimulatorFontGlyphIndex
{
    uint16_t codepoint;
    uint16_t offset;
} SimulatorFontGlyphIndex;

typedef struct _SimulatorFontGlyphEntry
{
    uint8_t width;
    uint8_t height;
    int8_t top;
    int8_t left;
    int8_t advance;
    uint32_t data[];
} SimulatorFontGlyphEntry;

typedef struct _SimulatorFont
{
    uint8_t version;
    uint8_t max_height;
    uint16_t number_of_glyphs;
    uint16_t wildcard_codepoint;
    SimulatorFontGlyphIndex indices[0];
    SimulatorFontGlyphEntry entries[0];

} SimulatorFont;

typedef struct _SimulatorResource
{
    uint32_t index;
    uint32_t offset;
    uint32_t size;
    uint32_t crc;
} SimulatorResource;

typedef struct _SimulatorResourcePack
{
    uint32_t resourceCount;
    uint32_t crcManifest;
    uint32_t unknown;
    char friendlyVersion[16];
    SimulatorResource resourceManifest[256];
    uint8_t resourceData[0];

} SimulatorResourcePack;

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