//
//  pebble_os.c
//  PebbleSimulator
//
//  Created by Steven van Rossum on 13-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//
//  Notes:
//  - Graphics routines currently implemented using Mac-native graphics routines (Core Graphics), non-portable.
//    Considering Cairo as a replacement.
//  - Other core structures currently implemented using Mac-native structures (Core Foundation), portable to other platforms.
//    Considering standard C and POSIX or C++11 core structures.
//  - 

#include "pebble_sim.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <time.h>
#include <sys/time.h>

PblTm pblNow;

FILE * resourcepack = NULL;
SimulatorGContext gCtx = (SimulatorGContext) { NULL, NULL };

ClickConfig backConfig;
ClickConfig upConfig;
ClickConfig selectConfig;
ClickConfig downConfig;

uint32_t uptime_ms = 0;

CFMutableArrayRef windowStack;
CFMutableDictionaryRef animationCollection;
CFMutableDictionaryRef appTimers;

void animation_update_applier(const void* key, const void *value, void *context);
void animation_unschedule_applier(const void *key, const void *value, void *context);

void app_timer_applier(const void *key, const void *value, void *context);
void app_callback_loop(CFRunLoopTimerRef timer, void * info);

void animation_init(struct Animation *animation)
{
    // TODO: verify.
    if (animation)
    {
        *animation = (Animation)
        {
            .list_node = (ListNode)
            {
                .prev = NULL,
                .next = NULL
            },
            .implementation = NULL,
            .handlers = (AnimationHandlers)
            {
                .started = NULL,
                .stopped = NULL
            },
            .context = NULL,
            .abs_start_time_ms = 0,
            .delay_ms = 0,
            .duration_ms = 250,
            .curve = AnimationCurveEaseInOut,
            .is_completed = false
        };
    }
}

void animation_set_delay(struct Animation *animation, uint32_t delay_ms)
{
    // TODO: verify.
    if (animation)
    {
        animation->delay_ms = delay_ms;
    }
}

void animation_set_duration(struct Animation *animation, uint32_t duration_ms)
{
    // TODO: verify.
    if (animation)
    {
        animation->duration_ms = duration_ms;
    }
}

void animation_set_curve(struct Animation *animation, AnimationCurve curve)
{
    // TODO: verify.
    if (animation)
    {
        animation->curve = curve;
    }
}

void animation_set_handlers(struct Animation *animation, AnimationHandlers callbacks, void *context)
{
    // TODO: verify.
    if (animation)
    {
        animation->handlers = callbacks;
        animation->context = context;
    }
}

void animation_set_implementation(struct Animation *animation, const AnimationImplementation *implementation)
{
    // TODO: verify.
    if (animation)
    {
        animation->implementation = implementation;
    }
}

void *animation_get_context(struct Animation *animation)
{
    // TODO: verify.
    if (animation)
    {
        return animation->context;
    }
    else
    {
        return NULL;
    }
}

void animation_schedule(struct Animation *animation)
{
    // TODO: verify.
    if (animation)
    {
        if (CFDictionaryContainsKey(animationCollection, animation))
        {
            animation_unschedule(animation);
        }
        
        animation->abs_start_time_ms = uptime_ms;
        animation->implementation->setup(animation);
        animation->handlers.started(animation, animation->context);
        uint32_t total_duration = animation->duration_ms + animation->delay_ms;
        CFDictionaryAddValue(animationCollection, animation, CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &total_duration));
    }
}

void animation_unschedule(struct Animation *animation)
{
    // TODO: verify.
    if (CFDictionaryContainsKey(animationCollection, animation))
    {
        CFDictionaryRemoveValue(animationCollection, animation);
        animation->handlers.stopped(animation, animation->is_completed, animation->context);
        animation->implementation->teardown(animation);
        animation->abs_start_time_ms = 0;
    }
}

void animation_unschedule_all(void)
{
    // TODO: verify.
    CFDictionaryApplyFunction(animationCollection, animation_unschedule_applier, NULL);
}

bool animation_is_scheduled(struct Animation *animation)
{
    // TODO: verify.
    if (animation)
    {
        return (bool)CFDictionaryContainsKey(animationCollection, animation);
    }
    else
    {
        return false;
    }
}

AppTimerHandle app_timer_send_event(AppContextRef app_ctx, uint32_t timeout_ms, uint32_t cookie)
{
    // TODO: verify.
    CFNumberRef key = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &cookie);
    CFNumberRef value = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &timeout_ms);
    CFDictionaryAddValue(appTimers, key, value);
    CFRelease(key);
    CFRelease(value);
    return cookie;
}

bool app_timer_cancel_event(AppContextRef app_ctx_ref, AppTimerHandle handle)
{
    // TODO: verify.
    CFNumberRef key = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &handle);
    bool exists = CFDictionaryContainsValue(appTimers, key);
    
    if (exists)
        CFDictionaryRemoveValue(appTimers, key);
    
    CFRelease(key);
    return exists;
}

void animation_unschedule_applier(const void *key, const void *value, void *context)
{
    // TODO: verify.
    Animation* animation = (Animation*)key;
    animation_unschedule(animation);
}


void animation_update_applier(const void *key, const void *value, void *context)
{
    // TODO: verify.
    Animation* animation = (Animation*)value;
    CFNumberRef total_duration_ms = (CFNumberRef)value;
    uint32_t total_duration_ms_val;
    CFNumberGetValue(total_duration_ms, kCFNumberIntType, &total_duration_ms_val);
    ++total_duration_ms_val;
    CFDictionaryReplaceValue(animationCollection, key, CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &total_duration_ms_val));
    
    if (total_duration_ms_val >= animation->delay_ms)
    {
        if (total_duration_ms_val >= animation->delay_ms + animation->duration_ms)
        {
            animation->is_completed = true;
            animation_unschedule(animation);
        }
        else
        {
            animation->implementation->update(animation, total_duration_ms_val - animation->delay_ms);
        }
    }
}

void app_timer_applier(const void *key, const void *value, void *context)
{
    // TODO: verify.
    CFNumberRef val = (CFNumberRef)value;
    uint32_t baseVal;
    CFNumberGetValue(val, kCFNumberIntType, &baseVal);
    --baseVal;
    CFDictionaryReplaceValue(appTimers, key, CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &baseVal));
    if (baseVal == 0)
    {
        uint32_t cookie;
        CFNumberGetValue(key, kCFNumberIntType, &cookie);
        (((SimulatorParams *)context)->handlers->timer_handler)(context, cookie, cookie);
        CFDictionaryRemoveValue(appTimers, key);
    }
}

void app_callback_loop(CFRunLoopTimerRef timer, void * info)
{
    // TODO: verify.
    SimulatorParams * app_task_ctx = (SimulatorParams *)info;
    
    if (app_task_ctx)
    {
        PebbleAppHandlers * handlers = app_task_ctx->handlers;
        
        CFDictionaryApplyFunction(animationCollection, &animation_update_applier, app_task_ctx);
        
        if (handlers->timer_handler)
        {
            CFDictionaryApplyFunction(appTimers, &app_timer_applier, app_task_ctx);
        }
        
        PblTm itmPblNow;
        get_time(&itmPblNow);
        if (handlers->tick_info.tick_handler)
        {
            TimeUnits flags = handlers->tick_info.tick_units;
            TimeUnits units_changed = 0;
            if (flags & YEAR_UNIT && itmPblNow.tm_year != pblNow.tm_year)
                units_changed |= YEAR_UNIT;
            if (flags & MONTH_UNIT && itmPblNow.tm_mon != pblNow.tm_mon)
                units_changed |= MONTH_UNIT;
            if (flags & DAY_UNIT && itmPblNow.tm_mday != pblNow.tm_mday)
                units_changed |= DAY_UNIT;
            if (flags & HOUR_UNIT && itmPblNow.tm_hour != pblNow.tm_hour)
                units_changed |= HOUR_UNIT;
            if (flags & MINUTE_UNIT && itmPblNow.tm_min != pblNow.tm_min)
                units_changed |= MINUTE_UNIT;
            if (flags & SECOND_UNIT && itmPblNow.tm_sec != pblNow.tm_sec)
                units_changed |= SECOND_UNIT;
            
            if (units_changed != 0)
                handlers->tick_info.tick_handler(app_task_ctx, &((PebbleTickEvent) { .tick_time = &itmPblNow, .units_changed = units_changed }));
        }
        pblNow = itmPblNow;
    }
    ++uptime_ms;
}

