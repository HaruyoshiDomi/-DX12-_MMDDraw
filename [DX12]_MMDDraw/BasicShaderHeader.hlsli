//頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体

//頂点シェーダ→ピクセルシェーダへのやり取りに使用する
//構造体
struct Model
{
    float4 svpos : SV_POSITION; //システム用頂点座標
    float4 pos : POSITION; //システム用頂点座標
    float4 normal : NORMAL0; //法線ベクトル
    float4 vnormal : NORMAL1; //法線ベクトル
    float2 uv : TEXCOORD; //UV値
    float4 exuv : TEXCOORD1; //UV値
    float3 ray : VECTOR; //ベクトル
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


//定数バッファ0
cbuffer SceneData : register(b0)
{
    matrix view;
    matrix proj; //ビュープロジェクション行列
    matrix shadow;
    float3 eye;
};
cbuffer Transform : register(b1)
{
    matrix world; //ワールド変換行列
    matrix bones[512];
}

//定数バッファ1
//マテリアル用
cbuffer Material : register(b2)
{
    float4 diffuse; //ディフューズ色
    float4 specular; //スペキュラ
    float3 ambient; //アンビエント
    
    bool edgeFlg; //マテリアル毎の輪郭線フラグ
    float4 edgecolor;
    float edgesize;
};
