//===== ENV_PORTAL_LASER//
// For the actual laser emitter entity
//

#ifndef ENVPORTALLASER_H
#define ENVPORTALLASER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "sdk23/sdk23_const.h"

class CEnvPortalLaser : public CBaseAnimating {
public:
	DECLARE_CLASS(CEnvPortalLaser, CBaseAnimating);
	void Precache(void);
	void Spawn(void);

	void InputTurnOn(inputdata_t& inputdata);
	void InputTurnOff(inputdata_t& inputdata);
	void InputToggle(inputdata_t& inputdata);

	bool TurnOn(void);
	bool TurnOff(void);

	DECLARE_DATADESC()
private:
	EHANDLE m_hBeam;
	EHANDLE m_hParticles;

	bool m_bStartDisabled;
	bool m_bInstaKill;
	bool m_bAutoAimBeam;
	SkinType m_iSkintype;

	CNetworkColor32(m_clrBeamColour);
	CNetworkColor32(m_clrSpriteColour);

	bool m_bDisableCollision;
};

#endif ENVPORTALLASER_H