//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef ENVPORTALLASER_H
#define ENVPORTALLASER_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "beam_shared.h"
#include "entityoutput.h"

class CSprite;


class CEnvPortalLaser : public CBeam
{
	DECLARE_CLASS(CEnvPortalLaser, CBeam);
public:
	void	Spawn(void);
	void	Precache(void);

	void	TurnOn(void);
	void	TurnOff(void);
	int		IsOn(void);

	void	LaserThink(void);

	void InputTurnOn(inputdata_t& inputdata);
	void InputTurnOff(inputdata_t& inputdata);
	void InputToggle(inputdata_t& inputdata);

	COutputEvent	m_OnTouchedByEntity;

	DECLARE_DATADESC();

private:
	float m_fNextSparkTime;

	bool m_bStartDisabled;
	bool m_bDoDamage;
	bool m_bAutoAimLaser;
	bool m_bRustedSkin;

	CNetworkColor32(m_clrBeamColour);
	CNetworkColor32(m_clrSpriteColour);

	bool m_bDisableCollision;
};

#endif // ENVLASER_H
