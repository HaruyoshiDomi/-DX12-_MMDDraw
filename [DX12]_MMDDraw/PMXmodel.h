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

	struct Vertices
	{
		XMFLOAT3 pos;
		XMFLOAT3 normal;
		XMFLOAT2 uv;
	};

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

	//�{�[���֘A
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
		uint16_t boneIdx;//IK�Ώۂ̃{�[��������
		uint16_t targetIdx;//�^�[�Q�b�g�ɋ߂Â��邽�߂̃{�[���̃C���f�b�N�X
		uint16_t iterations;//���s��
		float limit;//��񓖂���̉�]����
		std::vector<uint16_t> nodeIdxes;//�Ԃ̃m�[�h�ԍ�

	};
	std::vector<PMDIK> m_ikdata;

	//XMMATRIX* m_mappedmartrices = nullptr;

	Transform m_transform;
	Transform* m_mappedTransform = nullptr;
	ComPtr<ID3D12Resource> m_transformBuff = nullptr;


	ComPtr< ID3D12DescriptorHeap> m_materialHeap = nullptr;

	float m_angle = 0.0f;//�e�X�g�pY����]


	bool getPMXStringUTF16(FILE* helper, std::string& outpath);
	bool getPMXStringUTF16(FILE* helper, std::wstring& outpath);

	std::wstring ChangeCNToJP( std::wstring path);
};

