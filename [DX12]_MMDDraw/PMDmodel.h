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
	//�V�F�[�_���ɓ�������}�e���A���f�[�^
	struct MaterialForHlsl
	{
		DirectX::XMFLOAT3 diffuse; //�f�B�t���[�Y�F
		float alpha; // �f�B�t���[�Y��
		DirectX::XMFLOAT3 specular; //�X�y�L�����F
		float specularity;//�X�y�L�����̋���(��Z�l)
		DirectX::XMFLOAT3 ambient; //�A���r�G���g�F

		bool edgeFlag;
	};
	//����ȊO�̃}�e���A���f�[�^
	struct AdditionalMaterial
	{
		std::string texPath;//�e�N�X�`���t�@�C���p�X
		int toonIdx; //�g�D�[���ԍ�
	};
	//�܂Ƃ߂�����
	struct Material
	{
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

	ComPtr< ID3D12DescriptorHeap> m_materialHeap = nullptr;


	struct PMDIK
	{
		uint16_t boneIdx;//IK�Ώۂ̃{�[��������
		uint16_t targetIdx;//�^�[�Q�b�g�ɋ߂Â��邽�߂̃{�[���̃C���f�b�N�X
		uint16_t iterations;//���s��
		float limit;//��񓖂���̉�]����
		std::vector<uint16_t> nodeIdxes;//�Ԃ̃m�[�h�ԍ�

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
		BONE_FOLLOW,	//�Ǐ]
		PHYSICS,		//�������Z
		PHYSICS_BONE	//�������Z�i�{�[���ʒu���킹�j
	};

	struct RigitBodyData
	{
		char name[20];
		UINT16 relboneIdx;
		BYTE groupIdx;
		UINT16  grouptarget;
		ShapeTYPE shape_type;
		float w;		//���a(��)
		float h;		//����
		float d;		//���s
		XMFLOAT3 pos;
		XMFLOAT3 rot;	//���W�A���p
		float weight;	//����
		float posdim;	//�ړ���
		float rotdim;	//��]��
		float recoil;	//������
		float friction;	//���C��
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
		XMFLOAT3 constrain_pos1;	//����
		XMFLOAT3 constrain_pos2;	//����
		XMFLOAT3 constrain_rot1;	//����
		XMFLOAT3 constrain_rot2;	//����
		XMFLOAT3 spring_pos;		//�΂�
		XMFLOAT3 spring_rot;		//�΂�

	};

	std::vector<Joint> m_joint;
	//========================================================================
	// 
	//�ǂݍ��񂾃}�e���A�������ƂɃ}�e���A���o�b�t�@���쐬
	HRESULT CreateMaterialData();

	//�}�e���A�����e�N�X�`���̃r���[���쐬
	HRESULT CreateMaterialAndTextureView();

	//���W�ϊ��p�r���[�̐���
	HRESULT CreateTransformView();

	//PMD�t�@�C���̃��[�h
	HRESULT LoadPMDFile(const char* path);


	//CCD-IK�ɂ��{�[������������
	void SolveCCDIK(const PMDIK& ik);

	///�]���藝IK�ɂ��{�[������������
	void SolveCosineIK(const PMDIK& ik);

	///LookAt�s��ɂ��{�[������������
	void SolveLookAt(const PMDIK& ik);

	void IKSolve(int frameNo = 0);

	float m_angle = 0.0f; //�e�X�g�pY����]

public:
	std::vector<RigitBodyData>* GetRigitbody() { return &m_rigiddata; }
	std::vector<Joint>* GetJoint() { return &m_joint; }

};