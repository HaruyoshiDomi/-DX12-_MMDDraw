#include "BasicShaderHeader.hlsli"

Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, min16uint2 boneno : BONENO, min16uint weight : WEIGHT, bool edgeflg : EDGE_FLG, uint instNo : SV_InstanceID)
{
    Model output; //ピクセルシェーダへ渡す値
    float w = (float)weight / 100.0f;
    matrix bm = mq[boneno.x].bones * w + mq[boneno.y].bones * (1 - w);
    output.pos = mul(world, mul(bm, pos));
    if (instNo > 0)
        output.pos = mul(shadow, output.pos);
    
    output.svpos = mul(proj, mul(view, output.pos)); //シェーダでは列優先なので注意
    normal.w = 0;                                        //平行移動成分を無効にする
    output.normal = mul(world, mul(bm, normal)); //法線にもワールド変換を行う
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.ray = normalize(output.svpos.xyz - mul(view, eye)); //視線ベクトル
    output.edgeflg = edgeflg;
    output.instNo = instNo;

    return output;
    
}
