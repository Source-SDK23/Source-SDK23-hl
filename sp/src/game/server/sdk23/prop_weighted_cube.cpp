#include "cbase.h"

#include "prop_weighted_cube.h"
#include <env_portal_beam.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(prop_weighted_cube, CPropWeightedCube);
BEGIN_DATADESC(CPropWeightedCube)

DEFINE_FIELD(m_iIdleSkin, FIELD_INTEGER),
DEFINE_FIELD(m_iActiveSkin, FIELD_INTEGER),
DEFINE_FIELD(m_hBeam, FIELD_EHANDLE),
DEFINE_FIELD(m_hParticle, FIELD_EHANDLE),

DEFINE_KEYFIELD(m_iCubeType, FIELD_INTEGER, "cubetype"),
DEFINE_KEYFIELD(m_bUseLaserModifier, FIELD_BOOLEAN, "uselasermodifier"),
DEFINE_KEYFIELD(m_bUseLaserFilter, FIELD_BOOLEAN, "uselaserfilter"),

END_DATADESC()

void CPropWeightedCube::Spawn(void) {
	switch (m_iCubeType) {
	case CUBE_WEIGHTED_STORAGE:
		SetModelName(MAKE_STRING("models/props/metal_box.mdl"));
		if (m_iSkinType == SKIN_CLEAN) {
			m_iIdleSkin = 0;
			m_iActiveSkin = 2;
		}
		else {
			m_iIdleSkin = 3;
			m_iActiveSkin = 5;
		}
		break;
	case CUBE_WEIGHTED_COMPANION:
		SetModelName(MAKE_STRING("models/props/metal_box.mdl"));
		m_iIdleSkin = 1;
		m_iActiveSkin = 4;
		break;
	case CUBE_DISCOURAGEMENT_REDIRECTION:
		SetModelName(MAKE_STRING("models/props/reflection_cube.mdl"));
		if (m_iSkinType == SKIN_CLEAN) {
			m_iIdleSkin = 0;
			m_iActiveSkin = 0;
		}
		else {
			m_iIdleSkin = 1;
			m_iActiveSkin = 1;
		}
		break;
	case CUBE_SPHERE_CUBE:
		SetModelName(MAKE_STRING("models/npcs/personality_sphere/personality_sphere.mdl"));
		m_iIdleSkin = 0;
		m_iActiveSkin = 0;
		break;
	case CUBE_ANTIQUE:
		SetModelName(MAKE_STRING("models/props_underground_underground_weighted_cube.mdl"));
		m_iIdleSkin = 0;
		m_iActiveSkin = 0;
		break;
	case CUBE_QUANTUM:
		SetModelName(MAKE_STRING("models/props/reflection_cube.mdl"));
		m_iIdleSkin = 0;
		m_iActiveSkin = 0;
		break;
	}

	Precache();

	SetModel(GetModelName().ToCStr());
	SetSolid(SOLID_VPHYSICS);

	if (m_iCubeType == CUBE_DISCOURAGEMENT_REDIRECTION || m_iCubeType == CUBE_QUANTUM) {
		CEnvPortalBeam* beam = dynamic_cast<CEnvPortalBeam*>(CreateEntityByName("env_portal_beam"));
		beam->KeyValue("startstate", "1");
		beam->KeyValue("lethaldamage", "0");
		beam->KeyValue("autoaimenabled", "0");
		beam->KeyValue("DisablePlayerCollision", "0");

		Vector attachLoc;
		QAngle attachAng;
		GetAttachment("laser_attachment", attachLoc, attachAng);
		beam->SetAbsAngles(attachAng);
		beam->SetAbsOrigin(attachLoc);
		beam->SetParent(this);

		//beam->SetBeamColour(m_clrBeamColour->r, m_clrBeamColour->g, m_clrBeamColour->b);
		//beam->SetSpriteColour(m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
		DispatchSpawn(beam);
		beam->Activate();
		m_hBeam = beam;

		// Particle
		CParticleSystem* particle = dynamic_cast<CParticleSystem*>(CreateEntityByName("info_particle_system"));
		particle->KeyValue("effect_name", "reflector_start_glow");
		particle->KeyValue("start_active", "0");
		particle->SetAbsOrigin(attachLoc);
		particle->SetAbsAngles(attachAng);
		particle->SetParent(this);
		DispatchSpawn(particle);
		particle->Activate();

		m_hParticle = particle;
	}

	BaseClass::Spawn();
	Activate();
}

void CPropWeightedCube::Precache(void) {
	if (STRING(GetModelName())) {
		PrecacheModel(STRING(GetModelName()));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Send laser state to the matching cube
//-----------------------------------------------------------------------------
bool CPropWeightedCube::SendLaserState(bool state, int bR, int bB, int bG, int sR, int sB, int sG) {
	if (m_iCubeType == CUBE_DISCOURAGEMENT_REDIRECTION) {
		return RecieveLaserState(state, bR, bB, bG, sR, sB, sG); // Normal redirection cube
	} else if (m_iCubeType == CUBE_QUANTUM) {
		// Find matching cube
		CPropWeightedCube* matchingCube = dynamic_cast<CPropWeightedCube*>(gEntList.FindEntityByName(this, GetEntityName()));
		if (matchingCube == NULL) {
			return false;
		}
		return matchingCube->RecieveLaserState(state, bR, bB, bG, sR, sB, sG); // Send to quantum cube
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Recieve laser state and reflect it in state etc
//-----------------------------------------------------------------------------
bool CPropWeightedCube::RecieveLaserState(bool state, int bR, int bB, int bG, int sR, int sB, int sG) {
	if (m_iCubeType != CUBE_DISCOURAGEMENT_REDIRECTION && m_iCubeType != CUBE_QUANTUM) { return false; }

	CEnvPortalBeam* beam = dynamic_cast<CEnvPortalBeam*>(m_hBeam.Get());
	if (beam == NULL) {
		Msg("BEAM IS NULL ERRRR");
		return false;
	}
	if (beam->GetState() == state) {
		Msg("State matches");
		return false;
	}

	CParticleSystem* particleSystem = dynamic_cast<CParticleSystem*>(m_hParticle.Get());
	if (particleSystem == NULL) {
		return false;
	}
	if (state) {
		Msg("Turning stuff ON");
		//beam->SetColor(bR, bG, bB);
		//beam->SetSpriteColour(sR, sG, sB);
		beam->TurnOn();
		particleSystem->StartParticleSystem();
	}
	else {
		beam->TurnOff();
		particleSystem->StopParticleSystem();
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Stolen from turret for preferred angles
// Output : const QAngle
//-----------------------------------------------------------------------------
QAngle CPropWeightedCube::PreferredCarryAngles(void)
{
	// FIXME: Embed this into the class
	static QAngle g_prefAngles;

	Vector vecUserForward;
	CBasePlayer* pPlayer = AI_GetSinglePlayer();
	pPlayer->EyeVectors(&vecUserForward);

	// If we're looking up, then face directly forward
	if (vecUserForward.z >= 0.0f)
		return vec3_angle; // 0,0,0

	// Otherwise, stay "upright"
	g_prefAngles.Init();
	g_prefAngles.x = -pPlayer->EyeAngles().x; // ???

	return g_prefAngles;
}