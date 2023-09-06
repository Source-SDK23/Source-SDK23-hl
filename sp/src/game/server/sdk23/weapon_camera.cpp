//========= (not) Copyright Valve Corporation, All rights reserved. ============//
//
// Created by Bluebotlabz/thatHackerDudeFromCyberspace
// DO NOT DELETE THIS NOTICE
// 
// Purpose:		Implements the FStop camera
//
// $NoKeywords: $
//==============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "basehlcombatweapon.h"
#include "decals.h"
#include "soundenvelope.h"
#include "IEffects.h"
#include "engine/IEngineSound.h"
#include "vphysics/friction.h"
#include "weapon_camera.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <saverestore_utlvector.h>
#include <in_buttons.h>
#include <props.h>


#define	CAMERA_MAX_INVENTORY 3 // Maximum inventory  slots
#define CAMERA_SCALE_RATE 0.3f

// Random sounds, configurable
#define CAMERA_ZOOM_SOUND "Tile.StepRight"						// Sound for switching to zoom mode
#define CAMERA_PLACEMENT_SOUND "Cardboard.ImpactSoft"			// Sound for switching to placement mode
#define CAMERA_PLACE_SOUND "Metal_Box.ImpactSoft"				// Sound for placing object
#define CAMERA_CAPTURE_SOUND "NPC_CScanner.TakePhoto"			// Sound for capturing object
#define CAMERA_SCALE_UP_SOUND "NPC_Alyx.Climb_Pipe_strain_1"	// Sound for scaling object up
#define CAMERA_SCALE_DOWN_SOUND "NPC_Alyx.Climb_Pipe_strain_2"	// Sound for scaling object down

static const int CameraScalesLen = 5; // Max number of possible scales (do not ask why this isn't generated at runtime)
static const float CameraScales[] = {
	0.25, 0.5, 1.0, 2.0, 4.0
};

static const int CaptureBlacklistLen = 2; // Length of blacklist
static const char* CaptureBlacklist[] = { // Never capture these entities
	"worldspawn",			// DO NOT CAPTURE THE LEVEL
	"prop_vehicle_jeep"		// Crashes game during placement
};



// Handles scaling control
void CameraScale_f(const CCommand& args) {
	if (args.ArgC() < 1 || !args.Arg(1)) {
		Msg("Usage: camera_scale <1|0>\n");
		return;
	}

	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	CWeaponCamera* pWeapon = dynamic_cast<CWeaponCamera*>(pPlayer->GetActiveWeapon());

	if (pWeapon == NULL) {
		Msg("Player has no weapon");
		return;
	}

	if (!FClassnameIs(pWeapon, "weapon_camera")) {
		Msg("Player is not currently using a weapon_camera\n");
		return;
	}

	pWeapon->SetScale(strcmp(args.Arg(1), "1") == 0);
}
ConCommand CameraScale("camera_scale", CameraScale_f, "camera_scale 1", 0);

// Handles changing camera slot control
void CameraSlot_f(const CCommand& args) {
	if (args.ArgC() < 1 || !args.Arg(1)) {
		Msg("Usage: camera_slot <slot#>\n");
		return;
	}

	CBasePlayer* pPlayer = UTIL_GetCommandClient();
	CWeaponCamera* pWeapon = dynamic_cast<CWeaponCamera*>(pPlayer->GetActiveWeapon());

	if (pWeapon == NULL) {
		Msg("Player has no weapon");
		return;
	}

	if (!FClassnameIs(pWeapon, "weapon_camera")) {
		Msg("Player is not currently using a weapon_camera\n");
		return;
	}

	pWeapon->SetSlot(atoi(args.Arg(1)));
}
ConCommand CameraSlot("camera_slot", CameraSlot_f, "camera_slot 2", 0);






