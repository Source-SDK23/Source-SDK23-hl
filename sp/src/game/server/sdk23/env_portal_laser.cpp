//=========                BASED ON ENV_LASER CODE                 ============//
//
// Purpose: PORTAL. LASER.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "env_portal_laser.h"
#include "Sprite.h"
#include "IEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <particle_system.h>

LINK_ENTITY_TO_CLASS(env_portal_laser, CEnvPortalLaser);

BEGIN_DATADESC(CEnvPortalLaser)

DEFINE_FIELD(m_fNextSparkTime, FIELD_FLOAT),

// Keyfields
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "startstate"),
DEFINE_KEYFIELD(m_bDoDamage, FIELD_BOOLEAN, "lethaldamage"),

// Function Pointers
DEFINE_FUNCTION(LaserThink),

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
void CEnvPortalLaser::Spawn(void)
{
	SetSolid(SOLID_NONE);							// Remove model & collisions
	SetThink(&CEnvPortalLaser::LaserThink);
	m_fNextSparkTime = gpGlobals->curtime;

	BeamInit("sprites/laser.spr", 2.0f);
	//m_spawnflags = SF_BEAM_SPARKEND;
	
	SetWidth(2.0f);
	SetEndWidth(GetWidth());				// Note: EndWidth is not scaled
	SetNoise(0);
	SetScrollRate(0);
	SetModelName(MAKE_STRING("sprites/laser.spr"));
	int m_spriteTexture = PrecacheModel(STRING(MAKE_STRING("sprites/laser.spr")));
	SetTexture(m_spriteTexture);

	PointsInit(GetLocalOrigin(), GetLocalOrigin());

	Precache();

	if (GetEntityName() != NULL_STRING && !(m_spawnflags & SF_BEAM_STARTON))
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
void CEnvPortalLaser::Precache(void)
{
	SetModelIndex(PrecacheModel(STRING(GetModelName())));
}


//-----------------------------------------------------------------------------
// Purpose: Returns whether the laser is currently active.
//-----------------------------------------------------------------------------
int CEnvPortalLaser::IsOn(void)
{
	if (IsEffectActive(EF_NODRAW))
		return 0;
	return 1;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalLaser::InputTurnOn(inputdata_t& inputdata)
{
	if (!IsOn())
	{
		TurnOn();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalLaser::InputTurnOff(inputdata_t& inputdata)
{
	if (IsOn())
	{
		TurnOff();
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalLaser::InputToggle(inputdata_t& inputdata)
{
	if (IsOn())
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
void CEnvPortalLaser::TurnOff(void)
{
	AddEffects(EF_NODRAW);

	SetNextThink(TICK_NEVER_THINK);
	SetThink(NULL);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalLaser::TurnOn(void)
{
	RemoveEffects(EF_NODRAW);

	m_flFireTime = gpGlobals->curtime;

	SetThink(&CEnvPortalLaser::LaserThink);

	//
	// Call LaserThink here to update the end position, otherwise we will see
	// the beam in the wrong place for one frame since we cleared the nodraw flag.
	//
	LaserThink();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvPortalLaser::LaserThink(void)
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


