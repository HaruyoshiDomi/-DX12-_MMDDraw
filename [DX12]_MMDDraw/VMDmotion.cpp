#include "main.h"
#include "PMDmodel.h"
#include "VMDmotion.h"

VMDmotion::VMDmotion(std::string filename) : m_filePath(filename), m_model(nullptr)
{
	LoadVMD();
}

void VMDmotion::UpdateMotion()
{
	if (!m_model)
		return;
	if (m_motionFlag)
	{
		m_elapsedtime = timeGetTime() - m_starTime;
		m_frameNo = (30) * (m_elapsedtime / 1000.0f);
		m_frameNo += m_oldframeNo;
		if (m_frameNo > m_duration)
		{
			m_starTime = timeGetTime();
			m_frameNo = m_oldframeNo = 0;
		}
		std::cout << m_frameNo << "：" << m_duration << std::endl;
		auto bonemat = m_model->GetBoneMatrixAndQuatanion();
		//行列情報クリア
		std::fill(bonemat->begin(), bonemat->end(), m_model->MatAndQuatIdentity());

		for (auto& bonemotion : m_motionData)
		{
			auto& nodename = bonemotion.first;
			auto nodetb = m_model->GetBoneNodeTable();
			auto itBoneNode = nodetb->find(nodename);
			if (itBoneNode == nodetb->end())
				continue;
			auto node = itBoneNode->second;
			auto motions = bonemotion.second;
			auto f = m_frameNo;
			auto rit = std::find_if(motions.rbegin(), motions.rend(), [f](const Motion& motion)
				{
					return motion.frameNo <= f;
				});
			if (rit == motions.rend())
				continue;

			XMMATRIX rotation = XMMatrixIdentity();
			XMVECTOR offset = XMLoadFloat3(&rit->offset);
			auto it = rit.base();
			if (it != motions.end())
			{
				auto t = (float)(m_frameNo - rit->frameNo) / (float)(it->frameNo - rit->frameNo);
				t = GetYFromXOnBezier(t, it->p1, it->p2, 12);
				auto quatvec = XMQuaternionSlerp(rit->quaternion, it->quaternion, t);
				rotation = XMMatrixRotationQuaternion(quatvec);
				XMStoreFloat4(&(*bonemat)[node.boneidx].boneQuatanions, quatvec);
				offset = XMVectorLerp(offset, XMLoadFloat3(&it->offset), t);
			}
			else
			{
				rotation = XMMatrixRotationQuaternion(rit->quaternion);
				XMStoreFloat4(&(*bonemat)[node.boneidx].boneQuatanions, rit->quaternion);
			}
			auto& pos = node.startPos;

			auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
				* rotation
				* XMMatrixTranslation(pos.x, pos.y, pos.z);
			(*bonemat)[node.boneidx].boneMatrieces = mat * XMMatrixTranslationFromVector(offset);
		}
	}

}

void VMDmotion::SetMotionFlag(bool flag)
{
	m_motionFlag = flag;
	if (!m_motionFlag)
	{
		m_oldframeNo = m_frameNo;
	}
	else
	{
		m_starTime = timeGetTime();
	}
}

