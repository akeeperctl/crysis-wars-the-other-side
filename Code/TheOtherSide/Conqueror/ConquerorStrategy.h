#pragma once

class CConquerorSystem;
class CConquerorCommander;
struct SConquerorStrategy;

typedef _smart_ptr<SConquerorStrategy> SConquerorStrategyPtr;

struct IStrategyParam
{
	virtual void Reset() = 0;
	//virtual bool IsEmpty() = 0;
	virtual void GetMemoryStatistics(ICrySizer* s) = 0;
};

struct SGenericCondition : public IStrategyParam
{
	string m_type;//Game, Area
	string m_name;

	string m_relationship;//Hostile,Friendly,Any

	string m_conditional1;
	string m_conditional2;
	string m_operator;//Equal, Less, More, AnyOf, AllOf

	float m_fvalue;//float value to test the condition
	string m_svalue;//string value to test the condition

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
		s->Add(m_type);
		s->Add(m_name);
		s->Add(m_relationship);
		s->Add(m_conditional1);
		s->Add(m_conditional2);
		s->Add(m_operator);
		s->Add(m_svalue);
	}

	void Reset() override
	{
		m_type, m_name, m_relationship, m_conditional1, m_conditional2, m_operator, m_svalue = string();
		m_fvalue = 0;
	}

	//bool IsEmpty() override
	//{
	//	return m_archetype.empty() &&
	//		m_behaviour.empty() &&
	//		m_character.empty();
	//}
};

struct SStrategySetting : public IStrategyParam
{
	float m_vehicleUseDistance;
	float m_aggression;
	int m_numberOfAttacks;
	int m_numberOfDefences;
	int m_uncapturableSelect;
	float m_timeLimit;
	string m_targetSpecies;

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
		s->Add(m_targetSpecies);
	}

	void Reset() override
	{
		m_vehicleUseDistance, m_aggression, m_numberOfAttacks, m_numberOfDefences, m_timeLimit, m_uncapturableSelect = 0;
		m_targetSpecies = string();
	}

	//bool IsEmpty() override
	//{
	//	return m_archetype.empty() &&
	//		m_behaviour.empty() &&
	//		m_character.empty();
	//}
};

struct SStrategyPriority : public IStrategyParam
{
	string m_areaFlag;
	float m_priority;
	string m_status;
	string m_targetSpecies;

	void GetMemoryStatistics(ICrySizer* s) override
	{
		s->Add(*this);
		s->Add(m_areaFlag);
		s->Add(m_status);
		s->Add(m_targetSpecies);
	}

	void Reset() override
	{
		m_areaFlag, m_status, m_targetSpecies = string();
		m_priority = 0;
	}

	//bool IsEmpty() override
	//{
	//	return m_archetype.empty() &&
	//		m_behaviour.empty() &&
	//		m_character.empty();
	//}
};

struct SConquerorStrategy
{
public:
	friend class CConquerorSystem;
	//friend class CConquerorCommander;

	SConquerorStrategy()
	{
		//m_pOwner = nullptr;
		m_index = -1;
		m_refs = 0;
		m_name = "";
		m_settings.Reset();
		m_conditions.clear();
		m_goals.clear();
		m_priorities.clear();
	}

	SConquerorStrategy(const char* name)
	{
		m_index = -1;
		m_refs = 0;
		m_name = name;
		m_settings.Reset();
		m_conditions.clear();
		m_goals.clear();
		m_priorities.clear();
	}

	const auto GetIndex() const
	{
		return m_index;
	}

	const auto& GetConditions()
	{
		return m_conditions;
	}
	const auto& GetGoals()
	{
		return m_goals;
	}

	const auto GetPriorities() const
	{
		return m_priorities;
	}

	const auto& GetSettings() const
	{
		return m_settings;
	}

	const char* GetName() const
	{
		return m_name.c_str();
	}

	virtual void AddRef() const { ++m_refs; };
	virtual uint GetRefCount() const { return m_refs; };
	virtual void Release() const
	{
		if (--m_refs <= 0)
			delete this;
	}

	void GetMemoryStatistics(ICrySizer* s)
	{
		s->Add(*this);
		s->Add(m_name);
		
		m_settings.GetMemoryStatistics(s);
		
		for (auto& value : m_conditions)
			value.GetMemoryStatistics(s);

		for (auto& value : m_goals)
			value.GetMemoryStatistics(s);

		for (auto& value : m_priorities)
			value.GetMemoryStatistics(s);
	}
protected:
	mutable uint	m_refs;
private:
	//CConquerorCommander* m_pOwner;
	uint m_index;
	string m_name;
	SStrategySetting m_settings;

	std::vector<SGenericCondition> m_conditions;
	std::vector<SGenericCondition> m_goals;
	std::vector<SStrategyPriority> m_priorities;
};