void app_event_loop(AppTaskContextRef app_task_ctx, PebbleAppHandlers *handlers)
{
    // TODO: verify.
    // VERY IMPORTANT.
    SimulatorParams * app_params = (SimulatorParams *)app_task_ctx;
    if (app_params)
    {
        app_params->setGraphicsContext(&gCtx);
        app_params->handlers = handlers;
    }
    
    animationCollection = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    appTimers = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    
    //CFRunLoopSourceRef graphicsSource = CFRunLoopSourceCreate(kCFAllocatorDefault, 0, 0);
    
    get_time(&pblNow);
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent(), 1.0/1000.0, 0, 0, &app_callback_loop, &((CFRunLoopTimerContext){ .info = app_params }));
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
    
    if (handlers->init_handler)
        handlers->init_handler(app_task_ctx);
    if (handlers->tick_info.tick_handler)
        handlers->tick_info.tick_handler(app_task_ctx, &((PebbleTickEvent) { .tick_time = &pblNow, .units_changed = 0 }));
    
    CFRunLoopRun();
    
    if (handlers->deinit_handler)
        handlers->deinit_handler(app_task_ctx);
    
    CFRelease(timer);
}

bool bmp_init_container(int resource_id, BmpContainer *c)
{
    // TODO: malloc/init.
}

void bmp_deinit_container(BmpContainer *c)
{
    // TODO: free.
}

int32_t cos_lookup(int32_t angle)
{
    // TODO: verify.
    // incorrect.
    return (int32_t)cos((double)angle);
}

GFont fonts_get_system_font(const char *font_key)
{
    // TODO: figure it out.
}

GFont fonts_load_custom_font(ResHandle resource)
{
    // TODO: figure it out.
}

void fonts_unload_custom_font(GFont font)
{
    // TODO: free.
}

