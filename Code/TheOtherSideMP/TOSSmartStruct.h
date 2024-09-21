/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once

// Используется вместе с _smart_ptr
// Пример: std::vector<_smart_ptr<STOSSmartStruct>> myVec;
struct STOSSmartStruct
{
	STOSSmartStruct() : m_refs(0)
	{}
	virtual ~STOSSmartStruct()
	{};
	virtual void AddRef() const
	{
		++m_refs;
	};
	virtual uint GetRefCount() const
	{
		return m_refs;
	};
	virtual void Release() const
	{
		if (--m_refs <= 0)
		{
			delete this;
		}
	}

	mutable uint m_refs;
};
