#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer, *s_date_layer, *s_sunset_layer, *s_sunrise_layer;
static GFont s_time_font, s_date_font, s_sunset_font, s_sunrise_font;
static GBitmap *s_bitmap;
static BitmapLayer *s_bitmap_layer;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  if(tick_time->tm_min % 60 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_recieved_callback(DictionaryIterator *iterator, void *context) {
	static char sunset_buffer[8];
	static char sunrise_buffer[8];
	static char sstotal_buffer[16];
	static char srtotal_buffer[16];
	
	Tuple *sunset_tuple = dict_find(iterator, MESSAGE_KEY_SUNSET);
	snprintf(sunset_buffer, sizeof(sunset_buffer), "%s", sunset_tuple->value->cstring);
	snprintf(sstotal_buffer, sizeof(sstotal_buffer), "%s %s", "Sunset:", sunset_buffer);
	text_layer_set_text(s_sunset_layer, sstotal_buffer);
	
	Tuple *sunrise_tuple = dict_find(iterator, MESSAGE_KEY_SUNRISE);
	snprintf(sunrise_buffer, sizeof(sunrise_buffer), "%s", sunrise_tuple->value->cstring);
	snprintf(srtotal_buffer, sizeof(srtotal_buffer), "%s %s", "Sunrise:", sunrise_buffer);
	text_layer_set_text(s_sunrise_layer, srtotal_buffer);
}
	
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	static char s_buffer[8];
	strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
	text_layer_set_text(s_time_layer, s_buffer);
	
	static char s_date_buffer[16];
	strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %d", tick_time);
	text_layer_set_text(s_date_layer, s_date_buffer);
}
static void main_window_load(Window *window){
  Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
	s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LINE);
	s_bitmap_layer = bitmap_layer_create(
		GRect(12, 95, 120, 1));
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	
  s_time_layer = text_layer_create(
    GRect(0, 0, bounds.size.w, bounds.size.h));
	s_date_layer = text_layer_create(
		GRect(0,60,bounds.size.w, 30));
	s_sunrise_layer = text_layer_create(
		GRect(0, 100, bounds.size.w, bounds.size.h));
	s_sunset_layer = text_layer_create(
		GRect(0, 130, bounds.size.w, bounds.size.h));

	
	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_SANS_BOLD_54));
	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_SANS_24));
	s_sunset_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_SANS_24));
	s_sunrise_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPEN_SANS_24));
	
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
	
	text_layer_set_background_color(s_date_layer, GColorBlack);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	text_layer_set_font(s_date_layer, s_date_font);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
	
	text_layer_set_background_color(s_sunrise_layer, GColorBlack);
	text_layer_set_text_color(s_sunrise_layer, GColorWhite);
	text_layer_set_font(s_sunrise_layer, s_sunrise_font);
	text_layer_set_text_alignment(s_sunrise_layer, GTextAlignmentCenter);
	text_layer_set_text(s_sunrise_layer, "Loading...");
	layer_add_child(window_layer, text_layer_get_layer(s_sunrise_layer));
	
	text_layer_set_background_color(s_sunset_layer, GColorBlack);
	text_layer_set_text_color(s_sunset_layer, GColorWhite);
	text_layer_set_font(s_sunset_layer, s_sunset_font);
	text_layer_set_text_alignment(s_sunset_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_sunset_layer));
	
}

static void main_window_unload(Window *window) {
  fonts_unload_custom_font(s_time_font);
	fonts_unload_custom_font(s_date_font);
	fonts_unload_custom_font(s_sunset_font);
	fonts_unload_custom_font(s_sunrise_font);
	
	gbitmap_destroy(s_bitmap);
	bitmap_layer_destroy(s_bitmap_layer);
	
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_sunset_layer);
	text_layer_destroy(s_sunrise_layer);
	
}

static void init() {
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });  
  window_stack_push(s_main_window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	update_time();
	app_message_register_inbox_received(inbox_recieved_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void){
  init();
  app_event_loop();
  deinit();
}