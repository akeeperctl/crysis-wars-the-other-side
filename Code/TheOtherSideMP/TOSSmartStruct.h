#pragma once

/**
 * \brief Используется вместе с _smart_ptr
 */
struct STOSSmartStruct  // NOLINT(cppcoreguidelines-special-member-functions)
{
	STOSSmartStruct(): m_refs(0) {}
	virtual ~STOSSmartStruct() = default;
	virtual void AddRef() const { ++m_refs; };
	virtual uint GetRefCount() const { return m_refs; };
	virtual void Release() const
	{
		if (--m_refs <= 0)
		{
			delete this;
		}
	}

	mutable uint m_refs;
};
