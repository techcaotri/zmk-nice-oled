/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <dt-bindings/zmk/modifiers.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>

#include "modifiers.h"

struct modifiers_state {
  uint8_t modifiers;
};

struct modifier_symbol {
  uint8_t modifier;
  const lv_img_dsc_t *symbol_dsc;
  lv_obj_t *symbol;
  lv_obj_t *selection_line;
  bool is_active;
};

LV_IMG_DECLARE(control_icon);
struct modifier_symbol ms_control = {
    .modifier = MOD_LCTL | MOD_RCTL,
    .symbol_dsc = &control_icon,
};

LV_IMG_DECLARE(shift_icon);
struct modifier_symbol ms_shift = {
    .modifier = MOD_LSFT | MOD_RSFT,
    .symbol_dsc = &shift_icon,
};

LV_IMG_DECLARE(alt_icon);
struct modifier_symbol ms_alt = {
    .modifier = MOD_LALT | MOD_RALT,
    .symbol_dsc = &alt_icon,
};

LV_IMG_DECLARE(gui_icon);
struct modifier_symbol ms_gui = {
    .modifier = MOD_LGUI | MOD_RGUI,
    .symbol_dsc = &gui_icon,
};

static char *modifier_symbol_names[] = {
    // this order determines the order of the symbols
    "Control", "GUI", "Shift", "Alt"};

struct modifier_symbol *modifier_symbols[] = {
    // this order determines the order of the symbols
    &ms_control, &ms_gui, &ms_shift, &ms_alt};

#define NUM_SYMBOLS                                                            \
  (sizeof(modifier_symbols) / sizeof(struct modifier_symbol *))

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static void anim_y_cb(void *var, int32_t v) {
  LOG_DBG("anim_y_cb - var: %p, v: %d", var, v);
  lv_obj_set_y(var, v); 
}

static void move_object_y(void *obj, int32_t from, int32_t to) {
  static lv_anim_t a;
  LOG_DBG("move_object_y obj: %p, from: %d, to: %d", obj, from, to);
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_time(&a, 100); // will be replaced with lv_anim_set_duration
  lv_anim_set_exec_cb(&a, anim_y_cb);
  lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
  lv_anim_set_values(&a, from, to);
  lv_anim_start(&a);
}

static void set_modifiers(lv_obj_t *widget, struct modifiers_state state) {
  for (int i = 0; i < NUM_SYMBOLS; i++) {
    LOG_DBG("modifier_symbols[%s]->symbol: %p", modifier_symbol_names[i], modifier_symbols[i]->symbol);
    // LOG_DBG("modifiers: %d", state.modifiers);
    // LOG_DBG("modifier_symbols[%d]->modifier: %d", i, modifier_symbols[i]->modifier);
    bool mod_is_active = (state.modifiers & modifier_symbols[i]->modifier) > 0;
    // LOG_DBG("mod_is_active: %d", mod_is_active);
    // LOG_DBG("modifier_symbols[%d]->is_active: %d", i, modifier_symbols[i]->is_active);

    // if (mod_is_active && !modifier_symbols[i]->is_active) {
    //   // lv_obj_clear_flag(modifier_symbols[i]->selection_line, LV_OBJ_FLAG_HIDDEN);
    //   // lv_obj_set_style_opa(modifier_symbols[i]->selection_line, LV_OPA_COVER, 0);
    //   // move_object_y(modifier_symbols[i]->symbol, 0, 3);
    //   lv_obj_set_y(modifier_symbols[i]->symbol, 3);
    //   // move_object_y(modifier_symbols[i]->selection_line, SIZE_SYMBOLS + 2,
    //   //               SIZE_SYMBOLS + 4);
    //   LOG_DBG("update modifier_symbols[%d]->is_active: true", i);
    //   modifier_symbols[i]->is_active = true;
    // } else if (!mod_is_active && modifier_symbols[i]->is_active) {
    //   // lv_obj_add_flag(modifier_symbols[i]->selection_line, LV_OBJ_FLAG_HIDDEN);
    //   // lv_obj_set_style_opa(modifier_symbols[i]->selection_line, LV_OPA_TRANSP, 0);
    //   // move_object_y(modifier_symbols[i]->symbol, 3, 0);
    //   lv_obj_set_y(modifier_symbols[i]->symbol, 0);
    //   // move_object_y(modifier_symbols[i]->selection_line, SIZE_SYMBOLS + 4,
    //   //               SIZE_SYMBOLS + 2);
    //   LOG_DBG("update modifier_symbols[%d]->is_active: false", i);
    //   modifier_symbols[i]->is_active = false;
    // }
    if (mod_is_active) {
      // lv_obj_clear_flag(modifier_symbols[i]->selection_line, LV_OBJ_FLAG_HIDDEN);
      // lv_obj_set_style_opa(modifier_symbols[i]->selection_line, LV_OPA_COVER, 0);
      // move_object_y(modifier_symbols[i]->symbol, 0, 3);
      lv_obj_set_y(modifier_symbols[i]->symbol, 0);
      // move_object_y(modifier_symbols[i]->selection_line, SIZE_SYMBOLS + 2,
      //               SIZE_SYMBOLS + 4);
      LOG_DBG("update modifier_symbols[%s]->is_active: true, y: 0", modifier_symbol_names[i]);
      
      modifier_symbols[i]->is_active = true;
    } else {
      // lv_obj_add_flag(modifier_symbols[i]->selection_line, LV_OBJ_FLAG_HIDDEN);
      // lv_obj_set_style_opa(modifier_symbols[i]->selection_line, LV_OPA_TRANSP, 0);
      // move_object_y(modifier_symbols[i]->symbol, 3, 0);
      lv_obj_set_y(modifier_symbols[i]->symbol, 3);
      // move_object_y(modifier_symbols[i]->selection_line, SIZE_SYMBOLS + 4,
      //               SIZE_SYMBOLS + 2);
      LOG_DBG("update modifier_symbols[%s]->is_active: false, y: 3", modifier_symbol_names[i]);
      modifier_symbols[i]->is_active = false;
    }

    LOG_DBG("TriPham modifier_symbols[%s]->symbol.y: %d", modifier_symbol_names[i],
            lv_obj_get_y(modifier_symbols[i]->symbol));
  }
}

