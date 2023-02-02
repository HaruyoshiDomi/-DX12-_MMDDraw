#include "BasicShaderHeader.hlsli"


Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, min16float4 exuv : EXUV, min16uint type : TYPE, min16uint4 boneno : BONENO, min16uint4 weight : WEIGHT, bool edgeflg : EDGE_FLG)
{
    Model output; //�s�N�Z���V�F�[�_�֓n���l
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
    output.svpos = mul(mul(proj, view), pos); //�V�F�[�_�ł͗�D��Ȃ̂Œ���
    normal.w = 0; //�����d�v(���s�ړ������𖳌��ɂ���)
    output.normal = mul(world, normal); //�@���ɂ����[���h�ϊ����s��
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.exuv = exuv[0];
    output.ray = normalize(pos.xyz - mul(view, eye)); //�����x�N�g��

    return output;
}
