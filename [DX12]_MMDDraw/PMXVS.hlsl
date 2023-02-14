#include "BasicShaderHeader.hlsli"


Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, float4 exuv : EXUVI,
            uint type : TYPE, int4 boneno : BONENO, float4 weight : WEIGHT,
            float4 c : C, float4 r0 : RZ, float4 r1 : RO
            )
{
    Model output; //�s�N�Z���V�F�[�_�֓n���l
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
    output.svpos = mul(proj, mul(view, output.pos)); //�V�F�[�_�ł͗�D��Ȃ̂Œ���
    normal.w = 0; //�����d�v(���s�ړ������𖳌��ɂ���)
    output.normal = mul(world, mul(bm, normal)); //�@���ɂ����[���h�ϊ����s��
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.exuv = exuv[0];
    output.ray = normalize(output.svpos.xyz - mul(view, eye)); //�����x�N�g��

    return output;
}

matrix SDEF(float w1, float w2, min16uint2 b)
{
    return matrix(bones[b.x] * w1 + bones[b.y] * w2);
}