void graphics_context_set_stroke_color(GContext *ctx, GColor color)
{
    // TODO: verify.
    switch (color)
    {
        case GColorClear:
            CGContextSetRGBStrokeColor(((SimulatorGContext *)ctx)->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case GColorBlack:
            CGContextSetRGBStrokeColor(((SimulatorGContext *)ctx)->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case GColorWhite:
            CGContextSetRGBStrokeColor(((SimulatorGContext *)ctx)->coreGraphicsContext, 1.0f, 1.0f, 1.0f, 1.0f);
            break;
        default:
            // Just do it, can't touch this.
            break;
    }
}

void graphics_context_set_fill_color(GContext *ctx, GColor color)
{
    // TODO: verify.
    switch (color)
    {
        case GColorClear:
            CGContextSetRGBFillColor(((SimulatorGContext *)ctx)->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case GColorBlack:
            CGContextSetRGBFillColor(((SimulatorGContext *)ctx)->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case GColorWhite:
            CGContextSetRGBFillColor(((SimulatorGContext *)ctx)->coreGraphicsContext, 1.0f, 1.0f, 1.0f, 1.0f);
            break;
        default:
            // Just do it, can't touch this.
            break;
    }
}

void graphics_context_set_text_color(GContext *ctx, GColor color)
{
    // TODO: verify.
    graphics_context_set_fill_color(ctx, color);
}

void graphics_context_set_compositing_mode(GContext *ctx, GCompOp mode)
{
    // TODO: verify.
    ((SimulatorGContext *)ctx)->compositingMode = mode;
}

void graphics_draw_pixel(GContext *ctx, GPoint point)
{
    // TODO: verify.
    CGContextFillRect(((SimulatorGContext *)ctx)->coreGraphicsContext, CGRectMake(point.x, point.y, 1.0f, 1.0f));
}

void graphics_draw_line(GContext *ctx, GPoint p0, GPoint p1)
{
    // TODO: verify.
    CGContextStrokeLineSegments(((SimulatorGContext *)ctx)->coreGraphicsContext, (CGPoint[]) { CGPointMake((CGFloat)p0.x, (CGFloat)p0.y), CGPointMake((CGFloat)p1.x, (CGFloat)p1.y) }, 2);
}

void graphics_fill_rect(GContext *ctx, GRect rect, uint8_t corner_radius, GCornerMask corner_mask)
{
    // TODO: verify.
    
    CGContextFillRect(((SimulatorGContext *)ctx)->coreGraphicsContext, CGRectMake((CGFloat)rect.origin.x, (CGFloat)rect.origin.y, (CGFloat)rect.size.w, (CGFloat)rect.size.h));
    return;
    
    // TODO: fix rounded.
    CGAffineTransform transform = CGAffineTransformMakeTranslation((CGFloat)rect.origin.x, (CGFloat)rect.origin.y);
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, &transform, 0.0f, (CGFloat)corner_radius);
    CGPathAddArcToPoint(path, &transform, 0.0f, 0.0f, (CGFloat)corner_radius, 0.0f, (CGFloat)corner_radius);
    CGPathAddLineToPoint(path, &transform, (CGFloat)rect.size.w - (CGFloat)corner_radius, 0.0f);
    CGPathAddArcToPoint(path, &transform, (CGFloat)rect.size.w, 0.0f, (CGFloat)rect.size.w, (CGFloat)corner_radius, (CGFloat)corner_radius);
    CGPathAddLineToPoint(path, &transform, (CGFloat)rect.size.w, (CGFloat)rect.size.h - (CGFloat)corner_radius);
    CGPathAddArcToPoint(path, &transform, (CGFloat)rect.size.w, (CGFloat)rect.size.h, (CGFloat)rect.size.w - (CGFloat)corner_radius, (CGFloat)rect.size.h, (CGFloat)corner_radius);
    CGPathAddLineToPoint(path, &transform, (CGFloat)corner_radius, (CGFloat)rect.size.h);
    CGPathAddArcToPoint(path, &transform, 0.0f, (CGFloat)rect.size.h, 0.0f, (CGFloat)rect.size.h - (CGFloat)corner_radius, (CGFloat)corner_radius);
    CGPathCloseSubpath(path);
    CGContextAddPath(((SimulatorGContext *)ctx)->coreGraphicsContext, path);
    CGContextDrawPath(((SimulatorGContext *)ctx)->coreGraphicsContext, kCGPathStroke);
    CGPathRelease(path);
}

void graphics_draw_circle(GContext *ctx, GPoint p, int radius)
{
    // TODO: verify.
    CGContextStrokeEllipseInRect(((SimulatorGContext *)ctx)->coreGraphicsContext, CGRectMake(p.x, p.y, radius, radius));
}

void graphics_fill_circle(GContext *ctx, GPoint p, int radius)
{
    // TODO: verify.
    CGContextFillEllipseInRect(((SimulatorGContext *)ctx)->coreGraphicsContext, CGRectMake(p.x, p.y, radius, radius));

}

void graphics_draw_round_rect(GContext *ctx, GRect rect, int radius)
{
    // TODO: verify.
    CGAffineTransform transform = CGAffineTransformMakeTranslation((CGFloat)rect.origin.x, (CGFloat)rect.origin.y);
    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, &transform, 0.0f, (CGFloat)radius);
    CGPathAddArcToPoint(path, &transform, 0.0f, 0.0f, (CGFloat)radius, 0.0f, (CGFloat)radius);
    CGPathAddLineToPoint(path, &transform, (CGFloat)rect.size.w - (CGFloat)radius, 0.0f);
    CGPathAddArcToPoint(path, &transform, (CGFloat)rect.size.w, 0.0f, (CGFloat)rect.size.w, (CGFloat)radius, (CGFloat)radius);
    CGPathAddLineToPoint(path, &transform, (CGFloat)rect.size.w, (CGFloat)rect.size.h - (CGFloat)radius);
    CGPathAddArcToPoint(path, &transform, (CGFloat)rect.size.w, (CGFloat)rect.size.h, (CGFloat)rect.size.w - (CGFloat)radius, (CGFloat)rect.size.h, (CGFloat)radius);
    CGPathAddLineToPoint(path, &transform, (CGFloat)radius, (CGFloat)rect.size.h);
    CGPathAddArcToPoint(path, &transform, 0.0f, (CGFloat)rect.size.h, 0.0f, (CGFloat)rect.size.h - (CGFloat)radius, (CGFloat)radius);
    CGPathCloseSubpath(path);
    CGContextAddPath(((SimulatorGContext *)ctx)->coreGraphicsContext, path);
    CGContextDrawPath(((SimulatorGContext *)ctx)->coreGraphicsContext, kCGPathStroke);
    CGPathRelease(path);
}

void get_time(PblTm *t)
{
    // TODO: verify.
    time_t cur = time(NULL);
    struct tm * itm = localtime(&cur);

    *t = (PblTm)
    {
        .tm_sec = itm->tm_sec,
        .tm_min = itm->tm_min,
        .tm_hour = itm->tm_hour,
        .tm_mday = itm->tm_mday,
        .tm_mon = itm->tm_mon,
        .tm_year = itm->tm_year,
        .tm_wday = itm->tm_wday,
        .tm_yday = itm->tm_yday,
        .tm_isdst = itm->tm_isdst
    };
}

void gpath_init(GPath *path, const GPathInfo *init)
{
    // TODO: verify.
    path->num_points = init->num_points;
    path->points = init->points;
    path->rotation = 0;
    path->offset = GPointZero;
}

void gpath_move_to(GPath *path, GPoint point)
{
    // TODO: verify.
    path->offset = point;
}

void gpath_rotate_to(GPath *path, int32_t angle)
{
    // TODO: verify.
    path->rotation = angle;
}

void gpath_draw_outline(GContext *ctx, GPath *path)
{
    // TODO: verify.
    CGPoint points[path->num_points];
    CGAffineTransform transform = CGAffineTransformConcat(CGAffineTransformMakeTranslation((CGFloat)path->offset.x, (CGFloat)path->offset.y), CGAffineTransformMakeRotation((CGFloat)path->rotation));
    
    CGMutablePathRef corePath = CGPathCreateMutable();
    
    for (int i = 0; i < path->num_points; i++)
        points[i] = CGPointApplyAffineTransform(CGPointMake((CGFloat)path->points[i].x, (CGFloat)path->points[i].y), transform);
    
    CGPathAddLines(corePath, &transform, points, path->num_points);
    CGPathCloseSubpath(corePath);
    
    CGContextAddPath(((SimulatorGContext *)ctx)->coreGraphicsContext, corePath);
    CGContextDrawPath(((SimulatorGContext *)ctx)->coreGraphicsContext, kCGPathStroke);
    CGPathRelease(corePath);
}

void gpath_draw_filled(GContext *ctx, GPath *path)
{
    // TODO: verify.
    CGPoint points[path->num_points];
    CGAffineTransform transform = CGAffineTransformConcat(CGAffineTransformMakeTranslation((CGFloat)path->offset.x, (CGFloat)path->offset.y), CGAffineTransformMakeRotation((CGFloat)path->rotation));
    
    CGMutablePathRef corePath = CGPathCreateMutable();
    
    for (int i = 0; i < path->num_points; i++)
        points[i] = CGPointApplyAffineTransform(CGPointMake((CGFloat)path->points[i].x, (CGFloat)path->points[i].y), transform);
    
    CGPathAddLines(corePath, &transform, points, path->num_points);
    CGPathCloseSubpath(corePath);
    
    CGContextAddPath(((SimulatorGContext *)ctx)->coreGraphicsContext, corePath);
    CGContextDrawPath(((SimulatorGContext *)ctx)->coreGraphicsContext, kCGPathFill);
    CGPathRelease(corePath);
}

GPoint grect_center_point(GRect *rect)
{
    // TODO: verify.
    return GPoint(rect->origin.x + rect->size.w / 2, rect->origin.y + rect->size.h / 2);
}

void layer_mark_dirty(Layer *layer)
{
    // TODO: verify.
    if (layer)
    {
        layer->update_proc(layer, &gCtx);
        Layer * childLayer = layer->first_child;
        while (childLayer)
        {
            layer_mark_dirty(childLayer);
            childLayer = childLayer->next_sibling;
        }
    }
}

void layer_remove_from_parent(Layer *child)
{
    // TODO: verify.
    Layer* parent;
    Layer** current;
    
    if (!child || !child->parent)
        return;
    
    parent = child->parent;
    
    current = &(parent->first_child);
    while (*current && *current != child)
        current = &((*current)->next_sibling);
    
    if (!*current)
        return;
    
    *current = child->next_sibling;
    child->next_sibling = NULL;
    child->parent = NULL;
    child->window = NULL;
    layer_mark_dirty(parent);
}

void layer_add_child(Layer *parent, Layer *child)
{
    // TODO: verify.
    Layer** current;
    
    if (!parent || !child)
        return;
    
    layer_remove_from_parent(child);
    
    current = &parent->first_child;
    
    while (*current)
        current = &((*current)->next_sibling);
    
    *current = child;
    child->next_sibling = NULL;
    child->parent = parent;
    child->window = parent->window;
    layer_mark_dirty(parent);
}

GRect layer_get_frame(Layer *layer)
{
    // TODO: verify.
    return layer->frame;
}

void layer_set_frame(Layer *layer, GRect frame)
{
    // TODO: verify.
    if (grect_equal(&frame, &layer->frame))
        return;
    
    layer->frame = frame;
    layer->bounds.size = frame.size;
    layer_mark_dirty(layer);
}

void layer_set_hidden(Layer *layer, bool hidden)
{
    // TODO: verify.
    if (hidden == layer->hidden)
        return;
    
    layer->hidden = hidden;
    layer_mark_dirty(layer);
}

void layer_init(Layer *layer, GRect frame)
{
    // TODO: verify.
    layer->bounds = (GRect)
    {
        .origin = GPointZero,
        .size = frame.size
    };
    layer->clips = true;
    layer->frame = frame;
    layer->hidden = false;
    layer->next_sibling = NULL;
    layer->first_child = NULL;
    layer->parent = NULL;
    layer->update_proc = NULL;
}

void light_enable(bool enable)
{
    // TODO: verify.
    // Do nothing?
}

void light_enable_interaction(void)
{
    // TODO: verify.
    // Do nothing?
}

void psleep(int millis)
{
    // TODO: verify.
    usleep((useconds_t)millis);
}

void resource_init_current_app(ResVersionHandle version)
{
    // TODO: malloc/init.
}

ResHandle resource_get_handle(uint32_t file_id)
{
    // TODO: figure it out.
}

size_t resource_load(ResHandle h, uint8_t *buffer, size_t max_length)
{
    // TODO: figure it out.
}

size_t resource_load_byte_range(ResHandle h, uint32_t start_bytes, uint8_t *data, size_t num_bytes)
{
    // TODO: figure it out.
}

size_t resource_size(ResHandle h)
{
    // TODO: figure it out.
}

void rotbmp_deinit_container(RotBmpContainer *c)
{
    // TODO: no implementation, deprecated.
}

bool rotbmp_init_container(int resource_id, RotBmpContainer *c)
{
    // TODO: no implementation, deprecated.
    return false;
}

void rotbmp_pair_deinit_container(RotBmpPairContainer *c)
{
    // TODO: no implementation, deprecated.
}

bool rotbmp_pair_init_container(int white_resource_id, int black_resource_id, RotBmpPairContainer *c)
{
    // TODO: no implementation, deprecated.
    return false;
}

void rotbmp_pair_layer_set_src_ic(RotBmpPairLayer *pair, GPoint ic)
{
    // TODO: no implementation, deprecated.
}

void rotbmp_pair_layer_set_angle(RotBmpPairLayer *pair, int32_t angle)
{
    // TODO: no implementation, deprecated.
}

void window_init(Window *window, const char *debug_name)
{
    // TODO: verify.
    *window = (Window)
    {
        .layer = (Layer) { GRectZero, GRectZero, 1, 1, NULL, NULL, NULL, window, NULL },
        .status_bar_icon = NULL,
        .input_handlers = (WindowInputHandlers)
        {
            .buttons =
            {
                .up = NULL,
                .down = NULL
            }
        },
        .window_handlers = (WindowHandlers) { NULL, NULL, NULL, NULL },
        .click_config_provider = NULL,
        .click_config_context = NULL,
        .user_data = NULL,
        .debug_name = debug_name,
    };
}

void window_stack_push(Window *window, bool animated)
{
    // TODO: verify.
    if (window)
    {
        Window * top = window_stack_get_top_window();
        
        if (top)
            top->on_screen = false;
        
        CFArrayAppendValue(windowStack, window);
        window->on_screen = true;
        
        if (window->click_config_provider)
            window->click_config_provider((ClickConfig *[]){ (window->overrides_back_button ? &backConfig : (ClickConfig *)NULL), &upConfig, &selectConfig, &downConfig }, window);
    }
}

void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider)
{
    // TODO: verify.
    window->click_config_provider = click_config_provider;
    
    if (click_config_provider)
        click_config_provider((ClickConfig *[]){ (window->overrides_back_button ? &backConfig : (ClickConfig *)NULL), &upConfig, &selectConfig, &downConfig }, window);
}

void window_set_background_color(Window *window, GColor background_color)
{
    // TODO: verify.
    window->background_color = background_color;
}

void window_render(Window *window, GContext *ctx)
{
    // TODO: figure it out.
    if (window)
    {
        if (window->is_loaded && window->on_screen);
        {
            window->is_render_scheduled = true;
            layer_mark_dirty(&window->layer);
            window->is_render_scheduled = false;
        }
    }
}

void window_set_fullscreen(Window *window, bool enabled)
{
    // TODO: verify.
    if (window)
        window->is_fullscreen = enabled;
}

int32_t sin_lookup(int32_t angle)
{
    // TODO: verify.
    // The force isn't with me.
    return (int32_t)sin((double)angle);
}

void string_format_time(char *ptr, size_t maxsize, const char *format, const PblTm *timeptr)
{
    // TODO: verify.
    struct tm itm = (struct tm)
    {
        .tm_sec = timeptr->tm_sec,
        .tm_min = timeptr->tm_min,
        .tm_hour = timeptr->tm_hour,
        .tm_mday = timeptr->tm_mday,
        .tm_mon = timeptr->tm_mon,
        .tm_year = timeptr->tm_year,
        .tm_wday = timeptr->tm_wday,
        .tm_yday = timeptr->tm_yday,
        .tm_isdst = timeptr->tm_isdst
    };
    strftime(ptr, maxsize, format, &itm);
}

void text_layer_init(TextLayer *text_layer, GRect frame)
{
    // TODO: malloc/init.
}

const char *text_layer_get_text(TextLayer *text_layer)
{
    // TODO: verify.
    return text_layer->text;
}

void text_layer_set_text(TextLayer *text_layer, const char *text)
{
    // TODO: verify.
    text_layer->text = text;
}

void text_layer_set_font(TextLayer *text_layer, GFont font)
{
    // TODO: verify.
    text_layer->font = font;
}

void text_layer_set_text_color(TextLayer *text_layer, GColor color)
{
    // TODO: verify.
    text_layer->text_color = color;
}

void text_layer_set_background_color(TextLayer *text_layer, GColor color)
{
    // TODO: verify.
    text_layer->background_color = color;
}

void vibes_double_pulse(void)
{
    // TODO: verify.
    printf("Double vibe.\n");
}

void vibes_enqueue_custom_pattern(VibePattern pattern)
{
    // TODO: verify.
    for (int i = 0; i < pattern.num_segments; i++)
    {
        printf("%i vibe.\n", pattern.durations[i]);
    }
}

void vibes_long_pulse(void)
{
    // TODO: verify.
    printf("Long vibe.\n");
}

void vibes_short_pulse(void)
{
    // TODO: verify.
    printf("Short vibe.\n");
}

GContext *app_get_current_graphics_context(void)
{
    // TODO: verify.
    return &gCtx;
}

bool clock_is_24h_style(void)
{
    // TODO: verify.
    return true;
}

void property_animation_init_layer_frame(struct PropertyAnimation *property_animation, struct Layer *layer, GRect *from_frame, GRect *to_frame)
{
    // TODO: malloc/init.
}

void text_layer_set_text_alignment(TextLayer *text_layer, GTextAlignment text_alignment)
{
    // TODO: verify.
    text_layer->text_alignment = text_alignment;
}

void graphics_draw_bitmap_in_rect(GContext *ctx, const GBitmap *bitmap, GRect rect)
{
    // TODO: figure it out.
}

void graphics_text_draw(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, const GTextLayoutCacheRef layout)
{
    // TODO: figure it out.
}

void layer_set_bounds(Layer *layer, GRect bounds)
{
    // TODO: verify.
    if (grect_equal(&bounds, &layer->bounds))
        return;
    
    layer->bounds = bounds;
    layer_mark_dirty(layer);
}

GRect layer_get_bounds(Layer *layer)
{
    // TODO: verify.
    return layer->bounds;
}

void layer_set_update_proc(Layer *layer, LayerUpdateProc update_proc)
{
    // TODO: verify.
    layer->update_proc = update_proc;
}

struct Window *layer_get_window(Layer *layer)
{
    // TODO: verify.
    return layer->window;
}

void layer_remove_child_layers(Layer *parent)
{
    // TODO: verify.
    Layer * current = parent->first_child;
    parent->first_child = NULL;
    while (current)
    {
        current->parent = NULL;
        current->window = NULL;
        current = current->next_sibling;
        current->next_sibling = NULL;
    }
    layer_mark_dirty(parent);
}

void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer)
{
    // TODO: verify.
    Layer* parent;
    Layer** current;
    
    if (!layer_to_insert || !below_sibling_layer || !below_sibling_layer->parent)
        return;
    
    parent = below_sibling_layer->parent;
    current = &parent->first_child;
    while (*current && *current != below_sibling_layer)
        current = &((*current)->next_sibling);
    
    if (!*current)
        return;
    
    *current = layer_to_insert;
    layer_to_insert->next_sibling = below_sibling_layer;
    layer_to_insert->parent = parent;
    layer_to_insert->window = parent->window;
    layer_mark_dirty(parent);
}

