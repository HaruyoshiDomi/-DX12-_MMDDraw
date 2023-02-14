#include "BasicShaderHeader.hlsli"


Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, float4 exuv : EXUVI,
            uint type : TYPE, int4 boneno : BONENO, float4 weight : WEIGHT,
            float4 c : C, float4 r0 : RZ, float4 r1 : RO
            )
{
    Model output; //ピクセルシェーダへ渡す値
    float w1 = weight.x ;
    float w2 = weight.y ;
    float w3 = weight.z ;
    float w4 = weight.w ;
    matrix bm;
    switch (type)
    {
        case 0:
            bm = bones[boneno.x] * w1;
            break;
        case 1:
            bm = bones[boneno.x] * w1 + bones[boneno.y] * w2;
            break;
        case 2:
            bm = bones[boneno.x] * w1 + bones[boneno.y] * w2 + bones[boneno.z] * w3 + bones[boneno.w] * w4;
            break;
        case 3:
            bm = bones[boneno.x] * w1 + bones[boneno.y] * w2;
            break;
    }

    output.pos = mul(world, mul(bm, pos));
    output.svpos = mul(proj, mul(view, output.pos)); //シェーダでは列優先なので注意
    normal.w = 0; //ここ重要(平行移動成分を無効にする)
    output.normal = mul(world, mul(bm, normal)); //法線にもワールド変換を行う
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.exuv = exuv[0];
    output.ray = normalize(output.svpos.xyz - mul(view, eye)); //視線ベクトル

    return output;
}

matrix SDEF(float w1, float w2, min16uint2 b)
{
    return matrix(bones[b.x] * w1 + bones[b.y] * w2);
}