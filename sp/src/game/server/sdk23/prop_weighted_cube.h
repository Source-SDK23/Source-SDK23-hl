#ifndef PROP_WEIGHTED_CUBE_H
#define PROP_WEIGHTED_CUBE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player_pickup.h"
#include "particle_system.h"
#include "sdk23/sdk23_const.h"

class CPropWeightedCube : public CBaseAnimating, public CDefaultPlayerPickupVPhysics{
public:
	DECLARE_CLASS(CPropWeightedCube, CBaseAnimating);

	void Precache(void);
	void Spawn(void);
	QAngle PreferredCarryAngles(void);

	bool HasPreferredCarryAnglesForPlayer(CBasePlayer* pPlayer) { return true; }

	DECLARE_DATADESC()
private:
	EHANDLE m_hLaser;
	EHANDLE m_hParticle;
	int m_iIdleSkin;
	int m_iActiveSkin;

	CubeType m_iCubeType;
	RustedSkinType m_iSkinType;

	PaintType m_iPaintType;

	
	bool m_bUseLaserModifier;
	CNetworkColor32(m_clrLaserModifier);

	bool m_bUseLaserFilter;
	CNetworkColor32(m_clrLaserFilter);

};

#endif