//-----------------------------------------------------------------------------
// Purpose: Store captured objects
//-----------------------------------------------------------------------------
BEGIN_SIMPLE_DATADESC(CCameraEntity)
	DEFINE_FIELD(m_hEntity, FIELD_EHANDLE),
	DEFINE_FIELD(m_iSolidType, FIELD_INTEGER),
	DEFINE_FIELD(m_iMoveType, FIELD_INTEGER),
	DEFINE_FIELD(m_iEffects, FIELD_INTEGER),
	DEFINE_FIELD(m_bAwake, FIELD_BOOLEAN),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: Capture an prop and store its metadata
//-----------------------------------------------------------------------------
void CCameraEntity::CaptureEntity(void) {
	CBaseEntity* baseEntity = m_hEntity.Get();

	// Stores original data of the captured entity
	m_iSolidType = baseEntity->GetSolid();
	m_iMoveType = baseEntity->GetMoveType();
	m_iEffects = baseEntity->GetEffects();
	m_bAwake = baseEntity->VPhysicsGetObject()->IsGravityEnabled() || baseEntity->VPhysicsGetObject()->IsMotionEnabled();

	baseEntity->SetSolid(SOLID_NONE);
	baseEntity->SetMoveType(MOVETYPE_NONE);

	HideEntity();
}


//-----------------------------------------------------------------------------
// Purpose: Restores an prop
//-----------------------------------------------------------------------------
void CCameraEntity::RestoreEntity(void) {
	CBaseEntity* baseEntity = m_hEntity.Get();

	baseEntity->SetSolid(m_iSolidType);
	baseEntity->SetMoveType(m_iMoveType);

	if (m_bAwake) {
		baseEntity->VPhysicsGetObject()->Wake();
	}

	ShowEntity();
}


//-----------------------------------------------------------------------------
// Purpose: Shows entity preview (not yet transparent)
//-----------------------------------------------------------------------------
void CCameraEntity::ShowEntity(void) {
	CBaseEntity* baseEntity = m_hEntity.Get();

	baseEntity->SetEffects(m_iEffects);
	baseEntity->VPhysicsGetObject()->SetPosition(baseEntity->GetAbsOrigin(), baseEntity->GetAbsAngles(), true); // Update collider
}


//-----------------------------------------------------------------------------
// Purpose: Hides entity preview
//-----------------------------------------------------------------------------
void CCameraEntity::HideEntity(void) {
	CBaseEntity* baseEntity = m_hEntity.Get();

	baseEntity->SetAbsOrigin(Vector(3000, 3000, 3000));
	baseEntity->AddEffects(EF_NODRAW);
	baseEntity->VPhysicsGetObject()->SetPosition(baseEntity->GetAbsOrigin(), baseEntity->GetAbsAngles(), true); // Update collider
}






//-----------------------------------------------------------------------------
// Purpose: THE MAIN CAMERA CLASS
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CWeaponCamera)
	DEFINE_FIELD(m_iCameraState, FIELD_INTEGER),
	DEFINE_UTLVECTOR(m_vInventory, FIELD_EMBEDDED),
	DEFINE_FIELD(m_iCurrentInventorySlot, FIELD_INTEGER),
	DEFINE_FIELD(m_bButtonsPressed, FIELD_BOOLEAN),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeaponCamera, DT_WeaponCamera)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_camera, CWeaponCamera);
PRECACHE_WEAPON_REGISTER(weapon_camera);

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CWeaponCamera::Precache(void)
{
	BaseClass::Precache();

	//PrecacheScriptSound("Flare.Touch");
	//PrecacheScriptSound("Weapon_FlareGun.Burn");
	//UTIL_PrecacheOther("env_flare");

	PrecacheScriptSound( CAMERA_ZOOM_SOUND );
	PrecacheScriptSound( CAMERA_PLACEMENT_SOUND );
	PrecacheScriptSound( CAMERA_PLACE_SOUND );
	PrecacheScriptSound( CAMERA_CAPTURE_SOUND );
	PrecacheScriptSound( CAMERA_SCALE_UP_SOUND );
	PrecacheScriptSound( CAMERA_SCALE_DOWN_SOUND );
}


