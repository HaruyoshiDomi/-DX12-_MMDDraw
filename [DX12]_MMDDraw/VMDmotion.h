#pragma once
class VMDmotion
{
public:
	VMDmotion(std::string filename);
	~VMDmotion(){}

	void UpdateMotion();

	void SetMotionFlag(bool flag);
	bool GetMotionFlag() { return m_motionFlag; }
	uint32_t GetFrameNo() { return m_frameNo; }
	void SetNowPose();
	void ResetMotion();
	struct VMDIKEnable
	{
		uint32_t frameNo = 0;
		std::unordered_map<std::string, bool> ikEnableTable;
	};

private:
	friend class Model;
	void LoadVMD();
	/// ///////////////////////////////////////////////////////////////////////////////
	struct Motion
	{
		unsigned int frameNo;	
		XMVECTOR quaternion;	
		XMFLOAT3 offset;		//IK�̏������W����̃I�t�Z�b�g���
		XMFLOAT2 p1, p2;		//�x�W�F�Ȑ��̒��ԃR���g���[���|�C���g
		Motion(unsigned int fno, XMVECTOR& q, XMFLOAT3& ofst, const XMFLOAT2& ip1, const XMFLOAT2& ip2)
			: frameNo(fno), quaternion(q), offset(ofst), p1(ip1), p2(ip2){}
	};
	//IK�I���I�t�f�[�^
	std::vector<VMDIKEnable> m_ikEnableData;
	/// ///////////////////////////////////////////////////////////////////////////////

#pragma pack(1)
	//�\��f�[�^(���_���[�t�f�[�^)
	struct VMDMorph
	{
		char name[15];
		uint32_t framNo;
		float weight;	//0.0f�`1.0f
	};
#pragma pack()
	vector<VMDMorph> m_morphs;
	/// ///////////////////////////////////////////////////////////////////////////////

	std::unordered_map<std::string, std::vector<Motion>> m_motionData;
	std::string m_filePath{};
	DWORD m_starTime;
	unsigned int m_duration = 0;
	class Model* m_model;
	uint32_t m_frameNo = 0;
	uint32_t m_oldframeNo = 0;
	bool m_motionFlag = false;
	unsigned long m_elapsedtime = 0;

	float GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n);
	void SetTimer();
public:
	std::vector<VMDIKEnable>* GetIKEnableData() { return &m_ikEnableData; }
	std::unordered_map<std::string, std::vector<Motion>>* GetMotionData() { return &m_motionData; }
	void GetMorphNameFromFrame(std::vector<std::string>& name, std::map<std::string, float>& weight);
};

