/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/services/bas.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/hid.h>
#include <dt-bindings/zmk/modifiers.h>

#include "modifiers_info.h"

struct modifiers_state {
    uint8_t modifiers;
};

struct modifier_symbol {
    uint8_t modifier;
    const lv_img_dsc_t *symbol_dsc_normal;
    const lv_img_dsc_t *symbol_dsc_inverted;
    lv_obj_t *symbol;
    bool is_active;
};

// Declare both normal and inverted icons
LV_IMG_DECLARE(control_icon);
LV_IMG_DECLARE(control_icon_inverted);
struct modifier_symbol ms_control = {
    .modifier = MOD_LCTL | MOD_RCTL,
    .symbol_dsc_normal = &control_icon,
    .symbol_dsc_inverted = &control_icon_inverted,
};

LV_IMG_DECLARE(shift_icon);
LV_IMG_DECLARE(shift_icon_inverted);
struct modifier_symbol ms_shift = {
    .modifier = MOD_LSFT | MOD_RSFT,
    .symbol_dsc_normal = &shift_icon,
    .symbol_dsc_inverted = &shift_icon_inverted,
};

LV_IMG_DECLARE(opt_icon);
LV_IMG_DECLARE(opt_icon_inverted);
struct modifier_symbol ms_opt = {
    .modifier = MOD_LALT | MOD_RALT,
    .symbol_dsc_normal = &opt_icon,
    .symbol_dsc_inverted = &opt_icon_inverted,
};

LV_IMG_DECLARE(cmd_icon);
LV_IMG_DECLARE(cmd_icon_inverted);
struct modifier_symbol ms_cmd = {
    .modifier = MOD_LGUI | MOD_RGUI,
    .symbol_dsc_normal = &cmd_icon,
    .symbol_dsc_inverted = &cmd_icon_inverted,
};

struct modifier_symbol *modifier_symbols[] = {
    // this order determines the order of the symbols
    &ms_shift, &ms_opt, &ms_cmd, &ms_control};

#define NUM_SYMBOLS (sizeof(modifier_symbols) / sizeof(struct modifier_symbol *))

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

static void set_modifiers(lv_obj_t *widget, struct modifiers_state state) {
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        bool mod_is_active = state.modifiers & modifier_symbols[i]->modifier;

        if (mod_is_active != modifier_symbols[i]->is_active) {
            // Switch between normal and inverted icons based on active state
            const lv_img_dsc_t *icon_to_use = mod_is_active ? 
                modifier_symbols[i]->symbol_dsc_inverted : 
                modifier_symbols[i]->symbol_dsc_normal;
            
            lv_img_set_src(modifier_symbols[i]->symbol, icon_to_use);
            modifier_symbols[i]->is_active = mod_is_active;
        }
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

ZMK_DISPLAY_WIDGET_LISTENER(widget_modifiers, struct modifiers_state, modifiers_update_cb,
                            modifiers_get_state)

ZMK_SUBSCRIPTION(widget_modifiers, zmk_keycode_state_changed);

int zmk_widget_modifiers_init(struct zmk_widget_modifiers *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);

    lv_obj_set_size(widget->obj, SIZE_SYMBOLS + 3, DISP_WIDTH);
    lv_obj_align(widget->obj, LV_ALIGN_TOP_LEFT, MODS_OFFSET, 0);

    for (int i = 0; i < NUM_SYMBOLS; i++) {
        modifier_symbols[i]->symbol = lv_img_create(widget->obj);
        lv_obj_align(modifier_symbols[i]->symbol, LV_ALIGN_TOP_LEFT, 0, 2 + (SIZE_SYMBOLS + 2) * i);
        // Initialize with normal (inactive) icon
        lv_img_set_src(modifier_symbols[i]->symbol, modifier_symbols[i]->symbol_dsc_normal);
        // Initialize as inactive
        modifier_symbols[i]->is_active = false;
    }

    sys_slist_append(&widgets, &widget->node);

    widget_modifiers_init();

    return 0;
}

lv_obj_t *zmk_widget_modifiers_obj(struct zmk_widget_modifiers *widget) { 
    return widget->obj; 
}
