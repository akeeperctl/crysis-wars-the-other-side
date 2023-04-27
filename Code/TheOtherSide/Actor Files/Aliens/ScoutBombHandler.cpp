#include "StdAfx.h"
#include "ScoutBombHandler.h"

CScoutBombHandler::CScoutBombHandler() :
	m_localCenterPosition(0),
	m_currentBombCount(0)
{
	m_bombs.clear();
}

CScoutBombHandler::~CScoutBombHandler()
{
	delete this;
}