//-----------------------------------------------------------------------------
// Purpose: Keypress detection (stolen from... something else... I don't remember what tho)
//-----------------------------------------------------------------------------
void CWeaponCamera::ItemPostFrame(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	// -----------------------
	//  No buttons down
	// -----------------------
	if (!((pOwner->m_nButtons & IN_ATTACK) || (pOwner->m_nButtons & IN_ATTACK2))) // When no buttons are pressed, reset m_bButtonsPressed
	{
		m_bButtonsPressed = false;
	}
	if (m_bButtonsPressed) {
		return; // Disable autofire
	}

	//Track the duration of the fire
	//FIXME: Check for IN_ATTACK2 as well?
	//FIXME: What if we're calling ItemBusyFrame?
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// Secondary attack has priority
	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
		{
			// Don't do anything, just cancel the whole function
			return;
		}
		else {
			SecondaryAttack();
		}
		m_bButtonsPressed = true;
	}

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		if (pOwner->HasSpawnFlags(SF_PLAYER_SUPPRESS_FIRING))
		{
			// Don't do anything, just cancel the whole function
			return;
		}
		else {
			// If the firing button was just pressed, or the alt-fire just released, reset the firing time
			if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_ATTACK2))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime;
			}

			PrimaryAttack();
			m_bButtonsPressed = true;
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Main button (place object, switch to zoom mode, capture)
//-----------------------------------------------------------------------------
void CWeaponCamera::PrimaryAttack(void)
{
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.1f;
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	Vector facingVector;
	trace_t tr;

	// For sounds
	CPASAttenuationFilter filter(this);

	// Basically a state machine /j
	switch (m_iCameraState) {
	case CAMERA_NORMAL:
		SetZoom(true); // Zoom in
		m_iCameraState = CAMERA_ZOOM; // Zoom
		break;

	case CAMERA_ZOOM:
		// Unzoom
		SetZoom(false);
		m_iCameraState = CAMERA_NORMAL;

		if (m_vInventory.Count() == CAMERA_MAX_INVENTORY) {
			Msg("CANNOT CAPTURE! INVENTORY IS FULL");
			return;
		}

		// Perform traceline to find object to capture
		AngleVectors(pOwner->EyeAngles(), &facingVector);
		UTIL_TraceLine(pOwner->EyePosition(), pOwner->EyePosition() + (facingVector * MAX_TRACE_LENGTH), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

		if (tr.m_pEnt) {
			for (int i = 0; i < CaptureBlacklistLen; i++) {
				if (FClassnameIs(tr.m_pEnt, CaptureBlacklist[i])) {
					Msg("Cannot capture object, blacklisted");
					return;
				}
			}

			// Create new cameraEntity
			CCameraEntity camEntity;
			camEntity.SetEntity(tr.m_pEnt);
			camEntity.CaptureEntity();

			// Add cameraEntity to inventory
			m_vInventory.AddToTail(camEntity);

			EmitSound(filter, entindex(), CAMERA_CAPTURE_SOUND);
		}
		break;

	case CAMERA_PLACEMENT: // If in placement mode, place the object
		CCameraEntity camEntity = m_vInventory[m_iCurrentInventorySlot];
		SetThink(NULL); // No longer in placement mode
		PlacementThink(); // This is required, don't remember why tho
		camEntity.RestoreEntity();
		m_vInventory.Remove(m_iCurrentInventorySlot); // Remove from inventory
		m_iCameraState = CAMERA_NORMAL;

		m_iCurrentInventorySlot = max(m_vInventory.Count() - 1, 0); // Set inventory slot to either 0 or last item
		EmitSound(filter, entindex(), CAMERA_PLACE_SOUND);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Placement mode button (exit zoom mode, switch to placement mode)
//-----------------------------------------------------------------------------
void CWeaponCamera::SecondaryAttack(void)
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Switches directly to placement mode
	if (m_iCameraState == CAMERA_ZOOM) { // Always exit soom
		SetZoom(false); // Unzoom
		m_iCameraState = CAMERA_NORMAL;
	}
	if (m_iCameraState == CAMERA_PLACEMENT) { // Exit placement mode if already in it
		SetThink(NULL);
		m_vInventory[m_iCurrentInventorySlot].HideEntity();
		m_iCameraState = CAMERA_NORMAL;
		return;
	}

	// Switch to placement mode
	if (m_vInventory.Count() == 0) {
		Msg("Cannot enter placement mode, empty inventory");
		return;
	}
	if (m_iCurrentInventorySlot == m_vInventory.Count()) {
		m_iCurrentInventorySlot = m_vInventory.Count(); // Update inventory slot if invalid
	}

	m_iCameraState = CAMERA_PLACEMENT; // Set camera state
	SetThink(&CWeaponCamera::PlacementThink);

	// Play placement mode sound
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), CAMERA_PLACEMENT_SOUND);

	m_flNextScale = gpGlobals->curtime;
	SetNextThink(gpGlobals->curtime + 0.1f);
}