void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer)
{
    // TODO: verify.
    if (!layer_to_insert || !above_sibling_layer || !above_sibling_layer->parent)
        return;
    
    layer_to_insert->next_sibling = above_sibling_layer->next_sibling;
    above_sibling_layer->next_sibling = layer_to_insert;
    layer_to_insert->parent = above_sibling_layer->parent;
    layer_to_insert->window = layer_to_insert->parent->window;
    layer_mark_dirty(above_sibling_layer->parent);
}

bool layer_get_hidden(Layer *layer)
{
    // TODO: verify.
    return layer->hidden;
}

void layer_set_clips(Layer *layer, bool clips)
{
    // TODO: verify.
    if (clips == layer->clips)
        return;
    
    layer->clips = clips;
    layer_mark_dirty(layer);
}

bool layer_get_clips(Layer *layer)
{
    // TODO: verify.
    return layer->clips;
}

GSize text_layer_get_max_used_size(GContext *ctx, TextLayer *text_layer)
{
    // TODO: figure it out.
}

void text_layer_set_size(TextLayer *text_layer, const GSize max_size)
{
    // TODO: verify.
    text_layer->layer.frame.size = max_size;
    text_layer->layer.bounds.size = max_size;
}

void text_layer_set_overflow_mode(TextLayer *text_layer, GTextOverflowMode line_mode)
{
    // TODO: verify.
    text_layer->overflow_mode = line_mode;
}

