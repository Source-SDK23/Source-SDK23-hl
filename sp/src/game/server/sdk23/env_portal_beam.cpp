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

LINK_ENTITY_TO_CLASS(env_portal_beam, CEnvPortalBeam);

BEGIN_DATADESC(CEnvPortalBeam)

DEFINE_FIELD(m_fNextSparkTime, FIELD_FLOAT),

// Keyfields
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "startstate"),
DEFINE_KEYFIELD(m_bDoDamage, FIELD_BOOLEAN, "lethaldamage"),
DEFINE_KEYFIELD(m_bAutoAimBeam, FIELD_BOOLEAN, "autoaimenabled"),
DEFINE_KEYFIELD(m_bRustedSkin, FIELD_INTEGER, "skin"),

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
void CEnvPortalBeam::TurnOff(void)
{
	AddEffects(EF_NODRAW);

	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalBeam::TurnOn(void)
{
	RemoveEffects(EF_NODRAW);

	m_flFireTime = gpGlobals->curtime;

	SetThink(&CEnvPortalBeam::BeamThink);

	//
	// Call BeamThink here to update the end position, otherwise we will see
	// the beam in the wrong place for one frame since we cleared the nodraw flag.
	//
	BeamThink();
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

	if (gpGlobals->curtime > m_fNextSparkTime) {
		//DoSparks(GetAbsStartPos(), tr.endpos);
		Vector vecDir;
		AngleVectors(GetAbsAngles() + QAngle(0, 180, 0), &vecDir); // Particle is flipped
		g_pEffects->Sparks(tr.endpos, 1, 2, &vecDir);
		m_fNextSparkTime = gpGlobals->curtime + 0.1f;
	}

	SetNextThink(gpGlobals->curtime);
}


