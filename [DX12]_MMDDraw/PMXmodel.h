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

	//���[�g�V�O�l�`��������
	HRESULT CreateRootSignature()override;
	//�p�C�v���C��������
	HRESULT CreateGraphicsPipeline()override;

	//�ǂݍ��񂾃}�e���A�������ƂɃ}�e���A���o�b�t�@���쐬
	HRESULT CreateMaterialData();

	//�}�e���A�����e�N�X�`���̃r���[���쐬
	HRESULT CreateMaterialAndTextureView();

	//���W�ϊ��p�r���[�̐���
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
	//�V�F�[�_���ɓ�������}�e���A���f�[�^
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT4 diffuse; //�f�B�t���[�Y�F
		DirectX::XMFLOAT3 specular; //�X�y�L�����F
		float specularity;//�X�y�L�����̋���(��Z�l)
		DirectX::XMFLOAT3 ambient; //�A���r�G���g�F

		bool edgeFlg;//�}�e���A�����̗֊s���t���O
		XMFLOAT4 edgecolor;
		float edgesize;

	};

	//����ȊO�̃}�e���A���f�[�^
	struct AdditionalMaterial
	{
		int texIdx;//�e�N�X�`���t�@�C���p�X
		int spharIdx;//�e�N�X�`���t�@�C���p�X
		int toonIdx; //�g�D�[���ԍ�
		std::string defaltetoontex;
		BYTE mode;
		bool toonflag;
		std::string comment;

	};
	
	//�܂Ƃ߂�����
	struct Material
	{
		std::wstring name;
		std::wstring name_eng;
		unsigned int indicesNum;//�C���f�b�N�X��
		MaterialForHlsl material;
		AdditionalMaterial additional;
	};

	//�}�e���A���֘A
	std::vector<Material> m_materials;
	std::vector<ComPtr<ID3D12Resource>> m_textureResources;
	std::vector<ComPtr<ID3D12Resource>> m_sphResources;
	std::vector<ComPtr<ID3D12Resource>> m_spaResources;
	std::vector<ComPtr<ID3D12Resource>> m_toonResources;
	std::vector<ComPtr<ID3D12Resource>> m_subtextureResources;


	struct PMXIK
	{
		uint16_t boneIdx;//IK�Ώۂ̃{�[��������
		uint16_t targetIdx;//�^�[�Q�b�g�ɋ߂Â��邽�߂̃{�[���̃C���f�b�N�X
		uint16_t iterations;//���s��
		float limit;//��񓖂���̉�]����
		std::vector<uint16_t> nodeIdxes;//�Ԃ̃m�[�h�ԍ�
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

	float m_angle = 0.0f;//�e�X�g�pY����]


	bool getPMXStringUTF16(FILE* helper, std::string& outpath);
	bool getPMXStringUTF16(FILE* helper, std::wstring& outpath);

	void UpdateModelMorf();

	std::wstring ChangeCNToJP( std::wstring path);
};