void VMDmotion::SetNowPose()
{
	auto bonemat = m_model->GetBoneMatrixAndQuatanion();
	//行列情報クリア
	auto f = m_oldframeNo;
	std::fill(bonemat->begin(), bonemat->end(), m_model->MatAndQuatIdentity());

	for (auto& bonemotion : m_motionData)
	{
		auto& nodename = bonemotion.first;
		auto nodetb = m_model->GetBoneNodeTable();
		auto itBoneNode = nodetb->find(nodename);
		if (itBoneNode == nodetb->end())
			continue;
		auto node = itBoneNode->second;
		auto motions = bonemotion.second;
		auto rit = std::find_if(motions.rbegin(), motions.rend(), [f](const Motion& motion)
			{
				return motion.frameNo <= f;
			});
		if (rit == motions.rend())
			continue;

		XMMATRIX rotation = XMMatrixIdentity();
		XMVECTOR offset = XMLoadFloat3(&rit->offset);
		auto it = rit.base();
		if (it != motions.end())
		{
			auto t = (float)(f - rit->frameNo) / (float)(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->p1, it->p2, 12);
			auto quatvec = XMQuaternionSlerp(rit->quaternion, it->quaternion, t);
			rotation = XMMatrixRotationQuaternion(quatvec);
			XMStoreFloat4(&(*bonemat)[node.boneidx].boneQuatanions, quatvec);
			offset = XMVectorLerp(offset, XMLoadFloat3(&it->offset), t);
		}
		else
		{
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
			XMStoreFloat4(&(*bonemat)[node.boneidx].boneQuatanions, rit->quaternion);
		}
		auto& pos = node.startPos;

		auto mat = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		(*bonemat)[node.boneidx].boneMatrieces = mat * XMMatrixTranslationFromVector(offset);
	}
}

void VMDmotion::SetNewMotionFrame(const int f)
{
	if (f <= m_duration)
	{
		m_starTime = timeGetTime();
		m_oldframeNo = f;
		m_frameNo = 0;
		SetNowPose();
	}
	else
	{
		ResetMotion();
	}
}

void VMDmotion::ResetMotion()
{
	m_starTime = timeGetTime();
	m_frameNo = m_oldframeNo = 0;
	SetNowPose();
}

int VMDmotion::GetNowFrame()
{
	if(!m_motionFlag)
		return int(m_oldframeNo); 
	else
		return int(m_frameNo);

}


