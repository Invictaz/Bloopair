/*
 *   Copyright (C) 2021 GaryOderNichts & Invictaz
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <controllers.h>
#include "utils.h"

typedef struct {
    uint8_t led_flags;
    uint8_t led_color[3];
    uint8_t rumble;
} DualshockData_t;

static const uint8_t led_flags[] = {
    0,
    0x01, // player 1
    0x03, // player 2
    0,
    0x0b, // player 3
    0,
    0,
    0,
    0x1b, // player 4
};

static const uint8_t led_colors[][3] = {
    {0},
    {0x00, 0x00, 0x40}, // player 1
    {0x40, 0x00, 0x00}, // player 2
    {0},
    {0x00, 0x40, 0x00}, // player 3
    {0},
    {0},
    {0},
    {0x20, 0x00, 0x20}, // player 4
};

static const uint32_t dpad_map[9] = {
    WPAD_PRO_BUTTON_UP,
    WPAD_PRO_BUTTON_UP | WPAD_PRO_BUTTON_RIGHT,
    WPAD_PRO_BUTTON_RIGHT,
    WPAD_PRO_BUTTON_RIGHT | WPAD_PRO_BUTTON_DOWN,
    WPAD_PRO_BUTTON_DOWN,
    WPAD_PRO_BUTTON_DOWN | WPAD_PRO_BUTTON_LEFT,
    WPAD_PRO_BUTTON_LEFT,
    WPAD_PRO_BUTTON_LEFT | WPAD_PRO_BUTTON_UP,
    0,
};

#define AXIS_NORMALIZE_VALUE (1140 * 2)

static void sendRumbleLedState(Controller_t* controller)
{
    DualshockData_t* ds_data = (DualshockData_t*) controller->additionalData;

    uint8_t data[79];
    _memset(data, 0, sizeof(data));
    data[0] = 0xa2;
    data[1] = 0x31;
    data[2] = 0x02;
    data[3] = 0x03;
    data[4] = 0x14;
    data[5] = ds_data->rumble;
    data[6] = ds_data->rumble;
    data[41] = 0x02;
    data[44] = 0x02;
    data[46] = ds_data->led_flags;
    data[47] = ds_data->led_color[0];
    data[48] = ds_data->led_color[1];
    data[49] = ds_data->led_color[2];

    uint32_t crc = bswap32(~crc32(0xffffffff, data, sizeof(data) - 4));
    _memcpy(&data[75], &crc, 4);

    sendOutputData(controller->handle, data + 1, sizeof(data) - 1);
}

void controllerRumble_dualshock(Controller_t* controller, uint8_t rumble)
{
    DualshockData_t* ds_data = (DualshockData_t*) controller->additionalData;

    ds_data->rumble = rumble ? 6 : 0;

    sendRumbleLedState(controller);
}

void controllerSetLed_dualshock(Controller_t* controller, uint8_t led)
{
    DualshockData_t* ds_data = (DualshockData_t*) controller->additionalData;

    ds_data->led_flags = led_flags[led];
    
    _memcpy(ds_data->led_color, &led_colors[led], 3);

    sendRumbleLedState(controller);
}

void controllerData_dualshock(Controller_t* controller, uint8_t* buf, uint16_t len)
{
    if (buf[0] == 0x01) {
        int16_t left_stick_x = (buf[1] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;
        int16_t left_stick_y = (buf[2] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;
        int16_t right_stick_x = (buf[3] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;
        int16_t right_stick_y = (buf[4] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;

        uint32_t buttons = 0;

        buttons |= dpad_map[buf[5] & 0xf];

        if (buf[5] & 0x40)
            buttons |= WPAD_PRO_BUTTON_A;
        if (buf[5] & 0x20)
            buttons |= WPAD_PRO_BUTTON_B;
        if (buf[5] & 0x80)
            buttons |= WPAD_PRO_BUTTON_X;
        if (buf[5] & 0x10)
            buttons |= WPAD_PRO_BUTTON_Y;
        if (buf[6] & 0x01)
            buttons |= WPAD_PRO_TRIGGER_L;
        if (buf[6] & 0x02)
            buttons |= WPAD_PRO_TRIGGER_R;
        if (buf[6] & 0x04)
            buttons |= WPAD_PRO_TRIGGER_ZL;
        if (buf[6] & 0x08)
            buttons |= WPAD_PRO_TRIGGER_ZR;
        if (buf[6] & 0x10)
            buttons |= WPAD_PRO_BUTTON_MINUS;
        if (buf[6] & 0x20)
            buttons |= WPAD_PRO_BUTTON_PLUS;
        if (buf[6] & 0x40)
            buttons |= WPAD_PRO_BUTTON_STICK_L;
        if (buf[6] & 0x80)
            buttons |= WPAD_PRO_BUTTON_STICK_R;
        if (buf[7] & 0x01)
            buttons |= WPAD_PRO_BUTTON_HOME;

        sendControllerInput(controller, buttons, left_stick_x, right_stick_x, left_stick_y, right_stick_y);
    }
    else if (buf[0] == 0x31) {
        int16_t left_stick_x = (buf[2] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;
        int16_t left_stick_y = (buf[3] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;
        int16_t right_stick_x = (buf[4] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;
        int16_t right_stick_y = (buf[5] - 256 / 2) * AXIS_NORMALIZE_VALUE / 256;

        uint32_t buttons = 0;

        buttons |= dpad_map[buf[9] & 0xf];

        if (buf[9] & 0x40)
            buttons |= WPAD_PRO_BUTTON_A;
        if (buf[9] & 0x20)
            buttons |= WPAD_PRO_BUTTON_B;
        if (buf[9] & 0x80)
            buttons |= WPAD_PRO_BUTTON_X;
        if (buf[9] & 0x10)
            buttons |= WPAD_PRO_BUTTON_Y;
        if (buf[10] & 0x01)
            buttons |= WPAD_PRO_TRIGGER_L;
        if (buf[10] & 0x02)
            buttons |= WPAD_PRO_TRIGGER_R;
        if (buf[10] & 0x04)
            buttons |= WPAD_PRO_TRIGGER_ZL;
        if (buf[10] & 0x08)
            buttons |= WPAD_PRO_TRIGGER_ZR;
        if (buf[10] & 0x10)
            buttons |= WPAD_PRO_BUTTON_MINUS;
        if (buf[10] & 0x20)
            buttons |= WPAD_PRO_BUTTON_PLUS;
        if (buf[10] & 0x40)
            buttons |= WPAD_PRO_BUTTON_STICK_L;
        if (buf[10] & 0x80)
            buttons |= WPAD_PRO_BUTTON_STICK_R;
        if (buf[11] & 0x01)
            buttons |= WPAD_PRO_BUTTON_HOME;

        uint8_t battery_level = (buf[53] & 0xf) >> 1;
        controller->battery = battery_level > 4 ? 4 : battery_level;
        controller->isCharging = (buf[53] & 0x10) && !(buf[53] & 0x20);

        sendControllerInput(controller, buttons, left_stick_x, right_stick_x, left_stick_y, right_stick_y);
    }
}

void controllerDeinit_dualshock(Controller_t* controller)
{
    IOS_Free(0xcaff, controller->additionalData);
}

void controllerInit_dualshock(Controller_t* controller)
{
    controller->data = controllerData_dualshock;
    controller->setPlayerLed = controllerSetLed_dualshock;
    controller->rumble = controllerRumble_dualshock;
    controller->deinit = controllerDeinit_dualshock;

    controller->battery = 4;
    controller->isCharging = 0;

    controller->additionalData = IOS_Alloc(0xcaff, sizeof(DualshockData_t));
    _memset(controller->additionalData, 0, sizeof(DualshockData_t));
}
