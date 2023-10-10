#include "stdafx.h"

#include <CryFile.h>
#include <ISystem.h>

#include "vertexlist.h"

CVertexList::CVertexList()
{
	m_vList.clear();
}

CVertexList::~CVertexList() {}

int CVertexList::AddVertex(const ObstacleData& od)
{
	int index = FindVertex(od);
	if (index < 0)
	{
		m_vList.push_back(od);
		index = static_cast<int>(m_vList.size()) - 1;
	}

	return index;
}

int CVertexList::FindVertex(const ObstacleData& od)
{
	const auto oiend = m_vList.end();
	auto       oi = m_vList.begin();
	int        index = 0;

	for (; oi != oiend; ++oi, index++)
	{
		if (*oi == od)
			return index;
	}

	return -1;
}

ObstacleData CVertexList::GetVertex(const int index) const
{
	if (index < 0 || index >= static_cast<int>(m_vList.size()))
		CryError("[AISYSTEM] Tried to retrieve a non existing vertex from vertex list.Please regenerate the triangulation and re-export the map.");

	return m_vList[index];
}

ObstacleData& CVertexList::ModifyVertex(const int index)
{
	if (index < 0 || index >= static_cast<int>(m_vList.size()))
		CryError("[AISYSTEM] Tried to retrieve a non existing vertex from vertex list.Please regenerate the triangulation and re-export the map.");

	return m_vList[index];
}

void CVertexList::WriteToFile(CCryFile& file) const
{
	const int iNumber = static_cast<int>(m_vList.size());
	file.Write(&iNumber, sizeof(int));
	if (!iNumber)
		return;
	file.Write(m_vList.data(), iNumber * sizeof(ObstacleData));
}

void CVertexList::ReadFromFile(CCryFile& file)
{
	int iNumber;
	file.ReadRaw(&iNumber, sizeof(int));

	if (iNumber > 0)
	{
		m_vList.resize(iNumber);
		file.ReadRaw(m_vList.data(), iNumber * sizeof(ObstacleData));
	}
}