void VMDmotion::LoadVMD()
{
	FILE* fp = nullptr;
	auto err = fopen_s(&fp,m_filePath.c_str(), "rb");
	fseek(fp, 50, SEEK_SET);
	unsigned int motionDataNum = 0;
	fread(&motionDataNum, sizeof(motionDataNum), 1, fp);

	struct VMDMotion
	{
		char boneName[15];
		unsigned int frameNo;
		XMFLOAT3 location;
		XMFLOAT4 quaternion;
		unsigned char bezier[64];
	};

	std::vector<VMDMotion> vmdmotiondata(motionDataNum);
	for (auto& motion : vmdmotiondata)
	{
		fread(motion.boneName, sizeof(motion.boneName), 1, fp);
		auto datasize = sizeof(motion.frameNo) + sizeof(motion.location) + sizeof(motion.quaternion) + sizeof(motion.bezier);
		fread(&motion.frameNo, datasize, 1, fp);
	}
	uint32_t morphcnt = 0;
	fread(&morphcnt, sizeof(morphcnt), 1, fp);
	m_morphs.resize(morphcnt);
	fread(m_morphs.data(), sizeof(VMDMorph), morphcnt, fp);
	
#pragma pack(1)
	//カメラ
	struct VMDCamera 
	{
		uint32_t frameNo;
		float distance; 
		XMFLOAT3 pos; 
		XMFLOAT3 eulerAngle; 
		uint8_t interpolation[24]; 
		uint32_t fov; 
		uint8_t persFlg; 
	};
#pragma pack()
	uint32_t vmdcameracount = 0;
	fread(&vmdcameracount, sizeof(vmdcameracount), 1, fp);
	vector<VMDCamera> cameraData(vmdcameracount);
	fread(cameraData.data(), sizeof(VMDCamera), vmdcameracount, fp);

	// ライト照明データ
	struct VMDLight 
	{
		uint32_t frameNo; // フレーム番号
		XMFLOAT3 rgb; //ライト色
		XMFLOAT3 vec; //光線ベクトル(平行光線)
	};

	uint32_t vmdlightcount = 0;
	fread(&vmdlightcount, sizeof(vmdlightcount), 1, fp);
	vector<VMDLight> lights(vmdlightcount);
	fread(lights.data(), sizeof(VMDLight), vmdlightcount, fp);

#pragma pack(1)
	// セルフ影データ
	struct VMDSelfShadow 
	{
		uint32_t frameNo; // フレーム番号
		uint8_t mode; //影モード(0:影なし、1:モード１、2:モード２)
		float distance; //距離
	};
#pragma pack()
	uint32_t selfshadowcount = 0;
	fread(&selfshadowcount, sizeof(selfshadowcount), 1, fp);
	vector<VMDSelfShadow> selfShadowData(selfshadowcount);
	fread(selfShadowData.data(), sizeof(VMDSelfShadow), selfshadowcount, fp);

	//IKオンオフ切り替わり数
	uint32_t ikSwitchCount = 0;
	fread(&ikSwitchCount, sizeof(ikSwitchCount), 1, fp);

	m_ikEnableData.resize(ikSwitchCount);
	for (auto& ikEnable : m_ikEnableData) 
	{
		//キーフレーム情報なのでまずはフレーム番号読み込み
		fread(&ikEnable.frameNo, sizeof(ikEnable.frameNo), 1, fp);
		uint8_t visibleflg = 0;
		fread(&visibleflg, sizeof(visibleflg), 1, fp);
		//対象ボーン数読み込み
		uint32_t ikbonecount = 0;
		fread(&ikbonecount, sizeof(ikbonecount), 1, fp);
		//ループしつつ名前とON/OFF情報を取得
		for (int i = 0; i < ikbonecount; ++i)
		{
			char ikbonename[20];
			fread(ikbonename, _countof(ikbonename), 1, fp);
			uint8_t flg = 0;
			fread(&flg, sizeof(flg), 1, fp);
			ikEnable.ikEnableTable[ikbonename] = flg;
		}
	}
	fclose(fp);

	for (auto& vmdmotion : vmdmotiondata)
	{
		auto quaternion = XMLoadFloat4(&vmdmotion.quaternion);
		m_motionData[vmdmotion.boneName].emplace_back(Motion(vmdmotion.frameNo, quaternion, vmdmotion.location, 
			XMFLOAT2((float)vmdmotion.bezier[3] / 127.0f, (float)vmdmotion.bezier[7] / 127.0f),
			XMFLOAT2((float)vmdmotion.bezier[11] / 127.0f, (float)vmdmotion.bezier[15] / 127.0f)));
		m_duration = std::max<unsigned int>(m_duration, vmdmotion.frameNo);
	}
	for (auto& motion : m_motionData)
	{
		std::sort(motion.second.begin(), motion.second.end(),
			[](const Motion& lval, const Motion& rval)
			{
				return lval.frameNo <= rval.frameNo;
			});
	}
}

float VMDmotion::GetYFromXOnBezier(float x, const XMFLOAT2& a, const XMFLOAT2& b, uint8_t n)
{
	if (a.x == a.y && b.x == b.y)
	{
		return x;
	}
	float t = x;

	const float k0 = 1 + 3 * a.x - 3 * b.x;
	const float k1 = 3 * b.x - 6 * a.x;
	const float k2 = 3 * a.x;

	//誤差の範囲かどうかに使用する定数
	constexpr float epsilon = 0.0005f;

	//tの近似で求める
	for (int i = 0; i < n; ++i)
	{
		auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

		if (ft <= epsilon && ft >= -epsilon)
			break;

		t -= ft / 2; //刻む
	}
	auto r = 1 - t;

	return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
}

void VMDmotion::SetTimer()
{
	m_starTime = timeGetTime();
}

void VMDmotion::GetMorphNameFromFrame(std::vector<std::string>& name, std::map<std::string, float>& weight)
{
	unsigned int num = 0;
	name.clear();
	if (m_frameNo > 0)
	{
		for (auto& m : m_morphs)
		{
			if (m.framNo == m_frameNo)
			{
				num++;
				name.resize(num);
				name[num - 1] = m.name;
				weight[m.name] = m.weight;
			}
		}
	}

}

