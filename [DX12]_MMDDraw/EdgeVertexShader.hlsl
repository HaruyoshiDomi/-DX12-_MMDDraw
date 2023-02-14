#include "BasicShaderHeader.hlsli"


Model main(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, min16uint2 boneno : BONENO, min16uint weight : WEIGHT, bool edgeflg : EDGE_FLG, uint instNo : SV_InstanceID)
{
    Model output; //�s�N�Z���V�F�[�_�֓n���l
    float w = (float)weight / 100.0f;
    matrix bm = bones[boneno.x] * w + bones[boneno.y] * (1 - w);
    pos = mul(bm, pos);
    pos = mul(world, pos);
    output.svpos = mul(mul(proj, view), pos);            //�V�F�[�_�ł͗�D��Ȃ̂Œ���
    output.pos = mul(view, pos);
    normal.w = 0;                                        //���s�ړ������𖳌��ɂ���
    output.normal = mul(world, mul(bm, normal)); //�@���ɂ����[���h�ϊ����s��
    output.vnormal = mul(view, output.normal);
    output.uv = uv;
    output.ray = normalize(output.svpos.xyz - mul(view, eye)); //�����x�N�g��
    output.edgeflg = edgeflg;
    output.instNo = instNo;
    if (edgeFlg)
    {
        output.svpos += output.vnormal * 0.05;
    }
    return output;
    
}
