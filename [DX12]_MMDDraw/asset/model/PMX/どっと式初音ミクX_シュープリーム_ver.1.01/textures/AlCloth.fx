/* ------------------------------------------------------------
 * AlternativeFull
 * ------------------------------------------------------------ */
/* created by AlternativeFullFrontend. */
#define TEXTURE_THRESHOLD "shadingSkin.png"
#define TEXTURE_SHADOW "tBody_sdw.png"
#define USE_NORMALMAP
#define TEXTURE_NORMALMAP "tBody_nml.png"
float NormalMapResolution = 1;
#define USE_SELFSHADOW_MODE
#define USE_NONE_SELFSHADOW_MODE
float SelfShadowPower = 1;
#define HIGHLIGHT_ANTI_AUTOLUMINOUS
#define USE_MATERIAL_SPHERE
float3 DefaultModeShadowColor = {1,1,1};
#define MAX_ANISOTROPY 16
#define USE_EDGE_TWEAK
#define TEXTURE_EDGE_SCALE "tBody_oln.png"
float EdgeTickness = 0.0008;
float EdgeScale = 3;
float EdgePower = 2.0;
float EdgeDarkness = 0.4;

#include "AlternativeFull.fxsub"
