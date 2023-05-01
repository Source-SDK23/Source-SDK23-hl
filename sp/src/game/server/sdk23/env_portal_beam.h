//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ENVPORTALBEAM_H
#define ENVPORTALBEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "beam_shared.h"
#include "entityoutput.h"
#include "sdk23/sdk23_const.h"

class CSprite;


class CEnvPortalBeam : public CBeam
{
	DECLARE_CLASS(CEnvPortalBeam, CBeam);
public:
	void	Spawn(void);
	void	Precache(void);

	void	TurnOn(void);
	void	TurnOff(void);
	bool	GetState(void);

	void	BeamThink(void);

	void InputTurnOn(inputdata_t& inputdata);
	void InputTurnOff(inputdata_t& inputdata);
	void InputToggle(inputdata_t& inputdata);

	COutputEvent	m_OnTouchedByEntity;

	DECLARE_DATADESC();

private:
	float m_fNextSparkTime;

	bool m_bStartDisabled;
	bool m_bDoDamage;
	bool m_bAutoAimBeam;
	SkinType m_bRustedSkin;

	CNetworkColor32(m_clrBeamColour);
	CNetworkColor32(m_clrSpriteColour);

	bool m_bDisableCollision;
};

#endif // ENVPORTALBEAM_H
