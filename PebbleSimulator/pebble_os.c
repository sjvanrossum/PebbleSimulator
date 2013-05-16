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

#include "pebble_os.h"
#include "pebble_sim.h"
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>

PblTm pblNow;

GContext gCtx = (GContext) { NULL };
CFMutableArrayRef windowStack;
CFMutableDictionaryRef appTimers;
CFTimeZoneRef startupTimeZone;

void app_timer_applier(const void *key, const void *value, void *context);
void app_callback_loop(CFRunLoopTimerRef timer, void * info);

void animation_init(struct Animation *animation)
{
    // TODO: verify.
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
        .duration_ms = 0
    };
}

void animation_set_delay(struct Animation *animation, uint32_t delay_ms)
{
    // TODO: verify.
    animation->delay_ms = delay_ms;
}

void animation_set_duration(struct Animation *animation, uint32_t duration_ms)
{
    // TODO: verify.
    animation->duration_ms = duration_ms;
}

void animation_set_curve(struct Animation *animation, AnimationCurve curve)
{
    // TODO: verify.
    animation->curve = curve;
}

void animation_set_handlers(struct Animation *animation, AnimationHandlers callbacks, void *context)
{
    // TODO: verify.
    animation->handlers = callbacks;
    animation->context = context;
}

void animation_set_implementation(struct Animation *animation, const AnimationImplementation *implementation)
{
    // TODO: verify.
    animation->implementation = implementation;
}

void *animation_get_context(struct Animation *animation)
{
    // TODO: verify.
    return animation->context;
}

void animation_schedule(struct Animation *animation)
{
    // TODO: figure it out.
}

void animation_unschedule(struct Animation *animation)
{
    // TODO: figure it out.
}

void animation_unschedule_all(void)
{
    // TODO: figure it out.
}

