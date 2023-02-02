/* ------------------------------------------------------------
 * AlternativeFull
 * ------------------------------------------------------------ */
/* created by AlternativeFullFrontend. */
#define TEXTURE_THRESHOLD "shadingSkin.png"
#define TEXTURE_SHADOW "tAcc_sdw.png"
#define USE_NORMALMAP
#define TEXTURE_NORMALMAP "tAcc_nml.png"
float NormalMapResolution = 1;
#define USE_SELFSHADOW_MODE
#define USE_NONE_SELFSHADOW_MODE
float SelfShadowPower = 1;
#define HIGHLIGHT_ANTI_AUTOLUMINOUS
#define USE_MATERIAL_SPHERE
float3 DefaultModeShadowColor = {1,1,1};
#define MAX_ANISOTROPY 16
#define USE_EDGE_TWEAK
float EdgeTickness = 0.0025;
float EdgeScale = 3;
float EdgePower = 2.0;
float EdgeDarkness = 0.6;

#include "AlternativeFull.fxsub"
