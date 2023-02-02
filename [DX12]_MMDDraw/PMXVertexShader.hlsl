#include "BasicShaderHeader.hlsli"


Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, min16float4 exuv : EXUV, min16uint type : TYPE, min16uint4 boneno : BONENO, min16uint4 weight : WEIGHT, bool edgeflg : EDGE_FLG)
{
    Model output; //ピクセルシェーダへ渡す値
    float w1 = (float) weight.x / 100.0f;
    float w2 = (float) weight.y / 100.0f;
    float w3 = (float) weight.z / 100.0f;
    float w4 = (float) weight.w / 100.0f;
    switch (type)
    {
        case 0:
            break;
        case 1:
            break;
        case 2:
            break;
        case 3:
            break;
    }
    pos = mul(world, pos);
    output.pos = mul(view, pos);
    output.svpos = mul(mul(proj, view), pos); //シェーダでは列優先なので注意
    normal.w = 0; //ここ重要(平行移動成分を無効にする)
    output.normal = mul(world, normal); //法線にもワールド変換を行う
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.exuv = exuv[0];
    output.ray = normalize(pos.xyz - mul(view, eye)); //視線ベクトル

    return output;
}
