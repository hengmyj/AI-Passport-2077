#pragma once
#include "lvgl.h"
#include "xiaozhi_face_variants.h"
typedef struct {lv_image_dsc_t image;int16_t x,y;} xz_face_asset_t;
enum {XZ_DECOR_CHEEK,XZ_DECOR_TEAR,XZ_DECOR_HEART,XZ_DECOR_SWEAT,XZ_DECOR_QUESTION,XZ_DECOR_ZZZ,XZ_DECOR_STAR,XZ_DECOR_HAND,XZ_DECOR_COUNT};
extern const xz_face_asset_t xz_face_body;
extern const xz_face_asset_t xz_face_eye_frames[XZ_EYE_COUNT];
extern const xz_face_asset_t xz_face_mouth_frames[XZ_FACE_COUNT+3];
extern const xz_face_asset_t xz_face_decor[XZ_DECOR_COUNT];
extern const xz_face_asset_t xz_abstract_body;
extern const xz_face_asset_t xz_abstract_detail;
extern const xz_face_asset_t xz_female_body;
extern const xz_face_asset_t xz_female_detail;
extern const xz_face_asset_t xz_female_accent;
extern const xz_face_asset_t xz_bald_gloss;
extern const xz_face_asset_t xz_portrait_tear;
extern const xz_face_asset_t xz_round_outline,xz_round_hair,xz_round_skin,xz_round_nose,xz_round_shirt;
extern const xz_face_asset_t xz_round_eye_left_frames[XZ_EYE_COUNT];
extern const xz_face_asset_t xz_round_eye_right_frames[XZ_EYE_COUNT];
extern const xz_face_asset_t xz_round_mouth_frames[XZ_FACE_COUNT+3];
extern const xz_face_asset_t xz_abstract_eye_frames[XZ_EYE_COUNT];
extern const xz_face_asset_t xz_abstract_mouth_frames[XZ_FACE_COUNT+3];
