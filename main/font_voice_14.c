/* Shared Noto Sans SC 14px glyphs; see assets/fonts/OFL.txt. */
#include "lvgl.h"
extern const lv_font_fmt_txt_dsc_t font_ui_14_descriptor;
const lv_font_t font_voice_14={.get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,.get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,.line_height=18,.base_line=4,.dsc=&font_ui_14_descriptor};