GSize graphics_text_layout_get_max_used_size(GContext *ctx, const char *text, const GFont font, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, GTextLayoutCacheRef layout)
{
    // TODO: figure it out.
}

void inverter_layer_init(InverterLayer *inverter, GRect frame)
{
    // TODO: malloc/init.
}

void bitmap_layer_init(BitmapLayer *bitmap_layer, GRect frame)
{
    // TODO: malloc/init.
}

void bitmap_layer_set_bitmap(BitmapLayer *bitmap_layer, const GBitmap *bitmap)
{
    // TODO: verify.
    bitmap_layer->bitmap = bitmap;
}

void bitmap_layer_set_alignment(BitmapLayer *bitmap_layer, GAlign alignment)
{
    // TODO: verify.
    bitmap_layer->alignment = alignment;
}

void bitmap_layer_set_background_color(BitmapLayer *bitmap_layer, GColor color)
{
    // TODO: verify.
    bitmap_layer->background_color = color;
}

void bitmap_layer_set_compositing_mode(BitmapLayer *bitmap_layer, GCompOp mode)
{
    // TODO: verify.
    bitmap_layer->compositing_mode = mode;
}

bool heap_bitmap_init(HeapBitmap *hb, int resource_id)
{
    // TODO: malloc/init.
}

void heap_bitmap_deinit(HeapBitmap *hb)
{
    // TODO: figure it out.
}

ButtonId click_recognizer_get_button_id(ClickRecognizerRef recognizer)
{
    // TODO: figure it out.
}

uint8_t click_number_of_clicks_counted(ClickRecognizerRef recognizer)
{
    // TODO: figure it out.
}

void menu_cell_basic_draw(GContext *ctx, const Layer *cell_layer, const char *title, const char *subtitle, GBitmap *icon)
{
    // TODO: figure it out.
}

void menu_cell_title_draw(GContext *ctx, const Layer *cell_layer, const char *title)
{
    // TODO: figure it out.
}

void menu_cell_basic_header_draw(GContext *ctx, const Layer *cell_layer, const char *title)
{
    // TODO: figure it out.
}

void menu_layer_init(MenuLayer *menu_layer, GRect frame)
{
    // TODO: malloc/init.
}

Layer *menu_layer_get_layer(MenuLayer *menu_layer)
{
    // TODO: figure it out.
    
    // No clue which one to return.
    
    //return &menu_layer->inverter.layer;
    //return &menu_layer->scroll_layer.layer;
    //return &menu_layer->scroll_layer.content_sublayer;
}

void menu_layer_set_callbacks(MenuLayer *menu_layer, void *callback_context, MenuLayerCallbacks callbacks)
{
    // TODO: verify.
    menu_layer->callback_context = callback_context;
    menu_layer->callbacks = callbacks;
}

void menu_layer_set_click_config_onto_window(MenuLayer *menu_layer, struct Window *window)
{
    // TODO: verify.
    window_set_click_config_provider(window, menu_layer->scroll_layer.callbacks.click_config_provider);
}

void menu_layer_set_selected_next(MenuLayer *menu_layer, bool up, MenuRowAlign scroll_align, bool animated)
{
    // TODO: figure it out.
}

void menu_layer_set_selected_index(MenuLayer *menu_layer, MenuIndex index, MenuRowAlign scroll_align, bool animated)
{
    // TODO: figure it out.
}

void menu_layer_reload_data(MenuLayer *menu_layer)
{
    // TODO: figure it out.
}

int16_t menu_index_compare(MenuIndex *a, MenuIndex *b)
{
    // TODO: figure it out.
}

void scroll_layer_init(ScrollLayer *scroll_layer, GRect frame)
{
    // TODO: malloc/init.
}

void scroll_layer_add_child(ScrollLayer *scroll_layer, Layer *child)
{
    // TODO: figure it out.
}

void scroll_layer_set_click_config_onto_window(ScrollLayer *scroll_layer, struct Window *window)
{
    // TODO: figure it out.
    window_set_click_config_provider(window, scroll_layer->callbacks.click_config_provider);
}

void scroll_layer_set_callbacks(ScrollLayer *scroll_layer, ScrollLayerCallbacks callbacks)
{
    // TODO: verify.
    scroll_layer->callbacks = callbacks;
}

void scroll_layer_set_context(ScrollLayer *scroll_layer, void *context)
{
    // TODO: verify.
    scroll_layer->context = context;
}

void scroll_layer_set_content_offset(ScrollLayer *scroll_layer, GPoint offset, bool animated)
{
    // TODO: figure it out.
}

GPoint scroll_layer_get_content_offset(ScrollLayer *scroll_layer)
{
    // TODO: verify.
    return scroll_layer->content_sublayer.bounds.origin;
}

void scroll_layer_set_content_size(ScrollLayer *scroll_layer, GSize size)
{
    // TODO: figure it out.
}

GSize scroll_layer_get_content_size(ScrollLayer *scroll_layer)
{
    // TODO: figure it out.
    return scroll_layer->content_sublayer.frame.size;
}

void scroll_layer_set_frame(ScrollLayer *scroll_layer, GRect frame)
{
    // TODO: figure it out.
    layer_set_frame(&scroll_layer->layer, frame);
}

void scroll_layer_scroll_up_click_handler(ClickRecognizerRef recognizer, ScrollLayer *scroll_layer)
{
    // TODO: figure it out.
}

void scroll_layer_scroll_down_click_handler(ClickRecognizerRef recognizer, ScrollLayer *scroll_layer)
{
    // TODO: figure it out.
}

void simple_menu_layer_init(SimpleMenuLayer *simple_menu, GRect frame, Window *window, const SimpleMenuSection *sections, int num_sections, void *callback_context)
{
    // TODO: malloc/init.
}

