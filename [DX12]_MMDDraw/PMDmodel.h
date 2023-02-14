#pragma once
#include "Model.h"

typedef enum
{
	SPHERE,
	BOX,
	CAPSULE
}ShapeTYPE;


class PMDmodel : public Model
{
public:
	PMDmodel(const char* filepath, class Render& dx12);
	~PMDmodel();
	void Update(XMMATRIX& martrix)override;
	void Draw()override;


protected:

	HRESULT CreateGraphicsPipeline()override;
	HRESULT CreateRootSignature()override;

private:

	struct PMDVertex
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		WORD boneNum[2];
		BYTE weight;
		BYTE edgeFlg;
	};
	std::vector<PMDVertex> m_vertices;
	//シェーダ側に投げられるマテリアルデータ
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT3 diffuse; //ディフューズ色
		float alpha; // ディフューズα
		DirectX::XMFLOAT3 specular; //スペキュラ色
		float specularity;//スペキュラの強さ(乗算値)
		DirectX::XMFLOAT3 ambient; //アンビエント色

		bool edgeFlag;
	};
	//それ以外のマテリアルデータ
	struct AdditionalMaterial
	{
		std::string texPath;//テクスチャファイルパス
		int toonIdx; //トゥーン番号
	};
	//まとめたもの
	struct Material
	{
		unsigned int indicesNum;//インデックス数
		MaterialForHlsl material;
		AdditionalMaterial additional;
	};

	//マテリアル関連
	std::vector<Material> m_materials;
	std::vector<ComPtr<ID3D12Resource>> m_textureResources;
	std::vector<ComPtr<ID3D12Resource>> m_sphResources;
	std::vector<ComPtr<ID3D12Resource>> m_spaResources;
	std::vector<ComPtr<ID3D12Resource>> m_toonResources;

	ComPtr< ID3D12DescriptorHeap> m_materialHeap = nullptr;


	struct PMDIK
	{
		uint16_t boneIdx;//IK対象のボーンを示す
		uint16_t targetIdx;//ターゲットに近づけるためのボーンのインデックス
		uint16_t iterations;//試行回数
		float limit;//一回当たりの回転制限
		std::vector<uint16_t> nodeIdxes;//間のノード番号

	};
	std::vector<PMDIK> m_ikdata;

#pragma pack(1)
	struct skindata
	{
		DWORD skinIdx;
		XMFLOAT3 pos;
	};
#pragma pack()

	std::map<std::string, std::vector<skindata>> m_morphsData;


	//Rigitbody========================================================================

	enum RigitTYPE
	{
		BONE_FOLLOW,	//追従
		PHYSICS,		//物理演算
		PHYSICS_BONE	//物理演算（ボーン位置合わせ）
	};

	struct RigitBodyData
	{
		char name[20];
		UINT16 relboneIdx;
		BYTE groupIdx;
		UINT16  grouptarget;
		ShapeTYPE shape_type;
		float w;		//半径(幅)
		float h;		//高さ
		float d;		//奥行
		XMFLOAT3 pos;
		XMFLOAT3 rot;	//ラジアン角
		float weight;	//質量
		float posdim;	//移動減
		float rotdim;	//回転減
		float recoil;	//反発力
		float friction;	//摩擦力
		RigitTYPE rigid_type;
	};

	std::vector<RigitBodyData> m_rigiddata;
	//========================================================================

	//Joint===================================================================
	struct Joint
	{
		char name[20];
		DWORD rigit_a;
		DWORD rigit_b;
		XMFLOAT3 pos;
		XMFLOAT3 rot;
		XMFLOAT3 constrain_pos1;	//制限
		XMFLOAT3 constrain_pos2;	//制限
		XMFLOAT3 constrain_rot1;	//制限
		XMFLOAT3 constrain_rot2;	//制限
		XMFLOAT3 spring_pos;		//ばね
		XMFLOAT3 spring_rot;		//ばね

	};

	std::vector<Joint> m_joint;
	//========================================================================
	// 
	//読み込んだマテリアルをもとにマテリアルバッファを作成
	HRESULT CreateMaterialData();

	//マテリアル＆テクスチャのビューを作成
	HRESULT CreateMaterialAndTextureView();

	//座標変換用ビューの生成
	HRESULT CreateTransformView();

	//PMDファイルのロード
	HRESULT LoadPMDFile(const char* path);


	//CCD-IKによりボーン方向を解決
	void SolveCCDIK(const PMDIK& ik);

	///余弦定理IKによりボーン方向を解決
	void SolveCosineIK(const PMDIK& ik);

	///LookAt行列によりボーン方向を解決
	void SolveLookAt(const PMDIK& ik);

	void IKSolve(int frameNo = 0);

	float m_angle = 0.0f; //テスト用Y軸回転

public:
	std::vector<RigitBodyData>* GetRigitbody() { return &m_rigiddata; }
	std::vector<Joint>* GetJoint() { return &m_joint; }

};