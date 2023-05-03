#ifndef PROP_WEIGHTED_CUBE_H
#define PROP_WEIGHTED_CUBE_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "player_pickup.h"
#include "particle_system.h"
#include "sdk23/sdk23_const.h"
#include "props.h"

class CPropWeightedCube : public CPhysicsProp {
public:
	DECLARE_CLASS(CPropWeightedCube, CPhysicsProp);

	void Precache(void);
	void Spawn(void);
	QAngle PreferredCarryAngles(void);

	bool HasPreferredCarryAnglesForPlayer(CBasePlayer* pPlayer) { return true; };
	bool OnAttemptPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason) { return true; };
	bool CanBePickedUpByPhyscannon() { return true; };

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
	{
		CBasePlayer* pPlayer = ToBasePlayer(pActivator);
		if (pPlayer)
		{
			pPlayer->PickupObject(this, false);
		}
	}

	// Set "usability"
	int	ObjectCaps(void)
	{
		return FCAP_IMPULSE_USE;// | FCAP_USE_IN_RADIUS) : 0));
	}

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