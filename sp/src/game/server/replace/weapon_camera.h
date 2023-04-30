//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "basehlcombatweapon.h"
#include "soundenvelope.h"

#ifndef WEAPON_CAMERA_H
#define WEAPON_CAMERA_H
#ifdef _WIN32
#pragma once
#endif
class CCameraEntity {
public:
	void SetEntity(CBaseEntity* entity) { m_hEntity = entity; };
	CBaseEntity* GetEntity(void) { return dynamic_cast<CBaseEntity*>(m_hEntity.Get()); };

	void CaptureEntity(void);
	void RestoreEntity(void);
	void HideEntity(void);
	void ShowEntity(void);

	DECLARE_SIMPLE_DATADESC();
private:
	EHANDLE m_hEntity;

	SolidType_t m_iSolidType;
	MoveType_t m_iMoveType;
	int m_iEffects;
	bool m_bAwake;
};


enum CameraState {
	CAMERA_NORMAL = 0,
	CAMERA_ZOOM,
	CAMERA_PLACEMENT
};

//---------------------
// Camera
//---------------------
class CWeaponCamera : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeaponCamera, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();

	void Precache(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void ItemPostFrame(void);

	void SetZoom(bool zoom);
	void SetSlot(int slot);
	void SetScale(bool scaleUp);

	void PlacementThink(void);

	DECLARE_DATADESC();
private:
	CameraState m_iCameraState;
	CUtlVector<CCameraEntity> m_vInventory;
	int m_iCurrentInventorySlot;

	bool m_bButtonsPressed; // To stop autofire

	float m_flNextScale;
};

#endif // WEAPON_FLAREGUN_H

