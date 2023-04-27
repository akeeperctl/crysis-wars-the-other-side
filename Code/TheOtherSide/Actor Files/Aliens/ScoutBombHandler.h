#pragma once

struct SBombInfo
{
	EntityId entityId;
	Vec3 localPosition;
};

class CScoutBombHandler
{
public:

	CScoutBombHandler();
	~CScoutBombHandler();

	
	//std::vector<Vec3> m_localBombPositions[3];
	
	std::vector<SBombInfo> m_bombs;

	//local relative to the scout
	Vec3 m_localCenterPosition;

	int m_currentBombCount;

};