//=========                BASED ON ENV_LASER CODE                 ============//
//
// Purpose: PORTAL. LASER.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "env_portal_beam.h"
#include "Sprite.h"
#include "IEffects.h"

#include <particle_system.h>
#include <prop_laser_catcher.h>
#include <saverestore_utlvector.h>
#include <prop_laser_relay.h>
#include <prop_weighted_cube.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(env_portal_beam, CEnvPortalBeam);

BEGIN_DATADESC(CEnvPortalBeam)

DEFINE_FIELD(m_bLaserState, FIELD_BOOLEAN),
DEFINE_FIELD(m_fNextSparkTime, FIELD_FLOAT),
DEFINE_FIELD(m_hLaserCatcher, FIELD_EHANDLE),
DEFINE_FIELD(m_hLaserCube, FIELD_EHANDLE),
//DEFINE_UTLVECTOR(m_vhLaserRelays, FIELD_EHANDLE),

// Keyfields
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "startstate"),
DEFINE_KEYFIELD(m_bInstaKill, FIELD_BOOLEAN, "lethaldamage"),

DEFINE_KEYFIELD(m_clrBeamColour, FIELD_COLOR32, "beamcolor"),
DEFINE_KEYFIELD(m_clrSpriteColour, FIELD_COLOR32, "spritecolor"),

DEFINE_KEYFIELD(m_bDisableCollision, FIELD_BOOLEAN, "DisablePlayerCollision"),

// Function Pointers
DEFINE_FUNCTION(BeamThink),

// Input functions
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOn", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOff", InputTurnOff),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

