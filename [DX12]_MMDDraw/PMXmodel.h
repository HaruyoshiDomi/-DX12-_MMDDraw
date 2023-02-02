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

	struct Vertices
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

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

	//ボーン関連
	std::vector<XMMATRIX> m_boneMatrieces;

	struct BoneNode
	{
		uint16_t boneidx;
		uint16_t childrenidx;
		uint16_t bonetype;
		uint16_t parentBone;
		uint16_t ikparentBone;
		XMFLOAT3 startPos;
		XMFLOAT3 endpos;
		std::vector<BoneNode*> children;
	};
	std::map<std::string, BoneNode> m_boneNodetable;

	struct PMDIK
	{
		uint16_t boneIdx;//IK対象のボーンを示す
		uint16_t targetIdx;//ターゲットに近づけるためのボーンのインデックス
		uint16_t iterations;//試行回数
		float limit;//一回当たりの回転制限
		std::vector<uint16_t> nodeIdxes;//間のノード番号

	};
	std::vector<PMDIK> m_ikdata;

	//XMMATRIX* m_mappedmartrices = nullptr;

	Transform m_transform;
	Transform* m_mappedTransform = nullptr;
	ComPtr<ID3D12Resource> m_transformBuff = nullptr;


	ComPtr< ID3D12DescriptorHeap> m_materialHeap = nullptr;

	float m_angle = 0.0f;//テスト用Y軸回転


	bool getPMXStringUTF16(FILE* helper, std::string& outpath);
	bool getPMXStringUTF16(FILE* helper, std::wstring& outpath);

	std::wstring ChangeCNToJP( std::wstring path);
};