bool animation_is_scheduled(struct Animation *animation)
{
    // TODO: figure it out.
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

void app_timer_applier(const void *key, const void *value, void *context)
{
    CFNumberRef val = (CFNumberRef)value;
    uint32_t baseVal;
    CFNumberGetValue(val, kCFNumberIntType, &baseVal);
    --baseVal;
    CFDictionaryReplaceValue(appTimers, key, CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &(baseVal)));
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
    PebbleAppHandlers * handlers = app_task_ctx->handlers;
    
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

void app_event_loop(AppTaskContextRef app_task_ctx, PebbleAppHandlers *handlers)
{
    // TODO: verify.
    // VERY IMPORTANT.
    SimulatorParams * app_params = (SimulatorParams *)app_task_ctx;
    startupTimeZone = CFTimeZoneCopySystem();
    app_params->setGraphicsContext(&gCtx);
    app_params->handlers = handlers;
    
    appTimers = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, NULL, NULL);
    
    get_time(&pblNow);
    CFRunLoopTimerRef timer = CFRunLoopTimerCreate(kCFAllocatorDefault, CFAbsoluteTimeGetCurrent(), 1.0/1000.0, 0, 0, &app_callback_loop, &((CFRunLoopTimerContext){ .info = app_params }));
    CFRunLoopAddTimer(CFRunLoopGetCurrent(), timer, kCFRunLoopDefaultMode);
    
    if (handlers->init_handler)
        handlers->init_handler(app_task_ctx);
    
    CFRunLoopRun();
    
    if (handlers->deinit_handler)
        handlers->deinit_handler(app_task_ctx);
    
    CFRelease(timer);
    CFRelease(startupTimeZone);
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
            CGContextSetRGBStrokeColor(ctx->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case GColorBlack:
            CGContextSetRGBStrokeColor(ctx->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case GColorWhite:
            CGContextSetRGBStrokeColor(ctx->coreGraphicsContext, 1.0f, 1.0f, 1.0f, 1.0f);
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
            CGContextSetRGBFillColor(ctx->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 0.0f);
            break;
        case GColorBlack:
            CGContextSetRGBFillColor(ctx->coreGraphicsContext, 0.0f, 0.0f, 0.0f, 1.0f);
            break;
        case GColorWhite:
            CGContextSetRGBFillColor(ctx->coreGraphicsContext, 1.0f, 1.0f, 1.0f, 1.0f);
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
    // TODO: figure it out.
}

void graphics_draw_pixel(GContext *ctx, GPoint point)
{
    // TODO: verify.
    CGContextFillRect(ctx->coreGraphicsContext, CGRectMake(point.x, point.y, 1.0f, 1.0f));
}

void graphics_draw_line(GContext *ctx, GPoint p0, GPoint p1)
{
    // TODO: verify.
    CGContextStrokeLineSegments(ctx->coreGraphicsContext, (CGPoint[]) { CGPointMake((CGFloat)p0.x, (CGFloat)p0.y), CGPointMake((CGFloat)p1.x, (CGFloat)p1.y) }, 2);
}

void graphics_fill_rect(GContext *ctx, GRect rect, uint8_t corner_radius, GCornerMask corner_mask)
{
    // TODO: verify.
    
    CGContextFillRect(ctx->coreGraphicsContext, CGRectMake((CGFloat)rect.origin.x, (CGFloat)rect.origin.y, (CGFloat)rect.size.w, (CGFloat)rect.size.h));
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
    CGContextAddPath(ctx->coreGraphicsContext, path);
    CGContextDrawPath(ctx->coreGraphicsContext, kCGPathStroke);
    CGPathRelease(path);
}

void graphics_draw_circle(GContext *ctx, GPoint p, int radius)
{
    // TODO: verify.
    CGContextStrokeEllipseInRect(ctx->coreGraphicsContext, CGRectMake(p.x, p.y, radius, radius));
}

void graphics_fill_circle(GContext *ctx, GPoint p, int radius)
{
    // TODO: verify.
    CGContextFillEllipseInRect(ctx->coreGraphicsContext, CGRectMake(p.x, p.y, radius, radius));

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
    CGContextAddPath(ctx->coreGraphicsContext, path);
    CGContextDrawPath(ctx->coreGraphicsContext, kCGPathStroke);
    CGPathRelease(path);
}

void get_time(PblTm *time)
{
    // TODO: verify.
    CFAbsoluteTime itmNow = CFAbsoluteTimeGetCurrent();
    CFGregorianDate itmGreg = CFAbsoluteTimeGetGregorianDate(itmNow, startupTimeZone);

    *time = (PblTm)
    {
        .tm_sec = (int)itmGreg.second,
        .tm_min = (int)itmGreg.minute,
        .tm_hour = (int)itmGreg.hour,
        .tm_mday = (int)itmGreg.day,
        .tm_mon = (int)itmGreg.month,
        .tm_year = (int)itmGreg.year,
        .tm_wday = (int)CFAbsoluteTimeGetDayOfWeek(itmNow, startupTimeZone),
        .tm_yday = (int)CFAbsoluteTimeGetDayOfYear(itmNow, startupTimeZone),
        .tm_isdst = (int)CFTimeZoneIsDaylightSavingTime(startupTimeZone, itmNow)
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
    
    CGContextAddPath(ctx->coreGraphicsContext, corePath);
    CGContextDrawPath(ctx->coreGraphicsContext, kCGPathStroke);
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
    
    CGContextAddPath(ctx->coreGraphicsContext, corePath);
    CGContextDrawPath(ctx->coreGraphicsContext, kCGPathFill);
    CGPathRelease(corePath);
}

GPoint grect_center_point(GRect *rect)
{
    // TODO: verify.
    return GPoint(rect->origin.x + rect->size.w / 2, rect->origin.y + rect->size.h / 2);
}

void layer_mark_dirty(Layer *layer)
{
    // TODO: figure it out.
}

void layer_remove_from_parent(Layer *child)
{
    // TODO: verify.
    if (child->parent)
    {
        layer_mark_dirty(child->parent);
        if (child->parent->first_child == child)
        {
            child->parent->first_child = child->next_sibling;
        }
        else
        {
            Layer * current = child->parent->first_child;
            while (current && current != child)
            {
                if (current->next_sibling == child)
                {
                    current->next_sibling = child->next_sibling;
                    child->next_sibling = NULL;
                    child->parent = NULL;
                    return;
                }
                current = current->next_sibling;
            }
        }
    }
}

void layer_add_child(Layer *parent, Layer *child)
{
    // TODO: verify.
    child->next_sibling = parent->first_child;
    parent->first_child = child;
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
    layer->frame = frame;
    layer->bounds.size = frame.size;
    layer_mark_dirty(layer);
}

void layer_set_hidden(Layer *layer, bool hidden)
{
    // TODO: verify.
    if (layer->hidden != hidden)
    {
        layer->hidden = hidden;
        layer_mark_dirty(layer);
    }
}

void layer_init(Layer *layer, GRect frame)
{
    // TODO: verify.
    layer->bounds = (GRect)
    {
        .origin = GPointZero,
        .size = frame.size
    };
    layer->frame = frame;
    layer->next_sibling = NULL;
    layer->first_child = NULL;
    layer->parent = NULL;
    layer->update_proc = NULL;
}

void light_enable(bool enable)
{
    // TODO: figure it out.
}

void light_enable_interaction(void)
{
    // TODO: figure it out.
}

void psleep(int millis)
{
    if (millis > 0)
        usleep((useconds_t)millis);
    // TODO: figure it out.
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
    // TODO: free.
}

bool rotbmp_init_container(int resource_id, RotBmpContainer *c)
{
    // TODO: malloc/init.
}

void rotbmp_pair_deinit_container(RotBmpPairContainer *c)
{
    // TODO: free.
}

bool rotbmp_pair_init_container(int white_resource_id, int black_resource_id, RotBmpPairContainer *c)
{
    // TODO: malloc/init.
}

void rotbmp_pair_layer_set_src_ic(RotBmpPairLayer *pair, GPoint ic)
{
    // TODO: figure it out.
}

void rotbmp_pair_layer_set_angle(RotBmpPairLayer *pair, int32_t angle)
{
    // TODO: figure it out.
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
        .click_config_context = malloc(sizeof(ClickConfig*)*NUM_BUTTONS),
        .user_data = NULL,
        .debug_name = debug_name,
    };
    
    ((ClickConfig**)window->click_config_context)[BUTTON_ID_UP] = malloc(sizeof(ClickConfig));
    *((ClickConfig**)window->click_config_context)[BUTTON_ID_UP] = (ClickConfig)
    {
        .context = NULL,
        .click =
        {
            .handler = NULL,
            .repeat_interval_ms = 0
        },
        .multi_click =
        {
            .min = 0,
            .max = 0,
            .last_click_only = 0,
            .handler = NULL,
            .timeout = 0
        },
        .long_click =
        {
            .delay_ms = 0,
            .handler = NULL,
            .release_handler = NULL
        },
        .raw =
        {
            .up_handler = NULL,
            .down_handler = NULL,
            .context = NULL
        }
    };
    
    ((ClickConfig**)window->click_config_context)[BUTTON_ID_SELECT] = malloc(sizeof(ClickConfig));
    *((ClickConfig**)window->click_config_context)[BUTTON_ID_SELECT] = (ClickConfig)
    {
        .context = NULL,
        .click =
        {
            .handler = NULL,
            .repeat_interval_ms = 0
        },
        .multi_click =
        {
            .min = 0,
            .max = 0,
            .last_click_only = 0,
            .handler = NULL,
            .timeout = 0
        },
        .long_click =
        {
            .delay_ms = 0,
            .handler = NULL,
            .release_handler = NULL
        },
        .raw =
        {
            .up_handler = NULL,
            .down_handler = NULL,
            .context = NULL
        }
    };
    
    ((ClickConfig**)window->click_config_context)[BUTTON_ID_DOWN] = malloc(sizeof(ClickConfig));
    *((ClickConfig**)window->click_config_context)[BUTTON_ID_DOWN] = (ClickConfig)
    {
        .context = NULL,
        .click =
        {
            .handler = NULL,
            .repeat_interval_ms = 0
        },
        .multi_click =
        {
            .min = 0,
            .max = 0,
            .last_click_only = 0,
            .handler = NULL,
            .timeout = 0
        },
        .long_click =
        {
            .delay_ms = 0,
            .handler = NULL,
            .release_handler = NULL
        },
        .raw =
        {
            .up_handler = NULL,
            .down_handler = NULL,
            .context = NULL
        }
    };
}

void window_stack_push(Window *window, bool animated)
{
    // TODO: verify.
    //[windowStack addPointer:window];
}

void window_set_click_config_provider(Window *window, ClickConfigProvider click_config_provider)
{
    // TODO: verify.
    window->click_config_provider = click_config_provider;
    click_config_provider((ClickConfig**)window->click_config_context, window);
}

void window_set_background_color(Window *window, GColor background_color)
{
    // TODO: verify.
    window->background_color = background_color;
}

void window_render(Window *window, GContext *ctx)
{
    // TODO: figure it out.
}

void window_set_fullscreen(Window *window, bool enabled)
{
    // TODO: verify.
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
    // TODO: figure it out.
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
    layer->bounds = bounds;
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
    while (current)
    {
        current->parent = NULL;
        current = current->next_sibling;
        current->next_sibling = NULL;
    }
    parent->first_child = NULL;
}

void layer_insert_below_sibling(Layer *layer_to_insert, Layer *below_sibling_layer)
{
    // TODO: figure it out.
}

void layer_insert_above_sibling(Layer *layer_to_insert, Layer *above_sibling_layer)
{
    // TODO: figure it out.
}

bool layer_get_hidden(Layer *layer)
{
    // TODO: verify.
    return layer->hidden;
}

void layer_set_clips(Layer *layer, bool clips)
{
    // TODO: verify.
    layer->clips = clips;
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
    // TODO: figure it out.
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
    // TODO: figure it out.
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
    // TODO: figure it out.
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
    free(((ClickConfig**)window->click_config_context)[BUTTON_ID_UP]);
    free(((ClickConfig**)window->click_config_context)[BUTTON_ID_SELECT]);
    free(((ClickConfig**)window->click_config_context)[BUTTON_ID_DOWN]);
    free(window->click_config_context);
    window->click_config_context = NULL;
}

void window_set_click_config_provider_with_context(Window *window, ClickConfigProvider click_config_provider, void *context)
{
    // TODO: verify.
    window->click_config_context = context;
    window_set_click_config_provider(window, click_config_provider);
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
    return &window->layer; // TODO: verify correctness?
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
    //NSUInteger i = [windowStack count] - 1;
    //Window * window = [windowStack pointerAtIndex:i];
    
    //[windowStack removePointerAtIndex:i];
    
    //return window;
}

void window_stack_pop_all(const bool animated)
{
    // TODO: figure it out.
}

bool window_stack_contains_window(Window *window)
{
    // TODO: verify.
    //return [[windowStack allObjects] containsObject:[NSValue valueWithPointer:window]];
}

Window *window_stack_get_top_window(void)
{
    // TODO: verify.
    //NSUInteger i = [windowStack count] - 1;
    //Window * window = [windowStack pointerAtIndex:i];
    
    //return window;
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
    // TODO: figure it out.
}

DictionaryResult dict_write_begin(DictionaryIterator *iter, uint8_t * const buffer, const uint16_t size)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_data(DictionaryIterator *iter, const uint32_t key, const uint8_t * const data, const uint16_t size)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_cstring(DictionaryIterator *iter, const uint32_t key, const char * const cstring)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_int(DictionaryIterator *iter, const uint32_t key, const void *integer, const uint8_t width_bytes, const bool is_signed)
{
    // TODO: verify.
    switch (width_bytes)
    {
        case 1:
            return is_signed ? dict_write_int8(iter, key, *(int8_t *)integer) : dict_write_uint8(iter, key, *(uint8_t *)integer);
        case 2:
            return is_signed ? dict_write_int8(iter, key, *(int16_t *)integer) : dict_write_uint8(iter, key, *(uint16_t *)integer);
        case 4:
            return is_signed ? dict_write_int8(iter, key, *(int32_t *)integer) : dict_write_uint8(iter, key, *(uint32_t *)integer);
        default:
            return DICT_INVALID_ARGS;
    }
    
}

DictionaryResult dict_write_uint8(DictionaryIterator *iter, const uint32_t key, const uint8_t value)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_uint16(DictionaryIterator *iter, const uint32_t key, const uint16_t value)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_uint32(DictionaryIterator *iter, const uint32_t key, const uint32_t value)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_int8(DictionaryIterator *iter, const uint32_t key, const int8_t value)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_int16(DictionaryIterator *iter, const uint32_t key, const int16_t value)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_int32(DictionaryIterator *iter, const uint32_t key, const int32_t value)
{
    // TODO: figure it out.
}

uint32_t dict_write_end(DictionaryIterator *iter)
{
    // TODO: figure it out.
}

Tuple *dict_read_begin_from_buffer(DictionaryIterator *iter, const uint8_t * const buffer, const uint16_t size)
{
    // TODO: figure it out.
}

Tuple *dict_read_next(DictionaryIterator *iter)
{
    // TODO: verify.
    return ++iter->cursor;
}

Tuple *dict_read_first(DictionaryIterator *iter)
{
    // TODO: verify.
    return iter->dictionary->head;
}

DictionaryResult dict_serialize_tuplets(DictionarySerializeCallback callback, void *context, const uint8_t tuplets_count, const Tuplet * const tuplets)
{
    // TODO: figure it out.
}

DictionaryResult dict_serialize_tuplets_to_buffer(const uint8_t tuplets_count, const Tuplet * const tuplets, uint8_t *buffer, uint32_t *size_in_out)
{
    // TODO: figure it out.
}

DictionaryResult dict_serialize_tuplets_to_buffer_with_iter(const uint8_t tuplets_count, const Tuplet * const tuplets, DictionaryIterator *iter, uint8_t *buffer, uint32_t *size_in_out)
{
    // TODO: figure it out.
}

DictionaryResult dict_write_tuplet(DictionaryIterator *iter, const Tuplet * const tuplet)
{
    // TODO: figure it out.
}

uint32_t dict_calc_buffer_size_from_tuplets(const uint8_t tuplets_count, const Tuplet * const tuplets)
{
    // TODO: figure it out.
}

DictionaryResult dict_merge(DictionaryIterator *dest, uint32_t *dest_max_size_in_out, DictionaryIterator *source, const bool update_existing_keys_only, const DictionaryKeyUpdatedCallback key_callback, void *context)
{
    // TODO: figure it out.
}

Tuple *dict_find(const DictionaryIterator *iter, const uint32_t key)
{
    // TODO: figure it out.
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
    action_bar->icons[button_id] = icon;
}

void action_bar_layer_clear_icon(ActionBarLayer *action_bar, ButtonId button_id)
{
    // TODO: figure it out.
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