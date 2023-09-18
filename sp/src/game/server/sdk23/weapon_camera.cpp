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
#include "camera_placement.h"
#include "weapon_camera.h"
#include "physics_saverestore.h"

#include <saverestore_utlvector.h>
#include <in_buttons.h>
#include <props.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


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

	//baseEntity->SetSolid(SOLID_NONE);
	//baseEntity->SetMoveType(MOVETYPE_NONE);

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
	//baseEntity->VPhysicsGetObject()->SetPosition(baseEntity->GetAbsOrigin(), baseEntity->GetAbsAngles(), true); // Update collider
}


//-----------------------------------------------------------------------------
// Purpose: Hides entity preview
//-----------------------------------------------------------------------------
void CCameraEntity::HideEntity(void) {
	CBaseEntity* baseEntity = m_hEntity.Get();

	//baseEntity->SetAbsOrigin(Vector(3000, 3000, 3000));
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

	DEFINE_EMBEDDED(m_placementController),

	// Physptrs can't be inside embedded classes
	DEFINE_PHYSPTR(m_placementController.m_controller),
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

	// If camera is in normal state, enter zoom mode
	case CAMERA_NORMAL:
		SetZoom(true); // Zoom in
		m_iCameraState = CAMERA_ZOOM; // Zoom
		break;


	// If camera is in zoom mode, capture object
	case CAMERA_ZOOM:
		// Unzoom
		SetZoom(false);
		m_iCameraState = CAMERA_NORMAL; // Reset camera state to normal

		if (m_vInventory.Count() == CAMERA_MAX_INVENTORY) {
			Msg("CANNOT CAPTURE! INVENTORY IS FULL");
			return;
		}

		// Perform traceline to find object to capture
		AngleVectors(pOwner->EyeAngles(), &facingVector);
		UTIL_TraceLine(pOwner->EyePosition(), pOwner->EyePosition() + (facingVector * MAX_TRACE_LENGTH), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

		if (tr.m_pEnt) {
			for (int i = 0; i < CaptureBlacklistLen; i++) { // Do not capture blacklisted names
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


	// If in placement mode, place the object
	case CAMERA_PLACEMENT:
		CCameraEntity camEntity = m_vInventory[m_iCurrentInventorySlot];
		DeInitPlacementController(false);				// Remove the placement controller
		camEntity.RestoreEntity();						// Restore the entity
		m_vInventory.Remove(m_iCurrentInventorySlot);	// Remove the entity from inventory
		m_iCameraState = CAMERA_NORMAL;					// Reset camera state

		m_iCurrentInventorySlot = max(m_vInventory.Count() - 1, 0); // Set inventory slot to either 0 or last item
		EmitSound(filter, entindex(), CAMERA_PLACE_SOUND);			// Make placement sound
		break;
	}
}




//-----------------------------------------------------------------------------
// Purpose: Enters placement mode whilst exiting other modes
//-----------------------------------------------------------------------------
void CWeaponCamera::SecondaryAttack(void)
{
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.1f;
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	
	if (m_iCameraState == CAMERA_ZOOM) {				// Exit zoom mode
		SetZoom(false);
		m_iCameraState = CAMERA_NORMAL;
	} else if (m_iCameraState == CAMERA_PLACEMENT) {	// Exit placement mode if already in it
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
	if (m_iCurrentInventorySlot == m_vInventory.Count()) {	// It is imposible for this to actually be greater so >= not needed
		m_iCurrentInventorySlot = m_vInventory.Count();	// Update inventory slot if invalid
	}

	m_iCameraState = CAMERA_PLACEMENT; // Set camera state

	// Init placement controller
	InitPlacementController(true);
	

	// Play placement mode sound
	CPASAttenuationFilter filter(this);
	EmitSound(filter, entindex(), CAMERA_PLACEMENT_SOUND);

	m_flNextScale = gpGlobals->curtime;
}




//-----------------------------------------------------------------------------
// Purpose: Handle entering placement mode
//-----------------------------------------------------------------------------
void CWeaponCamera::InitPlacementController(bool showEntity) {
	// Get the current inventory slot and coresponding entity
	CCameraEntity camEntity = m_vInventory[m_iCurrentInventorySlot];
	CBaseEntity* baseEntity = dynamic_cast<CBaseEntity*>(camEntity.GetEntity()); // Get the actual captured entity

	m_placementController.DetachEntity(true); // Detatch current entity from controller

	// Show object preview
	if (showEntity) {
		camEntity.ShowEntity();
	}

	m_placementController.SetIgnorePitch(false);	// TODO: Fix placement angles
	m_placementController.SetAngleAlignment(0);		// TODO: Fix placement angles
	m_placementController.AttachEntity(ToBasePlayer(GetOwner()), baseEntity, baseEntity->VPhysicsGetObject(), false, vec3_origin, false);
	SetThink(&CWeaponCamera::PlacementThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
}




//-----------------------------------------------------------------------------
// Purpose: Handle exiting placement mode
//-----------------------------------------------------------------------------
void CWeaponCamera::DeInitPlacementController(bool hideEntity) {
	m_placementController.DetachEntity(true); // Detatch current entity from controller

	// Show object preview
	if (hideEntity) {
		CCameraEntity camEntity = m_vInventory[m_iCurrentInventorySlot];	// Get the current inventory slot
		camEntity.HideEntity();
	}
	SetThink(NULL);
}




//-----------------------------------------------------------------------------
// Purpose: Handle placement mode logic via the controller
//-----------------------------------------------------------------------------
void CWeaponCamera::PlacementThink(void) {
	m_placementController.UpdateObject(ToBasePlayer(GetOwner()));
	SetNextThink(gpGlobals->curtime + 0.1f);
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
		pOwner->SetFOV(this, 30, 0.2f);						// Actually zoom
		UserMessageBegin(filter, "ShowCameraViewfinder");	// Show the clientside camera HUD
		WRITE_BYTE(1);
		MessageEnd();

		// Play zoom sound
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), CAMERA_ZOOM_SOUND);
	}
	else {
		pOwner->SetFOV(this, 0, 0.1f);						// Reset zoom
		UserMessageBegin(filter, "ShowCameraViewfinder");	// Hide the clientside camera HUD
		WRITE_BYTE(0);
		MessageEnd();
	}
}




//-----------------------------------------------------------------------------
// Purpose: Set slot and enter placement mode if not already in it (called when number keys are pressed)
//-----------------------------------------------------------------------------
void CWeaponCamera::SetSlot(int slot)
{
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if (slot >= m_vInventory.Count() || slot < 0) {	// Check if the camera slot can be used
		Msg("Invalid Slot, OOB");
		return;
	}

	if (m_iCameraState == CAMERA_PLACEMENT) {
		DeInitPlacementController(true);				// Detatch the current object and hide it (if possible)
	}
	
	m_iCurrentInventorySlot = slot;	
	InitPlacementController(true);
	m_iCameraState = CAMERA_PLACEMENT;	// Enter placement mode
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
	CBaseAnimating* baseEntity = dynamic_cast<CBaseAnimating*>(m_vInventory[m_iCurrentInventorySlot].GetEntity());

	// Get current scale index
	int currentScaleIndex = 0;
	while (currentScaleIndex < CameraScalesLen) {
		if (CameraScales[currentScaleIndex] == baseEntity->GetModelScale()) {
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
	// Detach entity (but do not hide)
	DeInitPlacementController(false);

	// Custom function in props.cpp to fix scale physics
	UTIL_CreateScaledCameraPhysObject(baseEntity, CameraScales[currentScaleIndex], CAMERA_SCALE_RATE);

	// Re-initialise placement controller (because the vphysics object changed)
	InitPlacementController(false);

	m_flNextScale = gpGlobals->curtime + CAMERA_SCALE_RATE;
}