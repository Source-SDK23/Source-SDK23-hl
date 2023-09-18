//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Created by Bluebotlabz/thatHackerDudeFromCyberspace
// DO NOT DELETE THIS NOTICE
// 
// Purpose:		
//
// $NoKeywords: $
//========================================================================//

#include "basehlcombatweapon.h"
#include "soundenvelope.h"
#include "weapon_physcannon.h"

#ifndef CAMERA_PLACEMENT_H
#define CAMERA_PLACEMENT_H
#ifdef _WIN32
#pragma once
#endif

void CameraPickupObject(CBasePlayer* pPlayer, CBaseEntity* pObject);



//-----------------------------------------------------------------------------
class CPlacementController : public IMotionEvent
{
	DECLARE_SIMPLE_DATADESC();

public:

	CPlacementController(void);
	~CPlacementController(void);
	void AttachEntity(CBasePlayer* pPlayer, CBaseEntity* pEntity, IPhysicsObject* pPhys, bool bIsMegaPhysCannon, const Vector& vGrabPosition, bool bUseGrabPosition);
	void DetachEntity(bool bClearVelocity);
	void OnRestore();

	bool UpdateObject(CBasePlayer* pPlayer);

	void SetTargetPosition(const Vector& target, const QAngle& targetOrientation);
	float ComputeError();
	float GetLoadWeight(void) const { return m_flLoadWeight; }
	void SetAngleAlignment(float alignAngleCosine) { m_angleAlignment = alignAngleCosine; }
	void SetIgnorePitch(bool bIgnore) { m_bIgnoreRelativePitch = bIgnore; }
	QAngle TransformAnglesToPlayerSpace(const QAngle& anglesIn, CBasePlayer* pPlayer);
	QAngle TransformAnglesFromPlayerSpace(const QAngle& anglesIn, CBasePlayer* pPlayer);

	CBaseEntity* GetAttached() { return (CBaseEntity*)m_attachedEntity; }

	IMotionEvent::simresult_e Simulate(IPhysicsMotionController* pController, IPhysicsObject* pObject, float deltaTime, Vector& linear, AngularImpulse& angular);
	float GetSavedMass(IPhysicsObject* pObject);

	bool IsObjectAllowedOverhead(CBaseEntity* pEntity);

private:
	// Compute the max speed for an attached object
	void ComputeMaxSpeed(CBaseEntity* pEntity, IPhysicsObject* pPhysics);

	game_shadowcontrol_params_t	m_shadow;
	float			m_timeToArrive;
	float			m_errorTime;
	float			m_error;
	float			m_contactAmount;
	float			m_angleAlignment;
	bool			m_bCarriedEntityBlocksLOS;
	bool			m_bIgnoreRelativePitch;

	float			m_flLoadWeight;
	float			m_savedRotDamping[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	float			m_savedMass[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	EHANDLE			m_attachedEntity;
	QAngle			m_vecPreferredCarryAngles;
	bool			m_bHasPreferredCarryAngles;
	float			m_flDistanceOffset;

	QAngle			m_attachedAnglesPlayerSpace;
	Vector			m_attachedPositionObjectSpace;

	IPhysicsMotionController* m_controller;

	bool			m_bAllowObjectOverhead; // Can the player hold this object directly overhead? (Default is NO)

	// NVNT player controlling this grab controller
	CBasePlayer* m_pControllingPlayer;

	friend class CWeaponPhysCannon;
};

#endif