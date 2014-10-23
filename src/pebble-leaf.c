#include <pebble.h>

static Window *window;
static TextLayer *hvac_label;
static TextLayer *update_label;
static TextLayer *range_layer;

// static BitmapLayer *image_layer;

// static GBitmap *image;
static AppSync sync;
static uint8_t sync_buffer[256];

enum{
  PEBBLE_HVAC_STATE_INDEX = 0x0,
  PEBBLE_REFRESH_STATE_INDEX = 0x1,
  CURRENT_BATTERY = 0x2,
  RANGE = 0x3,
  CHARGE_TIME = 0x4,
  CURRENT_HVAC = 0x5,
  LAST_UPDATE_TIME = 0x6,
  CHARGER_TYPE = 0x7,
  CHARGING = 0x8,
  USE_METRIC = 0x9
};

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  Tuplet value = TupletInteger(0, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  Tuplet value = TupletInteger(1, 1);

  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  if (iter == NULL) {
    return;
  }

  dict_write_tuplet(iter, &value);
  dict_write_end(iter);

  app_message_outbox_send();
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}
char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %s", translate_error(app_message_error));
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case CURRENT_BATTERY:
      APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Received Battery: %d", new_tuple->value->uint8);
      // if (icon_bitmap) {
      //   gbitmap_destroy(icon_bitmap);
      // }
      // icon_bitmap = gbitmap_create_with_resource(WEATHER_ICONS[new_tuple->value->uint8]);
      // bitmap_layer_set_bitmap(icon_layer, icon_bitmap);
      break;
    case RANGE:
      // App Sync keeps new_tuple in sync_buffer, so we may use it directly
      text_layer_set_text(range_layer, new_tuple->value->cstring);
      break;
    case CHARGE_TIME:
      // text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
    case CURRENT_HVAC:
      // text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
    case LAST_UPDATE_TIME:
      // text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
    case CHARGING:
      // text_layer_set_text(city_layer, new_tuple->value->cstring);
      break;
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  // GRect bounds = layer_get_bounds(window_layer);

  GRect bounds = layer_get_frame(window_layer);

  // image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_LITTER);
  // image_layer = bitmap_layer_create(bounds);
  // bitmap_layer_set_bitmap(image_layer, image);
  // bitmap_layer_set_alignment(image_layer, GAlignCenter);
  // layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

  hvac_label = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(hvac_label, "Start HVAC");
  text_layer_set_text_alignment(hvac_label, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(hvac_label));

  update_label = text_layer_create((GRect) { .origin = { 0, 124 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(update_label, "Request Update");
  text_layer_set_text_alignment(update_label, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(update_label));

  range_layer = text_layer_create((GRect) { .origin = { 50, 72 }, .size = { 50, 20 } });
  text_layer_set_text(range_layer, "Request Update");
  text_layer_set_text_alignment(range_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(range_layer));

  Tuplet initial_values[] = {
    TupletInteger(CURRENT_BATTERY, (uint8_t) 0),
    TupletCString(RANGE, "0 miles"),
    TupletCString(CHARGE_TIME, "8 hours"),
    TupletInteger(CURRENT_HVAC, (uint8_t) 0),
    TupletCString(LAST_UPDATE_TIME, "---"),
    TupletCString(CHARGER_TYPE, "?"),
    TupletInteger(CHARGING, (uint8_t) 0),
    TupletInteger(USE_METRIC, (uint8_t) 0),
  };

  app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

}

static void window_unload(Window *window) {
  text_layer_destroy(hvac_label);
  text_layer_destroy(range_layer);
}

 void out_sent_handler(DictionaryIterator *sent, void *context) {
  //  text_layer_set_text(text_layer, "HVAC Started");
 }

 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
 }

 void in_dropped_handler(AppMessageResult reason, void *context) {
   // incoming message dropped
 }

 static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  const int inbound_size = 256;
  const int outbound_size = 256;
  app_message_open(inbound_size, outbound_size);
  app_message_register_outbox_sent(out_sent_handler);
  window_stack_push(window, animated);
}

static void deinit(void) {
  // gbitmap_destroy(image);
  app_sync_deinit(&sync);
  // bitmap_layer_destroy(image_layer);
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();

  deinit();
}
