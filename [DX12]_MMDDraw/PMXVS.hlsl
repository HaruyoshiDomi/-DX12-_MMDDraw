#include "BasicShaderHeader.hlsli"


Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, float4 exuv : EXUVI,
            uint type : TYPE, int4 boneno : BONENO, float4 weight : WEIGHT,
            float4 c : C, float4 r0 : RZ, float4 r1 : RO, uint instNo : SV_InstanceID
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
            bm = mq[boneno.x].bones * w1;
            break;
        case 1:
            bm = mq[boneno.x].bones * w1 + mq[boneno.y].bones * w2;
            break;
        case 2:
            bm = mq[boneno.x].bones * w1 + mq[boneno.y].bones * w2 + mq[boneno.z].bones * w3 + mq[boneno.w].bones * w4;
            break;
        case 3:
            bm = mq[boneno.x].bones * w1 + mq[boneno.y].bones * w2;
            break;
    }
  

    output.pos = mul(world, mul(bm, pos));
    if (instNo > 0)
        output.pos = mul(shadow, output.pos);
    output.svpos = mul(proj, mul(view, output.pos)); //シェーダでは列優先なので注意
    normal.w = 0; //ここ重要(平行移動成分を無効にする)
    output.normal = mul(world, mul(bm, normal)); //法線にもワールド変換を行う
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.exuv = exuv[0];
    output.ray = normalize(output.svpos.xyz - mul(view, eye)); //視線ベクトル
    output.instNo = instNo;

    return output;
}

void CalcSdefWeight(out float weight0, out float weight1, in float3 defR0, in float3 defR1)
{
    float leng0 = length(defR0);
    float leng1 = length(defR1);
    if (abs(leng0 - leng1) < 0.00001f)
    {
        weight1 = 0.5f;
    }
    else
    {
        weight1 = saturate(leng0 / (leng0 + leng1));
    }
    weight0 = 1.0f - weight1;
}

matrix SDEF(float w0, float w1, int2 b, float3 R0, float3 R1, float3 c, matrix m0, matrix m1)
{
    float w2, w3;
    CalcSdefWeight(w2, w3, R0, R1);
    //C点算出
    float4 r0 = float4(R0 + c, 1);
    float4 r1 = float4(R1 + c, 1);
    matrix mrc = m0 * w0 + m1 * w1;
    float3 prc = mul(float4(c, 1), mrc).xyz;
    
    //r0,r1による差分を算出して加算
    {
        matrix m2 = m0 * w0;
        matrix m3 = m1 * w1;
        matrix m = m2 + m3;
        
    }
}