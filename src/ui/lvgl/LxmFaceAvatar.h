#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include <cstddef>
#include <cstdint>

namespace LxmFaceAvatar {

struct View {
    lv_obj_t *box = nullptr;
    lv_obj_t *canvas = nullptr;
};

size_t bufferSize(lv_coord_t size);
bool   render(lv_obj_t *canvas, const String &seed);

} // namespace LxmFaceAvatar