//-----------------------------------------------------------------------------
// Purpose: Handle placement mode
//-----------------------------------------------------------------------------
void CWeaponCamera::PlacementThink(void)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	Vector facingVector;
	AngleVectors(pOwner->EyeAngles(), &facingVector);

	trace_t tr;
	UTIL_TraceLine(pOwner->EyePosition(), pOwner->EyePosition() + (facingVector * MAX_TRACE_LENGTH), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	CCameraEntity camEntity = m_vInventory[m_iCurrentInventorySlot];
	CBaseAnimating* baseEntity = dynamic_cast<CBaseAnimating*>(camEntity.GetEntity());

	baseEntity->UpdateModelScale();
	float entityRadius = baseEntity->CollisionProp()->BoundingRadius2D();
	float entityHeight = baseEntity->CollisionProp()->OBBSize().z;

	// 6 trace placement method (like Exposure, thanks to TNX for recommending this method)
	Vector offset(0, 0, 0);
	trace_t fTr; // Face Trace

	// TODO: IMPROVE THIS ENTIRE PART

	// +X
	Vector faceEndCoords = Vector(MAX_TRACE_LENGTH, 0, 0);
	UTIL_TraceLine(tr.endpos, faceEndCoords, MASK_SOLID, baseEntity, COLLISION_GROUP_NONE, &fTr);
	if (abs(tr.endpos.x - fTr.endpos.x) < entityRadius) {
		offset.x -= entityRadius- abs(tr.endpos.x - fTr.endpos.x);
	}
	else {
		// -X
		faceEndCoords = Vector(-MAX_TRACE_LENGTH, 0, 0);
		UTIL_TraceLine(tr.endpos, faceEndCoords, MASK_SOLID, baseEntity, COLLISION_GROUP_NONE, &fTr);
		if (abs(tr.endpos.x - fTr.endpos.x) < entityRadius) {
			offset.x += entityRadius- abs(tr.endpos.x - fTr.endpos.x);
		}
	}

	// +Y
	faceEndCoords = Vector(0, MAX_TRACE_LENGTH, 0);
	UTIL_TraceLine(tr.endpos, faceEndCoords, MASK_SOLID, baseEntity, COLLISION_GROUP_NONE, &fTr);
	if (abs(tr.endpos.y - fTr.endpos.y) < entityRadius) {
		offset.y -= entityRadius- abs(tr.endpos.y - fTr.endpos.y);
	}
	else {
		// -Y
		faceEndCoords = Vector(0, -MAX_TRACE_LENGTH, 0);
		UTIL_TraceLine(tr.endpos, faceEndCoords, MASK_SOLID, baseEntity, COLLISION_GROUP_NONE, &fTr);
		if (abs(tr.endpos.y - fTr.endpos.y) < entityRadius) {
			offset.y += entityRadius- abs(tr.endpos.y - fTr.endpos.y);
		}
	}

	// +Z
	faceEndCoords = Vector(0, 0, MAX_TRACE_LENGTH);
	UTIL_TraceLine(tr.endpos, faceEndCoords, MASK_SOLID, baseEntity, COLLISION_GROUP_NONE, &fTr);
	if (abs(tr.endpos.z - fTr.endpos.z) < entityHeight) {
		offset.z -= entityHeight-abs(tr.endpos.z - fTr.endpos.z);
	}
	else {
		// -Z
		//faceEndCoords = Vector(0, 0, -MAX_TRACE_LENGTH);
		//UTIL_TraceLine(tr.endpos, faceEndCoords, MASK_SOLID, baseEntity, COLLISION_GROUP_NONE, &fTr);
		//if (abs(tr.endpos.z - fTr.endpos.z) < entityHeight) {
		//	offset.z += entityHeight-abs(tr.endpos.z - fTr.endpos.z);
		//}
	}

	//baseEntity->SetLocalAngles(baseEntity->GetLocalAngles() + QAngle(0, 2, 0));
	baseEntity->SetAbsOrigin(tr.endpos + offset);

	SetNextThink(gpGlobals->curtime);// +0.1f);
	camEntity.ShowEntity();
}


