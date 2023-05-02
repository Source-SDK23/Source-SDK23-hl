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

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <particle_system.h>
#include <prop_laser_catcher.h>
#include <saverestore_utlvector.h>
#include <prop_laser_relay.h>

LINK_ENTITY_TO_CLASS(env_portal_beam, CEnvPortalBeam);

BEGIN_DATADESC(CEnvPortalBeam)

DEFINE_FIELD(m_fNextSparkTime, FIELD_FLOAT),
DEFINE_FIELD(m_hLaserCatcher, FIELD_EHANDLE),
DEFINE_UTLVECTOR(m_vhLaserRelays, FIELD_EHANDLE),

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
	SetThink(&CEnvPortalBeam::BeamThink);
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

	if (m_bStartDisabled) {
		TurnOff();
	} else {
		TurnOn();
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
	return IsEffectActive(EF_NODRAW);
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

	AddEffects(EF_NODRAW);

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
		return false;
	}

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
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + (vecDir * MAX_TRACE_LENGTH), MASK_OPAQUE_AND_NPCS, this, COLLISION_GROUP_NONE, &tr);
	
	SetAbsEndPos(tr.endpos);
	//DoSparks(GetAbsStartPos(), tr.endpos);

	bool sparksEnabled = true;

	// "Handle" laser relay logic...
	CUtlVector<EHANDLE> hitRelays;
	while (FClassnameIs(tr.m_pEnt, "prop_laser_relay")) {
		sparksEnabled = false;

		CPropLaserRelay* laserRelay = dynamic_cast<CPropLaserRelay*>(tr.m_pEnt);
		
		hitRelays.AddToTail(laserRelay); // Add relay to list of hit relays

		// If laser relay has not been hit before
		if (!m_vhLaserRelays.HasElement(laserRelay)) {
			laserRelay->Toggle(true, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b); // Turn on relay
		}

		// New trace ignoring the relay we just hit
		UTIL_TraceLine(laserRelay->GetAbsOrigin(), laserRelay->GetAbsOrigin() + (vecDir * MAX_TRACE_LENGTH), MASK_OPAQUE_AND_NPCS, laserRelay, COLLISION_GROUP_NONE, &tr);
	}

	// Disable all laser relays we are not hitting anymore
	for (int i = 0; i < m_vhLaserRelays.Count(); i++) {
		if (hitRelays.HasElement(m_vhLaserRelays[i])) {
			continue;
		}

		CPropLaserRelay* oldLaserRelay = dynamic_cast<CPropLaserRelay*>(m_vhLaserRelays[i].Get());
		oldLaserRelay->Toggle(false, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
	}
	m_vhLaserRelays = hitRelays; // Replace new vector list

	// Handle laser hit logic
	if (FClassnameIs(tr.m_pEnt, "prop_laser_catcher")) {
		CPropLaserCatcher* laserCatcher = dynamic_cast<CPropLaserCatcher*>(tr.m_pEnt);
		CPropLaserCatcher* oldLaserCatcher = dynamic_cast<CPropLaserCatcher*>(m_hLaserCatcher.Get());

		if (laserCatcher != oldLaserCatcher) {
			if (oldLaserCatcher != NULL) { // Deactivate old laser catcher
				oldLaserCatcher->Toggle(false, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
			}

			if (laserCatcher->Toggle(true, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b)) {
				m_hLaserCatcher = laserCatcher; // Track new laser catcher
			}
		}

		sparksEnabled = false; // Don't do sparks
	}
	else if (m_hLaserCatcher != NULL) {
		CPropLaserCatcher* laserCatcher = dynamic_cast<CPropLaserCatcher*>(m_hLaserCatcher.Get());
		laserCatcher->Toggle(false, m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
		m_hLaserCatcher = NULL;
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


