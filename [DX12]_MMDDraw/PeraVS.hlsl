#include "BasicShaderHeader.hlsli"
Polygon main(float4 pos : POSITION, float2 uv : TEXCOORD)
{
    Polygon output;
    output.svpos = pos;
    output.uv = uv;
    return output;
}