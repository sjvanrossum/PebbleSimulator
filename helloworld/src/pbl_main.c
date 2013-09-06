//
//  pbl_main.c
//  helloworld
//
//  Created by Steven van Rossum on 15-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#include "pebble_os.h"

void handle_render(AppContextRef app_ctx, PebbleRenderEvent *event)
{
    graphics_context_set_fill_color(event->ctx, GColorWhite);
    graphics_context_set_stroke_color(event->ctx, GColorWhite);
    graphics_draw_round_rect(event->ctx, GRect(10, 10, 50, 50), 5);
    graphics_draw_circle(event->ctx, GPoint(0, 0), 15);
    graphics_fill_rect(event->ctx, GRect(40, 40, 64, 88), 0, 0);
}

void handle_init(AppContextRef app_ctx)
{
    size_t test_size = 58;
    uint8_t test_data[] = { 0x04, 0xed, 0xff, 0xea, 0xad, 0x00, 0x08, 0x00, 0x22, 0x44, 0x88, 0x11, 0x33, 0x55, 0x77, 0x99, 0xed, 0xdb, 0xde, 0xfa, 0x02, 0x04, 0x00, 0xd4, 0xff, 0xff, 0xff, 0xbe, 0xba, 0xfe, 0xca, 0x02, 0x04, 0x00, 0x16, 0x00, 0x00, 0x00, 0xef, 0xbe, 0xad, 0xde, 0x01, 0x0d, 0x00, 0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x20, 0x57, 0x6f, 0x72, 0x6c, 0x64, 0x21, 0x00 };
    
    uint8_t bytes[8] = { 0x22, 0x44, 0x88, 0x11, 0x33, 0x55, 0x77, 0x99 };
    char * string = "Hello World!";
    Tuplet t[4];
    t[0] = TupletBytes(0xadeaffed, bytes, 8);
    t[1] = TupletInteger(0xfadedbed, (int32_t)-44);
    t[2] = TupletInteger(0xcafebabe, (uint32_t)22);
    t[3] = TupletCString(0xdeadbeef, string);
    APP_LOG(APP_LOG_LEVEL_WARNING, "Hello World!\n");
    printf("iOS NSDictionary serialization:\nMemory buffer contents:\n");
    for (int i = 0; i< test_size; ++i)
        printf("%02x", test_data[i]);
    printf("\nMemory buffer size: %zu bytes.\n\nTuplets serializer:\n", test_size);

    // Tuplets
    uint32_t size = dict_calc_buffer_size_from_tuplets(4, t);
    printf("dict_calc_buffer_size_from_tuplets size test: %s!\n", (size == test_size ? "Succeeded" : "Failed"));

    uint8_t buf[size];
    printf("dict_serialize_tuplets_to_buffer memory test: %s!\n", (dict_serialize_tuplets_to_buffer(4, t, buf, &size) == DICT_OK ? "Succeeded" : "Failed"));
    
    printf("dict_serialize_tuplets_to_buffer size test: %s!\n", (size == test_size ? "Succeeded" : "Failed"));
    printf("Memory buffer contents:\n");
    for (int i = 0; i< test_size; ++i)
        printf("%02x", buf[i]);
    printf("\nMemory buffer size: %u bytes.\n", size);
    printf("Memory comparison test: %s!\n\nDictionaryIterator reader:\n", (!memcmp(test_data, buf, test_size) ? "Succeeded" : "Failed"));
    
    DictionaryIterator read_iter;
    dict_read_begin_from_buffer(&read_iter, test_data, test_size);
    do {
        printf("Key: 0x%08x\n", read_iter.cursor->key);
        printf("Value:\n");
        switch (read_iter.cursor->type)
        {
            case TUPLE_BYTE_ARRAY:
                for (int i = 0; i < read_iter.cursor->length; i++)
                    printf("%x", read_iter.cursor->value->data[i]);
                printf("\n");
                break;
            case TUPLE_CSTRING:
                printf("\"%s\"\n", read_iter.cursor->value->cstring);
                break;
            case TUPLE_INT:
                switch (read_iter.cursor->length) {
                    case 1:
                        printf("%i\n", read_iter.cursor->value->int8);
                        break;
                    case 2:
                        printf("%i\n", read_iter.cursor->value->int16);
                        break;
                    case 4:
                        printf("%i\n", read_iter.cursor->value->int32);
                        break;
                    default:
                        printf("Error\n");
                }
                break;
            case TUPLE_UINT:
                switch (read_iter.cursor->length) {
                    case 1:
                        printf("%u\n", read_iter.cursor->value->uint8);
                        break;
                    case 2:
                        printf("%u\n", read_iter.cursor->value->uint16);
                        break;
                    case 4:
                        printf("%u\n", read_iter.cursor->value->uint32);
                        break;
                    default:
                        printf("Error\n");
                }
                break;
        }
    } while (dict_read_next(&read_iter));
    
    // Dict writer.
    memset(buf, 0, test_size);
    printf("\nDictionaryIterator writer:\n");
    DictionaryIterator iter_writer;
    printf("dict_write_begin: %s!\n", (dict_write_begin(&iter_writer, buf, test_size) == DICT_OK ? "Succeeded" : "Failed"));
    printf("dict_write_tuplet: %s!\n", (dict_write_tuplet(&iter_writer, &t[0]) == DICT_OK ? "Succeeded" : "Failed"));
    printf("dict_write_tuplet: %s!\n", (dict_write_tuplet(&iter_writer, &t[1]) == DICT_OK ? "Succeeded" : "Failed"));
    printf("dict_write_tuplet: %s!\n", (dict_write_tuplet(&iter_writer, &t[2]) == DICT_OK ? "Succeeded" : "Failed"));
    printf("dict_write_tuplet: %s!\n", (dict_write_tuplet(&iter_writer, &t[3]) == DICT_OK ? "Succeeded" : "Failed"));
    size = dict_write_end(&iter_writer);
    printf("Memory buffer contents:\n");
    for (int i = 0; i< test_size; ++i)
        printf("%02x", buf[i]);
    printf("\nMemory buffer size: %u bytes.\n", size);
    
    printf("Memory comparison test: %s!\n", (!memcmp(test_data, buf, test_size) ? "Succeeded" : "Failed"));
    
    DictionaryIterator iter;
    dict_read_begin_from_buffer(&iter, buf, size);
    
    //app_timer_send_event(app_ctx, 5000, 5313178);
}

void handle_tick(AppContextRef app_ctx, PebbleTickEvent * event)
{
    PblTm tm;
    get_time(&tm);
    char buf[256];
    char * fmt = "%X %x";
    string_format_time(buf, 256, fmt, &tm);
    printf("%s\n", buf);
}

void handle_timer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie)
{
    printf("Cookie: %i\n", cookie);
}

void pbl_main(void * params)
{
    PebbleAppHandlers handlers = (PebbleAppHandlers)
    {
        .render_handler = &handle_render,
        /*.tick_info = (PebbleAppTickInfo)
        {
            .tick_handler = &handle_tick,
            .tick_units = SECOND_UNIT
        },*/
        .init_handler = &handle_init,
        .timer_handler = &handle_timer
    };
    app_event_loop(params, &handlers);
}