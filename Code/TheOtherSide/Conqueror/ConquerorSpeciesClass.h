#pragma once

#include "ConquerorStrategy.h";

class CSpeciesClass;
typedef _smart_ptr<CSpeciesClass> CSpeciesClassPtr;

enum ESpeciesClassFlags
{
	eSCF_UnlockedByArea = BIT(0),
	eSCF_NeedHumanMode = BIT(1),
	eSCF_LeaderClass = BIT(2),
	//eSCF_NonLeaderClass = BIT(3),
	eSCF_OnlyThirdPerson = BIT(3),
	eSCF_IsAir = BIT(4),
	eSCF_UnlockedForAI = BIT(5),
	eSCF_UnlockedForPlayer = BIT(6),
	//eSCF_UnlockedForAll = eSCF_UnlockedForAI | eSCF_UnlockedForPlayer,
};

struct IClassParam
{
	virtual void Reset() = 0;
	virtual bool IsEmpty() = 0;
	virtual void GetMemoryStatistics(ICrySizer* s) = 0;
};

struct SClassAI : public IClassParam
{
	string	m_archetype;
	string	m_character;
	string	m_behaviour;

	SClassAI() {};
	SClassAI(const char* _archetype, const char* _character, const char* _behaviour)
	{
		m_archetype = _archetype;
		m_character = _character;
		m_behaviour = _behaviour;
	};

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
		s->Add(m_archetype);
		s->Add(m_character);
		s->Add(m_behaviour);
	}

	void Reset() override
	{
		m_archetype, m_behaviour, m_character = "";
	}

	bool IsEmpty() override
	{
		return m_archetype.empty() &&
			m_behaviour.empty() &&
			m_character.empty();
	}
};

struct SClassModel : public IClassParam
{
	string	m_lobbyAnim;
	string	m_character;
	string	m_arms;
	string	m_fp3p;
	string	m_mat;
	string	m_armsMat;
	string	m_helmetMat;
	Vec3 m_worldOffset;
	float m_scale;

	SClassModel() {};
	SClassModel(string _character,
		string _lobbyAnim = "",
		string _mat = "",
		string _arms = "",
		string _fp3p = "",
		string _armsMat = "",
		string _helmetMat = "",
		Vec3 _worldOffset = Vec3(0, 0, 0),
		float _scale = 1.0f
	)
	{
		m_character = _character;
		m_lobbyAnim = _lobbyAnim;
		m_arms = _arms;
		m_fp3p = _fp3p;
		m_mat = _mat;
		m_armsMat = _armsMat;
		m_helmetMat = _helmetMat;
		m_worldOffset = _worldOffset;
		m_scale = _scale;
	};

	void Reset() override
	{
		m_character, m_lobbyAnim, m_arms, m_fp3p, m_mat, m_armsMat, m_helmetMat = string();
		m_worldOffset = Vec3(0, 0, 0);
		m_scale = 0;
	}

	bool IsEmpty() override
	{
		return m_character.empty() &&
			m_lobbyAnim.empty() &&
			m_arms.empty() &&
			m_fp3p.empty() &&
			m_mat.empty() &&
			m_armsMat.empty() &&
			m_helmetMat.empty();
	}

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(m_arms);
		s->Add(m_armsMat);
		s->Add(m_character);
		s->Add(m_fp3p);
		s->Add(m_helmetMat);
		s->Add(m_lobbyAnim);
		s->Add(m_mat);
	}

	void operator = (SClassModel& modelInfo)
	{
		m_character = modelInfo.m_character;
		m_lobbyAnim = modelInfo.m_lobbyAnim;
		m_arms = modelInfo.m_arms;
		m_fp3p = modelInfo.m_fp3p;
		m_mat = modelInfo.m_mat;
		m_armsMat = modelInfo.m_armsMat;
		m_helmetMat = modelInfo.m_helmetMat;
		m_worldOffset = modelInfo.m_worldOffset;
		m_scale = modelInfo.m_scale;
	}
};

struct SClassEquipment : public IClassParam
{
	SClassEquipment() {};
	SClassEquipment(const char* equipPack)
	{
		//m_primaryWeapon = primaryWeapon;
		m_equipPack = equipPack;
	}

	//string m_primaryWeapon;
	string m_equipPack;

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
		s->Add(m_equipPack);
	}

	void Reset() override
	{
		//m_primaryWeapon, m_equipPack = string();
		m_equipPack = string();
	}

	bool IsEmpty() override
	{
		return m_equipPack.empty();
		//return m_primaryWeapon.empty() &&
		//	m_equipPack.empty();
	}
};

class CSpeciesClass
{
public:
	friend class CConquerorSystem;

	CSpeciesClass()
	{
		Reset();
	}

	auto& GetModel() const noexcept { return m_model; }
	auto& GetEquipment() const noexcept { return m_equipment; }
	auto& GetAI() const noexcept { return m_ai; }
	auto& GetAbilities() const noexcept { return m_abilities; }
	const auto GetConditions() const noexcept { return m_conditions;}

	const inline bool IsOnlyThirdPerson() { return stl::find(m_flags, eSCF_OnlyThirdPerson); }
	const inline bool IsNeedHumanMode() { return stl::find(m_flags, eSCF_NeedHumanMode); }
	const inline bool IsLeaderClass() { return stl::find(m_flags, eSCF_LeaderClass); }
	const inline bool IsNonLeaderClass() { return !stl::find(m_flags, eSCF_LeaderClass); }
	const inline bool IsAirClass() { return stl::find(m_flags, eSCF_IsAir); }

	auto GetFlags() const noexcept { return m_flags; }

	auto* GetName() const noexcept { return m_name.c_str(); }

	void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
		s->Add(m_name);
		s->AddContainer(m_abilities);
		m_model.GetMemoryStatistics(s);
		m_ai.GetMemoryStatistics(s);
		m_equipment.GetMemoryStatistics(s);
	}

	void Reset()
	{
		//m_unlockedByArea = false;
		//m_onlyThirdPerson = false;
		//m_needHumanMode = false;

		m_flags.clear();

		m_name = "";
		m_abilities.clear();

		m_ai.Reset();
		m_equipment.Reset();
		m_model.Reset();

		m_refs = 0;
	}

	bool CheckFlag(ESpeciesClassFlags flag) const { return stl::find(m_flags, flag); }

	virtual void AddRef() const { ++m_refs; };
	virtual uint GetRefCount() const { return m_refs; };
	virtual void Release() const
	{
		if (--m_refs <= 0)
			delete this;
	}

protected:

	//void SetFlags(uint flags) noexcept { m_flags = flags; };
	//void SetNeedHumanMode(bool hm) noexcept { m_needHumanMode = hm; }
	//void SetOnlyThirdPerson(bool tp) noexcept { m_onlyThirdPerson = tp; }
	void SetName(const char* name) noexcept { m_name = name; }
	bool inline PushFlag(ESpeciesClassFlags flag) { return stl::push_back_unique(m_flags, flag); }
	bool inline CleanFlag(ESpeciesClassFlags flag) { return stl::find_and_erase(m_flags, flag); }
	//void SetIsLeader(bool lr) noexcept { m_isLeaderClass = lr; };

	mutable uint	m_refs;
private:

	string m_name;
	std::vector<ESpeciesClassFlags> m_flags;

	SClassModel m_model;
	SClassEquipment m_equipment;
	SClassAI m_ai;
	
	std::vector<SGenericCondition> m_conditions;
	std::set<string> m_abilities;

protected:
};