//-----------------------------------------------------------------------------
// Purpose: Set player FOV and apply HUD
//-----------------------------------------------------------------------------
void CWeaponCamera::SetZoom(bool zoom)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;
	
	CSingleUserRecipientFilter filter(pOwner);
	if (zoom) {
		pOwner->SetFOV(this, 30, 0.2f);
		UserMessageBegin(filter, "ShowCameraViewfinder");
		WRITE_BYTE(1);
		MessageEnd();

		// Play zoom sound
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), CAMERA_ZOOM_SOUND);
	}
	else {
		pOwner->SetFOV(this, 0, 0.1f);
		UserMessageBegin(filter, "ShowCameraViewfinder");
		WRITE_BYTE(0);
		MessageEnd();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Set slot and enter placement mode if not already in it
//-----------------------------------------------------------------------------
void CWeaponCamera::SetSlot(int slot)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (slot >= m_vInventory.Count() || slot < 0) {
		Msg("Invalid Slot, OOB");
		return;
	}

	m_iCurrentInventorySlot = slot;
	if (m_iCameraState != CAMERA_PLACEMENT) {
		SecondaryAttack();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Scale object in placement
//-----------------------------------------------------------------------------
void CWeaponCamera::SetScale(bool scaleUp)
{
	if (gpGlobals->curtime < m_flNextScale) {
		return; // Wait until next scale
	}

	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (m_iCameraState != CAMERA_PLACEMENT) {
		Msg("Cannot scale, not in placement mode");
		return;
	}

	Msg("Preparing to scale");
	CBaseAnimating* placementEntity = dynamic_cast<CBaseAnimating*>(m_vInventory[m_iCurrentInventorySlot].GetEntity());

	// Get current scale index
	int currentScaleIndex = 0;
	while (currentScaleIndex < CameraScalesLen) {
		if (CameraScales[currentScaleIndex] == placementEntity->GetModelScale()) {
			break;
		}
		currentScaleIndex++;
	}

	CPASAttenuationFilter filter(this);
	if (scaleUp && currentScaleIndex < CameraScalesLen-1) {
		currentScaleIndex++;
		EmitSound(filter, entindex(), CAMERA_SCALE_UP_SOUND);
	}
	else if (!scaleUp && currentScaleIndex > 0) {
		currentScaleIndex--;
		EmitSound(filter, entindex(), CAMERA_SCALE_DOWN_SOUND);
	}
	else {
		return;
	}

	Msg("Scaling object...");
	// Custom function in props.cpp to fix scale physics
	UTIL_CreateScaledCameraPhysObject(placementEntity, CameraScales[currentScaleIndex], CAMERA_SCALE_RATE);
	//placementEntity->SetModelScale(CameraScales[currentScaleIndex], 0);
	m_flNextScale = gpGlobals->curtime + CAMERA_SCALE_RATE;
}