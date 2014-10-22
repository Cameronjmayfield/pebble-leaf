#include <pebble.h>

static Window *window;
static TextLayer *hvac_label;
static TextLayer *update_label;

static BitmapLayer *image_layer;

static GBitmap *image;

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {

}

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
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  // GRect bounds = layer_get_bounds(window_layer);

  GRect bounds = layer_get_frame(window_layer);

  image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_LITTER);
  image_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(image_layer, image);
  bitmap_layer_set_alignment(image_layer, GAlignCenter);
  layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));


  hvac_label = text_layer_create((GRect) { .origin = { 0, 10 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(hvac_label, "Start HVAC");
  text_layer_set_text_alignment(hvac_label, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(hvac_label));

  update_label = text_layer_create((GRect) { .origin = { 0, 124 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(update_label, "Request Update");
  text_layer_set_text_alignment(update_label, GTextAlignmentRight);
  layer_add_child(window_layer, text_layer_get_layer(update_label));

}

static void window_unload(Window *window) {
  text_layer_destroy(hvac_label);
  text_layer_destroy(update_label);
}

 void out_sent_handler(DictionaryIterator *sent, void *context) {
  //  text_layer_set_text(text_layer, "HVAC Started");
 }


 void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
   // outgoing message failed
 }


 void in_received_handler(DictionaryIterator *received, void *context) {

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
  const int inbound_size = 64;
  const int outbound_size = 64;
  app_message_open(inbound_size, outbound_size);
  app_message_register_outbox_sent(out_sent_handler);
  window_stack_push(window, animated);
}

static void deinit(void) {
  gbitmap_destroy(image);

  bitmap_layer_destroy(image_layer);
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();

  deinit();
}
