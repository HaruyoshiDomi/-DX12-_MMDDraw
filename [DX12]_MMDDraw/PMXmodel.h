#pragma once
#include "Model.h"
class PMXmodel : public Model
{
public:
	PMXmodel(const char* filepath, Render& dx12);
	~PMXmodel();

	void Update(XMMATRIX& martrix)override;
	void Draw()override;
protected:

	//ルートシグネチャ初期化
	HRESULT CreateRootSignature()override;
	//パイプライン初期化
	HRESULT CreateGraphicsPipeline()override;

	//読み込んだマテリアルをもとにマテリアルバッファを作成
	HRESULT CreateMaterialData();

	//マテリアル＆テクスチャのビューを作成
	HRESULT CreateMaterialAndTextureView();

	//座標変換用ビューの生成
	HRESULT CreateTransformView();


	HRESULT LoadPMXFile(const char* filepath);

private:

	struct PMXVertice
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
		XMFLOAT4 additionaluv[4];
		uint8_t type;
		int born[4] = { -1, -1, -1, -1 };
		XMFLOAT4 weight = { -1, -1, -1, -1 };
		XMFLOAT3 c;
		XMFLOAT3 r0;
		XMFLOAT3 r1;

		float edgenif;
	};
	std::vector<PMXVertice> m_vertice;

	struct OriginalVertices
	{
		XMFLOAT3 pos;
		XMFLOAT2 uv;
	};
	std::vector<OriginalVertices> m_originalVertices;
	//シェーダ側に投げられるマテリアルデータ
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT4 diffuse; //ディフューズ色
		DirectX::XMFLOAT3 specular; //スペキュラ色
		float specularity;//スペキュラの強さ(乗算値)
		DirectX::XMFLOAT3 ambient; //アンビエント色

		bool edgeFlg;//マテリアル毎の輪郭線フラグ
		XMFLOAT4 edgecolor;
		float edgesize;

	};

	//それ以外のマテリアルデータ
	struct AdditionalMaterial
	{
		int texIdx;//テクスチャファイルパス
		int spharIdx;//テクスチャファイルパス
		int toonIdx; //トゥーン番号
		std::string defaltetoontex;
		BYTE mode;
		bool toonflag;
		std::string comment;

	};
	
	//まとめたもの
	struct Material
	{
		std::wstring name;
		std::wstring name_eng;
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
	std::vector<ComPtr<ID3D12Resource>> m_subtextureResources;


	struct PMXIK
	{
		uint16_t boneIdx;//IK対象のボーンを示す
		uint16_t targetIdx;//ターゲットに近づけるためのボーンのインデックス
		uint16_t iterations;//試行回数
		float limit;//一回当たりの回転制限
		std::vector<uint16_t> nodeIdxes;//間のノード番号
	};
	std::vector<PMXIK> m_ikdata;


	struct MorphsData
	{
		uint32_t vertexIndex;
		XMFLOAT3 posval;
		XMFLOAT4 uv;
	};
	
	std::map<std::string, std::vector<MorphsData>> m_morphsData;
	ComPtr< ID3D12DescriptorHeap> m_materialHeap = nullptr;

	float m_angle = 0.0f;//テスト用Y軸回転


	bool getPMXStringUTF16(FILE* helper, std::string& outpath);
	bool getPMXStringUTF16(FILE* helper, std::wstring& outpath);

	void UpdateModelMorf();

	std::wstring ChangeCNToJP( std::wstring path);
};

