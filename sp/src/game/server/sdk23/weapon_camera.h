//========= (not) Copyright Valve Corporation, All rights reserved. ============//
//
// Created by Bluebotlabz/thatHackerDudeFromCyberspace
// DO NOT DELETE THIS NOTICE
// 
// Purpose:		
//
// $NoKeywords: $
//==============================================================================//

#include "basehlcombatweapon.h"
#include "soundenvelope.h"
#include "camera_placement.h"

#ifndef WEAPON_CAMERA_H
#define WEAPON_CAMERA_H
#ifdef _WIN32
#pragma once
#endif



//---------------------
// Camera Entity
//---------------------
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
	void PrimaryAttack(void);							// Used to Zoom, Capture and Place
	void SecondaryAttack(void);							// Used to enter/exit Placement Mode
	void ItemPostFrame(void);							// Used to get input

	void InitPlacementController(bool showEntity);		// Used to initialise the placement controller (and also enters placement mode)
	void DeInitPlacementController(bool hideEntity);	// Used to detach the entity from the placement controller (and also exits placement mode)
	void PlacementThink(void);							// Handle placement "logic"
	
	void SetZoom(bool zoom);							// Used to enter the Zoom HUD or Exit it
	void SetSlot(int slot);								// Used to change the camera slot whilst in placement mode
	void SetScale(bool scaleUp);						// Used to change the scale of an object whilst in placement mode

	DECLARE_DATADESC();
private:
	CameraState m_iCameraState;
	CUtlVector<CCameraEntity> m_vInventory;
	int m_iCurrentInventorySlot;

	bool m_bButtonsPressed; // To stop autofire

	float m_flNextScale;

	CPlacementController m_placementController;
};

#endif // WEAPON_FLAREGUN_H

