// CLOUD -- outline cloud for the header weather glyph.
// Boundary of a union of lobes, traced rather than stroked: overlapping
// arcs cut lines through the interior, and a filled one reads as a blob.
#pragma once

#include <pgmspace.h>

static const int CLOUD_W = 16;
static const int CLOUD_H = 11;
static const uint8_t CLOUD_BITMAP[22] PROGMEM = {
    0x00, 0x70, 0x01, 0x8c, 0x0e, 0x04, 0x10, 0x02,
    0x20, 0x02, 0x20, 0x02, 0x40, 0x02, 0x80, 0x01,
    0x80, 0x01, 0x80, 0x01, 0x7f, 0xfe,
};
