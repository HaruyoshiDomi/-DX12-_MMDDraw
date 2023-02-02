//���_�V�F�[�_�[����s�N�Z���V�F�[�_�[�ւ̂����Ɏg�p����\����

//���_�V�F�[�_���s�N�Z���V�F�[�_�ւ̂����Ɏg�p����
//�\����
struct Model
{
    float4 svpos : SV_POSITION; //�V�X�e���p���_���W
    float4 pos : POSITION; //�V�X�e���p���_���W
    float4 normal : NORMAL0; //�@���x�N�g��
    float4 vnormal : NORMAL1; //�@���x�N�g��
    float2 uv : TEXCOORD; //UV�l
    float4 exuv : TEXCOORD1; //UV�l
    float3 ray : VECTOR; //�x�N�g��
    bool edgeflg : EDGE_FLG;
    uint instNo : SV_InstanceID;
};

struct Polygon
{
    float4 svpos : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D<float4> tex : register(t0);
Texture2D<float4> sph : register(t1);
Texture2D<float4> spa : register(t2);
Texture2D<float4> toon : register(t3);
Texture2D<float4> subtex : register(t4);

SamplerState smp : register(s0);
SamplerState smptoon : register(s1);


//�萔�o�b�t�@0
cbuffer SceneData : register(b0)
{
    matrix view;
    matrix proj; //�r���[�v���W�F�N�V�����s��
    matrix shadow;
    float3 eye;
};
cbuffer Transform : register(b1)
{
    matrix world; //���[���h�ϊ��s��
    matrix bones[512];
}

//�萔�o�b�t�@1
//�}�e���A���p
cbuffer Material : register(b2)
{
    float4 diffuse; //�f�B�t���[�Y�F
    float4 specular; //�X�y�L����
    float3 ambient; //�A���r�G���g
    
    bool edgeFlg; //�}�e���A�����̗֊s���t���O
    float4 edgecolor;
    float edgesize;
};