#ifdef MAPBASE
DEFINE_OUTPUT(m_OnTouchedByEntity, "OnTouchedByEntity"),
#endif

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::Spawn(void)
{
	SetSolid(SOLID_NONE);							// Remove model & collisions
	m_fNextSparkTime = gpGlobals->curtime;

	BeamInit("sprites/laser.spr", 2.0f);
	//m_spawnflags = SF_BEAM_SPARKEND;
	
	SetWidth(4.0f);
	SetEndWidth(GetWidth());				// Note: EndWidth is not scaled
	SetNoise(0);
	SetScrollRate(0);
	SetModelName(MAKE_STRING("sprites/laser.spr"));
	int m_spriteTexture = PrecacheModel(STRING(MAKE_STRING("sprites/laser.spr")));
	SetTexture(m_spriteTexture);
	SetColor(m_clrBeamColour->r, m_clrBeamColour->g, m_clrBeamColour->b);
	SetRenderMode(kRenderGlow);
	SetBrightness(255);

	PointsInit(GetLocalOrigin(), GetLocalOrigin());

	Precache();

	BaseClass::Spawn();

	SetNextThink(gpGlobals->curtime + 0.1f); // TICK!
	if (m_bStartDisabled) {
		TurnOff();
	} else {
		// Manually turn on BUT DO NOT RUN think at spawn or stuff breaks!!!
		// SetThink is used anyway because it waits for spawn to be complete
		m_bLaserState = true;
		RemoveEffects(EF_NODRAW);
		m_flFireTime = gpGlobals->curtime;
		SetThink(&CEnvPortalBeam::BeamThink);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::Precache(void)
{
	SetModelIndex(PrecacheModel(STRING(GetModelName())));
}


//-----------------------------------------------------------------------------
// Purpose: Returns whether the laser is currently active.
//-----------------------------------------------------------------------------
bool CEnvPortalBeam::GetState(void)
{
	return m_bLaserState;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::InputTurnOn(inputdata_t& inputdata)
{
	if (!GetState())
	{
		TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::InputTurnOff(inputdata_t& inputdata)
{
	if (GetState())
	{
		TurnOff();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::InputToggle(inputdata_t& inputdata)
{
	if (GetState())
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEnvPortalBeam::TurnOff(void)
{
	if (!GetState()) {
		return false;
	}
	m_bLaserState = false;

	AddEffects(EF_NODRAW);

	// Handle "children" of laser chain
	CPropLaserCatcher* oldLaserCatcher = dynamic_cast<CPropLaserCatcher*>(m_hLaserCatcher.Get());
	CPropWeightedCube* oldLaserCube = dynamic_cast<CPropWeightedCube*>(m_hLaserCube.Get());
	if (oldLaserCatcher != NULL) {
		oldLaserCatcher->Toggle(false, 0, 0, 0);
		m_hLaserCatcher = NULL;
	}
	if (oldLaserCube != NULL) {
		oldLaserCube->SendLaserState(false, 0, 0, 0, 0, 0, 0);
		m_hLaserCube = NULL;
	}

	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CEnvPortalBeam::TurnOn(void)
{
	if (GetState()) {
		Msg("BEAM IS ALREADY ONNN!!!");
		return false;
	}
	m_bLaserState = true;

	RemoveEffects(EF_NODRAW);

	m_flFireTime = gpGlobals->curtime;

	SetThink(&CEnvPortalBeam::BeamThink);

	//
	// Call BeamThink here to update the end position, otherwise we will see
	// the beam in the wrong place for one frame since we cleared the nodraw flag.
	//
	BeamThink();
	return true;
}

void CEnvPortalBeam::SetBeamColour(int r, int g, int b) {
	m_clrBeamColour.SetR(r);
	m_clrBeamColour.SetG(g);
	m_clrBeamColour.SetB(b);
	SetColor(r, g, b);
}

void CEnvPortalBeam::SetSpriteColour(int r, int g, int b) {
	m_clrSpriteColour.SetR(r);
	m_clrSpriteColour.SetG(g);
	m_clrSpriteColour.SetB(b);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::BeamThink(void)
{
	// Create Vector for direction
	Vector vecDir;
	// Take the Player's EyeAngles and turn it into a direction
	AngleVectors(GetAbsAngles(), &vecDir);

	trace_t tr; // Create our trace_t class to hold the end result
	// Do the TraceLine, and write our results to our trace_t class, tr.
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + (vecDir * MAX_TRACE_LENGTH), MASK_OPAQUE_AND_NPCS, GetParent(), COLLISION_GROUP_NONE, &tr); // Ignore parent
	
	//CollisionProp()->SetCollisionBounds(GetAbsOrigin(), tr.endpos); // Set collider
	//CollisionProp()->SetCollisionBounds

	SetAbsEndPos(tr.endpos);
	//DoSparks(GetAbsStartPos(), tr.endpos);

	bool sparksEnabled = true;

	// Handle hit logic
	if (FClassnameIs(tr.m_pEnt, "player")) {

		Vector playerDir;
		AngleVectors(tr.m_pEnt->GetAbsAngles(), &playerDir);

		tr.m_pEnt->SetAbsOrigin(tr.m_pEnt->GetAbsOrigin() - (playerDir * 5));
		tr.m_pEnt->SetAbsVelocity(tr.m_pEnt->GetAbsVelocity().Normalized() * -300); // -300 is a good constant, using -2*absvel causes more vertical yeeting
		tr.m_pEnt->TakeDamage(CTakeDamageInfo(this, this, 2.5f, DMG_BURN));
		SetNextThink(gpGlobals->curtime);
		return; // Do nothing else
	}

	// Handle laser catcher hit logic
	if (FClassnameIs(tr.m_pEnt, "prop_laser_catcher")) {
		CPropLaserCatcher* laserCatcher = dynamic_cast<CPropLaserCatcher*>(tr.m_pEnt);
		CPropLaserCatcher* oldLaserCatcher = dynamic_cast<CPropLaserCatcher*>(m_hLaserCatcher.Get());

		if (laserCatcher != oldLaserCatcher) {
			Msg("Catcher hit and it does not match old one");
			if (oldLaserCatcher != NULL) { // Deactivate old laser catcher
				Msg("Disabling old catcher");
				oldLaserCatcher->Toggle(false, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
				oldLaserCatcher = NULL;
			}

			if (laserCatcher->Toggle(true, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b)) {
				Msg("Setting catcher var");
				m_hLaserCatcher = laserCatcher; // Track new laser catcher
			}
		}

		sparksEnabled = false; // Don't do sparks
	} else if (m_hLaserCatcher != NULL) {
		Msg("Turning off catcher");
		CPropLaserCatcher* laserCatcher = dynamic_cast<CPropLaserCatcher*>(m_hLaserCatcher.Get());
		laserCatcher->Toggle(false, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
		m_hLaserCatcher = NULL;
	}

	if (FClassnameIs(tr.m_pEnt, "prop_weighted_cube")) {
		CPropWeightedCube* laserCube = dynamic_cast<CPropWeightedCube*>(tr.m_pEnt);
		CPropWeightedCube* oldLaserCube = dynamic_cast<CPropWeightedCube*>(m_hLaserCube.Get());

		Msg("Cube detected, checking");
		if (laserCube != oldLaserCube) {
			if (oldLaserCube != NULL) {
				oldLaserCube->SendLaserState(false, 0, 0, 0, 0, 0, 0);
				oldLaserCube = NULL;
			}

			Msg("Trying to turn on new cube");
			if (laserCube->SendLaserState(true, m_clrBeamColour->r, m_clrBeamColour->g, m_clrBeamColour->b, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b)) {
				m_hLaserCube = laserCube;
			}
		}
		else {
			Msg("Cube is the same");
		}

		sparksEnabled = false;
	} else if (m_hLaserCube != NULL) {
		CPropWeightedCube* oldLaserCube = dynamic_cast<CPropWeightedCube*>(m_hLaserCube.Get());
		oldLaserCube->SendLaserState(false, 0, 0, 0, 0, 0, 0);
		m_hLaserCube = NULL;
	}


	if (gpGlobals->curtime > m_fNextSparkTime && sparksEnabled) {
		//DoSparks(GetAbsStartPos(), tr.endpos);
		Vector vecDir;
		AngleVectors(GetAbsAngles() + QAngle(0, 180, 0), &vecDir); // Particle is flipped
		g_pEffects->Sparks(tr.endpos, 1, 2, &vecDir);
		m_fNextSparkTime = gpGlobals->curtime + 0.1f;
	}

	SetNextThink(gpGlobals->curtime);
}