void modifiers_update_cb(struct modifiers_state state) {
  struct zmk_widget_modifiers *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_modifiers(widget->obj, state);
  }
}

static struct modifiers_state modifiers_get_state(const zmk_event_t *eh) {
  return (struct modifiers_state){.modifiers = zmk_hid_get_explicit_mods()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_modifiers, struct modifiers_state,
                            modifiers_update_cb, modifiers_get_state)

ZMK_SUBSCRIPTION(widget_modifiers, zmk_keycode_state_changed);

int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget,
                              lv_obj_t *parent) {
  widget->obj = lv_obj_create(parent);

  lv_obj_set_size(widget->obj, NUM_SYMBOLS * (SIZE_SYMBOLS + 1) + 1,
                  SIZE_SYMBOLS + 3);

  static lv_style_t style_line;
  lv_style_init(&style_line);
  lv_style_set_line_width(&style_line, 2);

  static const lv_point_t selection_line_points[] = {{0, 0}, {SIZE_SYMBOLS, 0}};

  for (int i = 0; i < NUM_SYMBOLS; i++) {
    // Calculate absolute position for each symbol
    int pos_x = 1 + (SIZE_SYMBOLS + 1) * i;
    int pos_y = 3; // Default Y position (can be moved later independently)
    
    // Create and position the symbol using absolute coordinates
    modifier_symbols[i]->symbol = lv_img_create(widget->obj);
    lv_obj_set_pos(modifier_symbols[i]->symbol, pos_x, pos_y);
    lv_img_set_src(modifier_symbols[i]->symbol,
                   modifier_symbols[i]->symbol_dsc);

    // Create selection line
    // modifier_symbols[i]->selection_line = lv_line_create(widget->obj);
    // lv_line_set_points(modifier_symbols[i]->selection_line,
    //                    selection_line_points, 2);
    // lv_obj_add_style(modifier_symbols[i]->selection_line, &style_line, 0);
    
    // Position the selection line - use absolute positioning
    // Position it below its corresponding symbol
    // lv_obj_set_pos(modifier_symbols[i]->selection_line, 
    //               pos_x, // Same X as the symbol
    //               pos_y + SIZE_SYMBOLS + 3); // Below the symbol
    
    // // Hide the selection line initially
    // lv_obj_add_flag(modifier_symbols[i]->selection_line, LV_OBJ_FLAG_HIDDEN);
  }

  sys_slist_append(&widgets, &widget->node);

  widget_modifiers_init();

  return 0;
}

// int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget,
//                               lv_obj_t *parent) {
//   widget->obj = lv_obj_create(parent);

//   lv_obj_set_size(widget->obj, NUM_SYMBOLS * (SIZE_SYMBOLS + 1) + 1,
//                   SIZE_SYMBOLS + 3);

//   static lv_style_t style_line;
//   lv_style_init(&style_line);
//   lv_style_set_line_width(&style_line, 2);

//   static const lv_point_t selection_line_points[] = {{0, 0}, {SIZE_SYMBOLS, 0}};

//   for (int i = 0; i < NUM_SYMBOLS; i++) {
//     modifier_symbols[i]->symbol = lv_img_create(widget->obj);
//     lv_obj_align(modifier_symbols[i]->symbol, LV_ALIGN_LEFT_MID,
//                  1 + (SIZE_SYMBOLS + 1) * i, 1);
//     lv_img_set_src(modifier_symbols[i]->symbol,
//                    modifier_symbols[i]->symbol_dsc);

//     modifier_symbols[i]->selection_line = lv_line_create(widget->obj);
//     lv_line_set_points(modifier_symbols[i]->selection_line,
//                        selection_line_points, 2);
//     lv_obj_add_style(modifier_symbols[i]->selection_line, &style_line, 0);
//     lv_obj_align_to(modifier_symbols[i]->selection_line,
//                     modifier_symbols[i]->symbol, LV_ALIGN_OUT_BOTTOM_LEFT, 0,
//                     3);
//   }

//   sys_slist_append(&widgets, &widget->node);

//   widget_modifiers_init();

//   return 0;
// }

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget) {
  return widget->obj;
}