Layer *simple_menu_layer_get_layer(SimpleMenuLayer *simple_menu)
{
    // TODO: figure it out.
}

int simple_menu_layer_get_selected_index(SimpleMenuLayer *simple_menu)
{
    // TODO: figure it out.
}

void simple_menu_layer_set_selected_index(SimpleMenuLayer *simple_menu, int index, bool animated)
{
    // TODO: figure it out.
}

void window_deinit(Window *window)
{
    // TODO: verify.
}

void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context)
{
    // TODO: verify.
    window->click_config_context = context;
    window->click_config_provider = click_config_provider;
    
    if (click_config_provider)
        click_config_provider((ClickConfig *[]){ (ClickConfig *)NULL, &upConfig, &selectConfig, &downConfig }, context);
}

ClickConfigProvider window_get_click_config_provider(Window *window)
{
    // TODO: verify.
    return window->click_config_provider;
}

void window_set_window_handlers(Window *window, WindowHandlers handlers)
{
    // TODO: verify.
    window->window_handlers = handlers;
}

struct Layer *window_get_root_layer(Window *window)
{
    // TODO: verify.
    return &(window->layer);
}

bool window_get_fullscreen(Window *window)
{
    // TODO: verify.
    return window->is_fullscreen;
}

void window_set_status_bar_icon(Window *window, const GBitmap *icon)
{
    // TODO: verify.
    window->status_bar_icon = icon;
    if (!window->is_fullscreen)
        layer_mark_dirty(&window->layer);
}

bool window_is_loaded(Window *window)
{
    // TODO: verify.
    return window->is_loaded;
}

Window *window_stack_pop(bool animated)
{
    // TODO: verify.
    CFIndex idx = CFArrayGetCount(windowStack) - 1;
    Window* window = (Window *)CFArrayGetValueAtIndex(windowStack, idx);
    
    CFArrayRemoveValueAtIndex(windowStack, idx);
    
    return window;
}

void window_stack_pop_all(const bool animated)
{
    // TODO: verify.
    CFArrayRemoveAllValues(windowStack);
}

bool window_stack_contains_window(Window *window)
{
    // TODO: verify.
    return CFArrayContainsValue(windowStack, CFRangeMake(0, CFArrayGetCount(windowStack)), window);
}

Window *window_stack_get_top_window(void)
{
    // TODO: verify.
    CFIndex idx = CFArrayGetCount(windowStack) - 1;
    Window* window = (Window *)CFArrayGetValueAtIndex(windowStack, idx);
    
    return window;
}

Window *window_stack_remove(Window *window, bool animated)
{
    // TODO: figure it out.
}

void property_animation_init(struct PropertyAnimation *property_animation, const struct PropertyAnimationImplementation *implementation, void *subject, void *from_value, void *to_value)
{
    // TODO: malloc/init.
}

void property_animation_update_int16(struct PropertyAnimation *property_animation, const uint32_t time_normalized)
{
    // TODO: figure it out.
}

void property_animation_update_gpoint(struct PropertyAnimation *property_animation, const uint32_t time_normalized)
{
    // TODO: figure it out.
}

void property_animation_update_grect(struct PropertyAnimation *property_animation, const uint32_t time_normalized)
{
    // TODO: figure it out.
}

AppMessageResult app_message_register_callbacks(AppMessageCallbacksNode *callbacks_node)
{
    // TODO: figure it out.
}

AppMessageResult app_message_deregister_callbacks(AppMessageCallbacksNode *callbacks_node)
{
    // TODO: figure it out.
}

AppMessageResult app_message_out_get(DictionaryIterator **iter_out)
{
    // TODO: figure it out.
}

AppMessageResult app_message_out_send(void)
{
    // TODO: figure it out.
}

AppMessageResult app_message_out_release(void)
{
    // TODO: figure it out.
}

void app_sync_init(struct AppSync *s, uint8_t *buffer, const uint16_t buffer_size, const Tuplet * const keys_and_initial_values, const uint8_t count, AppSyncTupleChangedCallback tuple_changed_callback, AppSyncErrorCallback error_callback, void *context)
{
    // TODO: malloc/init.
}

void app_sync_deinit(struct AppSync *s)
{
    // TODO: free.
}

AppMessageResult app_sync_set(struct AppSync *s, const Tuplet * const keys_and_values_to_update, const uint8_t count)
{
    // TODO: figure it out.
}

const Tuple *app_sync_get(const struct AppSync *s, const uint32_t key)
{
    // TODO: verify.
    return dict_find(&s->current_iter, key);
}

uint32_t dict_calc_buffer_size(const uint8_t tuple_count, ...)
{
    // TODO: verify.
    uint32_t size = 1 + (tuple_count * 7);
    va_list vargs;
    va_start(vargs, tuple_count);
    for (int i = 0; i < tuple_count; i++)
    {
        size += va_arg(vargs, size_t);
    }
    va_end(vargs);
    return size;
}

DictionaryResult dict_write_begin(DictionaryIterator *iter, uint8_t * const buffer, const uint16_t size)
{
    // TODO: verify.
    if (!iter || !buffer)
        return DICT_INVALID_ARGS;
    
    if (!size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->end = (void *)(buffer + size);
    iter->dictionary = (Dictionary *)buffer;
    iter->dictionary->count = 0;
    iter->cursor = iter->dictionary->head;
    
    return DICT_OK;
}

DictionaryResult dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size)
{
    // TODO: verify.
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_BYTE_ARRAY;
    memcpy(iter->cursor->value->data, data, size);
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_cstring(DictionaryIterator *iter, const uint32_t key, const char * const cstring)
{
    // TODO: verify.
    size_t size = strlen(cstring) + 1;
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_CSTRING;
    memcpy(iter->cursor->value->cstring, cstring, size);
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_int(DictionaryIterator *iter, const uint32_t key, const void *integer, const uint8_t width_bytes, const bool is_signed)
{
    // TODO: verify.
    switch (width_bytes)
    {
        case 1:
            return is_signed ? dict_write_int8(iter, key, *(int8_t *)integer) : dict_write_uint8(iter, key, *(uint8_t *)integer);
        case 2:
            return is_signed ? dict_write_int16(iter, key, *(int16_t *)integer) : dict_write_uint16(iter, key, *(uint16_t *)integer);
        case 4:
            return is_signed ? dict_write_int32(iter, key, *(int32_t *)integer) : dict_write_uint32(iter, key, *(uint32_t *)integer);
        default:
            return DICT_INVALID_ARGS;
    }
    
}

DictionaryResult dict_write_uint8(DictionaryIterator *iter, const uint32_t key, const uint8_t value)
{
    // TODO: verify.
    size_t size = sizeof(value);
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_UINT;
    iter->cursor->value->uint8 = value;
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_uint16(DictionaryIterator *iter, const uint32_t key, const uint16_t value)
{
    // TODO: verify.
    size_t size = sizeof(value);
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_UINT;
    iter->cursor->value->uint16 = value;
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_uint32(DictionaryIterator *iter, const uint32_t key, const uint32_t value)
{
    // TODO: verify.
    size_t size = sizeof(value);
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_UINT;
    iter->cursor->value->uint32 = value;
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_int8(DictionaryIterator *iter, const uint32_t key, const int8_t value)
{
    // TODO: verify.
    size_t size = sizeof(value);
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_INT;
    iter->cursor->value->int8 = value;
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_int16(DictionaryIterator *iter, const uint32_t key, const int16_t value)
{
    // TODO: verify.
    size_t size = sizeof(value);
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_INT;
    iter->cursor->value->int16 = value;
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

DictionaryResult dict_write_int32(DictionaryIterator *iter, const uint32_t key, const int32_t value)
{
    // TODO: verify.
    size_t size = sizeof(value);
    
    if (!iter)
        return DICT_INVALID_ARGS;
    
    if (iter->end < (void *)iter->cursor->value + size)
        return DICT_NOT_ENOUGH_STORAGE;
    
    iter->cursor->key = key;
    iter->cursor->length = size;
    iter->cursor->type = TUPLE_INT;
    iter->cursor->value->int32 = value;
    
    ++iter->dictionary->count;
    iter->cursor = (void*)iter->cursor->value + size;
    return DICT_OK;
}

uint32_t dict_write_end(DictionaryIterator *iter)
{
    // TODO: verify.
    if (!iter)
        return 0;
    
    return (uint32_t)((iter->end = (void*)iter->cursor) - (void *)iter->dictionary);
}

Tuple *dict_read_begin_from_buffer(DictionaryIterator *iter, const uint8_t * const buffer, const uint16_t size)
{
    // TODO: verify.
    if (!iter || !buffer || !size)
        return NULL;
    
    iter->dictionary = (Dictionary *)buffer;
    iter->end = (void *)buffer + size;
    
    return dict_read_first(iter);
}

Tuple *dict_read_next(DictionaryIterator *iter)
{
    // TODO: verify.
    void * newCursor = (void*)iter->cursor->value + iter->cursor->length;
    
    if (newCursor < iter->end)
        return (iter->cursor = newCursor);
    else
        return NULL;
}

Tuple *dict_read_first(DictionaryIterator *iter)
{
    // TODO: verify.
    iter->cursor = iter->dictionary->head;
    return iter->cursor;
}

DictionaryResult dict_serialize_tuplets(DictionarySerializeCallback callback, void *context, const uint8_t tuplets_count, const Tuplet * const tuplets)
{
    // TODO: verify.
    
    if (!tuplets)
        return DICT_INVALID_ARGS;
    
    uint32_t size = dict_calc_buffer_size_from_tuplets(tuplets_count, tuplets);
    
    if (size > UINT16_MAX)
        return DICT_NOT_ENOUGH_STORAGE;
    
    uint8_t buf[size];
    
    DictionaryResult res = dict_serialize_tuplets_to_buffer(tuplets_count, tuplets, buf, &size);
    
    if (res == DICT_OK && callback)
        callback(buf, (uint16_t)size, context);
    
    return res;
}

DictionaryResult dict_serialize_tuplets_to_buffer(const uint8_t tuplets_count, const Tuplet * const tuplets, uint8_t *buffer, uint32_t *size_in_out)
{
    // TODO: verify.
    DictionaryIterator iter;
    
    return dict_serialize_tuplets_to_buffer_with_iter(tuplets_count, tuplets, &iter, buffer, size_in_out);
}

DictionaryResult dict_serialize_tuplets_to_buffer_with_iter(const uint8_t tuplets_count, const Tuplet * const tuplets, DictionaryIterator *iter, uint8_t *buffer, uint32_t *size_in_out)
{
    // TODO: verify.
    if (!size_in_out)
        return DICT_INVALID_ARGS;
    
    DictionaryResult res = dict_write_begin(iter, buffer, *size_in_out);
    int i = 0;
    
    while (i < tuplets_count && res == DICT_OK)
    {
        res = dict_write_tuplet(iter, tuplets + i);
        ++i;
    }
    
    *size_in_out = dict_write_end(iter);
    
    return res;
}

DictionaryResult dict_write_tuplet(DictionaryIterator *iter, const Tuplet * const tuplet)
{
    // TODO: verify.    
    if (!iter || !tuplet)
        return DICT_INVALID_ARGS;
    
    switch (tuplet->type)
    {
        case TUPLE_BYTE_ARRAY:
            return dict_write_data(iter, tuplet->key, tuplet->bytes.data, tuplet->bytes.length);
        case TUPLE_CSTRING:
            return dict_write_cstring(iter, tuplet->key, tuplet->cstring.data);
        case TUPLE_INT:
        case TUPLE_UINT:
            return dict_write_int(iter, tuplet->key, &tuplet->integer.storage, (uint8_t)tuplet->integer.width, tuplet->type == TUPLE_INT);
        default:
            return DICT_INVALID_ARGS;
    }
}

uint32_t dict_calc_buffer_size_from_tuplets(const uint8_t tuplets_count, const Tuplet * const tuplets)
{
    // TODO: verify.
    uint32_t size = 1 + (tuplets_count * 7);
    for (int i = 0; i < tuplets_count; i++)
    {
        switch (tuplets[i].type)
        {
            case TUPLE_BYTE_ARRAY:
                size += tuplets[i].bytes.length;
                break;
            case TUPLE_CSTRING:
                size += tuplets[i].cstring.length;
                break;
            case TUPLE_INT:
            case TUPLE_UINT:
                size += tuplets[i].integer.width;
                break;
            default:
                // Can't touch this.
                break;
        }
    }
    return size;
}

DictionaryResult dict_merge(DictionaryIterator *dest, uint32_t *dest_max_size_in_out, DictionaryIterator *source, const bool update_existing_keys_only, const DictionaryKeyUpdatedCallback key_callback, void *context)
{
    // TODO: figure it out.
    if (!dest || !dest_max_size_in_out || !source)
        return DICT_INVALID_ARGS;
    
    Tuple* existingKeys[513];
    Tuple* missingKeys[257];
    
    memset(existingKeys, 0, 513 * sizeof(Tuple*));
    memset(missingKeys, 0, 257 * sizeof(Tuple*));
    
    Tuple* src_t = dict_read_first(source);
    Tuple** existing_iter = existingKeys;
    Tuple** missing_iter = missingKeys;
    
    do {
        Tuple* dst_t;
        if ((dst_t = dict_find(dest, src_t->key)) && dst_t->type == src_t->type && dst_t->length == src_t->length)
        {
            *existing_iter = dst_t;
            ++existing_iter;
            *existing_iter = src_t;
            ++existing_iter;
        }
        else
        {
            *missing_iter = src_t;
            missing_iter++;
        }
    } while ((src_t = dict_read_next(source)));
        
    if (!update_existing_keys_only)
    {
        dest->cursor = (Tuple*)dest->end;
        
    }
    
    existing_iter = existingKeys;
    missing_iter = missingKeys;
    
    while (*existing_iter)
    {
        Tuple cpy_dst_t = **existing_iter;
        Tuple * dst = *(existing_iter++);
        Tuple * src = *(existing_iter++);
        *dst = *src;
        
        if (key_callback)
            key_callback(cpy_dst_t.key, dst, &cpy_dst_t, context);
    }
}

Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key)
{
    // TODO: verify.
    if (!iter)
        return NULL;
    
    DictionaryIterator search_iter;
    Tuple * res = dict_read_begin_from_buffer(&search_iter, (void *)iter->dictionary, iter->end - (void *)iter->dictionary);
    
    do
    {
        if (res->key == key)
            break;
    }
    while ((res = dict_read_next(&search_iter)));
    
    return res;
}

void action_bar_layer_init(ActionBarLayer *action_bar)
{
    // TODO: malloc/init.
}

void action_bar_layer_set_context(ActionBarLayer *action_bar, void *context)
{
    // TODO: verify.
    action_bar->context = context;
}

void action_bar_layer_set_click_config_provider(ActionBarLayer *action_bar, ClickConfigProvider click_config_provider)
{
    // TODO: verify.
    action_bar->click_config_provider = click_config_provider;
}

void action_bar_layer_set_icon(ActionBarLayer *action_bar, ButtonId button_id, const GBitmap *icon)
{
    // TODO: verify.
    if (action_bar)
    {
        int idx;
        
        switch (button_id)
        {
            case BUTTON_ID_UP:
                idx = 0;
                break;
            case BUTTON_ID_SELECT:
                idx = 1;
                break;
            case BUTTON_ID_BACK:
                idx = 2;
                break;
            default:
                idx = -1;
                break;
        }
        
        action_bar->icons[idx] = icon;
        action_bar->click_config_provider((ClickConfig *[]){ (ClickConfig *)NULL, &upConfig, &selectConfig, &downConfig }, action_bar->context);
    }
}

void action_bar_layer_clear_icon(ActionBarLayer *action_bar, ButtonId button_id)
{
    // TODO: verify.
    action_bar_layer_set_icon(action_bar, button_id, NULL);
}

void action_bar_layer_add_to_window(ActionBarLayer *action_bar, struct Window *window)
{
    // TODO: figure it out.
    layer_add_child(window_get_root_layer(window), &action_bar->layer);
}

void action_bar_layer_remove_from_window(ActionBarLayer *action_bar)
{
    // TODO: verify.
    layer_remove_from_parent(&action_bar->layer);
    action_bar->window = NULL;
}

void action_bar_layer_set_background_color(ActionBarLayer *action_bar, GColor background_color)
{
    // TODO: verify.
    action_bar->background_color = background_color;
}

void number_window_init(NumberWindow *numberwindow, const char *label, NumberWindowCallbacks callbacks, void *callback_context)
{
    // TODO: verify.
    numberwindow->window = (Window) { };
    numberwindow->action_bar = (ActionBarLayer) { };
    numberwindow->value_label = (TextLayer) { };
    numberwindow->value_output = (TextLayer) { };
    numberwindow->value = 0;
    numberwindow->max_val = 0;
    numberwindow->min_val = 0;
    numberwindow->step_size = 0;
    numberwindow->is_wrapping_enabled = false;
    numberwindow->callbacks = callbacks;
    numberwindow->callback_context = callback_context;
}

void number_window_set_label(NumberWindow *numberwindow, const char *label)
{
    // TODO: verify.
    numberwindow->value_label.text = label;
}

void number_window_set_max(NumberWindow *numberwindow, int max)
{
    // TODO: verify.
    numberwindow->max_val = max;
}

void number_window_set_min(NumberWindow *numberwindow, int min)
{
    // TODO: verify.
    numberwindow->min_val = min;
}

void number_window_set_value(NumberWindow *numberwindow, int value)
{
    // TODO: verify.
    numberwindow->value = value;
}

void number_window_set_step_size(NumberWindow *numberwindow, int step)
{
    // TODO: verify.
    numberwindow->step_size = step;
}

int number_window_get_value(NumberWindow *numberwindow)
{
    // TODO: verify.
    return numberwindow->value;
}

void clock_copy_time_string(char *buffer, uint8_t size)
{
    // TODO: figure it out.
}

void gbitmap_init_as_sub_bitmap(GBitmap *sub_bitmap, const GBitmap *base_bitmap, GRect sub_rect)
{
    // TODO: figure it out.
}

void gbitmap_init_with_data(GBitmap *bitmap, const uint8_t *data)
{
    // TODO: figure it out.
}

void app_comm_set_sniff_interval(const SniffInterval interval)
{
    // TODO: figure it out.
}

void app_log(uint8_t log_level, const char *src_filename, int src_line_number, const char *fmt, ...)
{
    // TODO: figure it out.
    va_list va;
    char * level;
    switch (log_level)
    {
        case APP_LOG_LEVEL_ERROR:
            level = "error";
            break;
        case APP_LOG_LEVEL_WARNING:
            level = "warning";
            break;
        case APP_LOG_LEVEL_INFO:
            level = "info";
            break;
        case APP_LOG_LEVEL_DEBUG:
            level = "debug";
            break;
        case APP_LOG_LEVEL_DEBUG_VERBOSE:
            level = "verbose";
            break;
        default:
            level = "undefined";
    }
    char fname[strlen(src_filename) + strlen(level) + 5];
    sprintf(fname, "%s_%s.log", src_filename, level);
    FILE * src_file = fopen(fname, "a");
    if (src_file)
    {
        fprintf(src_file, "[\"%s\" logged at line %i]: ", level, src_line_number);
        va_start(va, fmt);
        vfprintf(src_file, fmt, va);
        va_end(va);
        fclose(src_file);
    }
}

void graphics_draw_rect(GContext *ctx, GRect rect)
{
    // TODO: figure it out.
}

void vibes_cancel(void)
{
    // TODO: figure it out.
}

MenuIndex menu_layer_get_selected_index(MenuLayer *menu_layer)
{
    // TODO: verify.
    return menu_layer->selection.index;
}

bool gpoint_equal(const GPoint * const point_a, const GPoint * const point_b)
{
    // TODO: verify.
    return point_a == point_b || (point_a->x == point_b->x && point_a->y == point_b->y);
}

bool grect_contains_point(GRect *rect, GPoint *point)
{
    // TODO: verify.
    return point->x >= rect->origin.x && point->x < rect->origin.x + rect->size.w && point->y >= rect->origin.y && point->y < rect->origin.y + rect->size.h;
}

void grect_align(GRect *rect, const GRect *inside_rect, const GAlign alignment, const bool clip)
{
    // TODO: figure it out.
}

void grect_clip(GRect * const rect_to_clip, const GRect * const rect_clipper)
{
    // TODO: figure it out.
}

GRect grect_crop(GRect rect, const int crop_size_px)
{
    // TODO: verify.
    int16_t crop_size_wh = crop_size_px * 2;
    if (crop_size_wh >= rect.size.h || crop_size_wh >= rect.size.w)
        return GRectZero;
    
    return GRect(rect.origin.x + crop_size_px, rect.origin.y + crop_size_px, rect.size.w - crop_size_wh, rect.size.h - crop_size_wh);
}

bool grect_equal(const GRect * const rect_a, const GRect * const rect_b)
{
    // TODO: verify.
    return rect_a == rect_b || (gpoint_equal((GPoint*const)&rect_a->origin, (GPoint*const)&rect_b->origin) && gsize_equal((GSize*const)&rect_a->size, (GSize*const)&rect_b->size));
}

bool grect_is_empty(const GRect * const rect)
{
    // TODO: verify.
    return rect->size.h == 0 || rect->size.w == 0;
}

void grect_standardize(GRect *rect)
{
    // TODO: verify.
    if (rect->size.h < 0)
        rect->origin.y -= (rect->size.h *= -1);
    
    if (rect->size.w < 0)
        rect->origin.x -= (rect->size.w *= -1);
}

bool gsize_equal(GSize *size_a, GSize *size_b)
{
    // TODO: verify.
    return size_a == size_b || (size_a->h == size_b->h && size_a->w == size_b->w);
}

time_t pbl_override_time(time_t *tloc)
{
    // TODO: nodoc.
    return 0;
}

uint16_t time_ms(time_t *tloc, uint16_t *out_ms)
{
    // TODO: verify.
    struct timeval current;
    gettimeofday(&current, NULL);
    if (tloc)
        *tloc = current.tv_sec;
    
    if (out_ms)
        return (*out_ms = (uint16_t)(current.tv_usec / 1000));
    else
        return (uint16_t)(current.tv_usec / 1000);
}