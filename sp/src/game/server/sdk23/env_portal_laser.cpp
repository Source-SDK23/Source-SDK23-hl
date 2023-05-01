
#include "cbase.h"
#include "env_portal_laser.h"
#include <env_portal_beam.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#include <particle_system.h>


LINK_ENTITY_TO_CLASS(env_portal_laser, CEnvPortalLaser);

BEGIN_DATADESC(CEnvPortalLaser)

// Keyfields
DEFINE_KEYFIELD(m_bStartDisabled, FIELD_BOOLEAN, "startstate"),
DEFINE_KEYFIELD(m_bInstaKill, FIELD_BOOLEAN, "lethaldamage"),
DEFINE_KEYFIELD(m_bAutoAimBeam, FIELD_BOOLEAN, "autoaimenabled"),
DEFINE_KEYFIELD(m_iSkintype, FIELD_INTEGER, "skin"),

DEFINE_KEYFIELD(m_clrBeamColour, FIELD_COLOR32, "beamcolor"),
DEFINE_KEYFIELD(m_clrSpriteColour, FIELD_COLOR32, "spritecolor"),

DEFINE_KEYFIELD(m_bDisableCollision, FIELD_BOOLEAN, "DisablePlayerCollision"),

// Fields
DEFINE_FIELD(m_hBeam, FIELD_EHANDLE),
DEFINE_FIELD(m_hParticles, FIELD_EHANDLE),

// Input functions
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOn", InputTurnOn),
DEFINE_INPUTFUNC(FIELD_VOID, "TurnOff", InputTurnOff),
DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

END_DATADESC()



void CEnvPortalLaser::Spawn(void) {
	Precache();


	// Beam
	CEnvPortalBeam* beam = dynamic_cast<CEnvPortalBeam*>(CreateEntityByName("env_portal_beam"));
	beam->KeyValue("startstate", m_bStartDisabled);
	beam->KeyValue("lethaldamage", m_bInstaKill);
	beam->KeyValue("autoaimenabled", m_bAutoAimBeam);
	beam->KeyValue("DisablePlayerCollision", m_bDisableCollision);

	SetModel(GetModelName().ToCStr());
	SetSolid(SOLID_VPHYSICS);

	int laserAttachment = LookupAttachment("laser_attachment");
	Vector attachLoc;
	QAngle attachAng;
	GetAttachment(laserAttachment, attachLoc, attachAng);
	beam->SetAbsAngles(attachAng);
	beam->SetAbsOrigin(attachLoc);

	beam->SetBeamColour(m_clrBeamColour->r, m_clrBeamColour->g, m_clrBeamColour->b);
	beam->SetSpriteColour(m_clrSpriteColour->r, m_clrSpriteColour->g, m_clrSpriteColour->b);
	DispatchSpawn(beam);
	beam->Activate();

	m_hBeam = beam;

	
	// Particle
	CParticleSystem* particle = dynamic_cast<CParticleSystem*>(CreateEntityByName("info_particle_system"));
	particle->KeyValue("effect_name", "reflector_start_glow");
	particle->KeyValue("start_active", 1);
	particle->SetAbsOrigin(attachLoc);
	particle->SetAbsAngles(attachAng);
	DispatchSpawn(particle);
	particle->Activate();

	m_hParticles = particle;
}

void CEnvPortalLaser::Precache(void) {
	const char* pModelName = STRING(GetModelName());
	if (pModelName) {
		PrecacheModel(pModelName);
	}
}

void CEnvPortalLaser::InputTurnOn(inputdata_t& inputdata) {
	TurnOn();
}
void CEnvPortalLaser::InputTurnOff(inputdata_t& inputdata) {
	TurnOff();
}
void CEnvPortalLaser::InputToggle(inputdata_t& inputdata) {
	CEnvPortalBeam* beam = dynamic_cast<CEnvPortalBeam*>(m_hBeam.Get());

	if (beam->GetState()) {
		TurnOff();
	} else {
		TurnOn();
	}
}

bool CEnvPortalLaser::TurnOn(void) {
	CEnvPortalBeam* beam = dynamic_cast<CEnvPortalBeam*>(m_hBeam.Get());
	return beam->TurnOn();
}
bool CEnvPortalLaser::TurnOff(void) {
	CEnvPortalBeam* beam = dynamic_cast<CEnvPortalBeam*>(m_hBeam.Get());
	return beam->TurnOff();
}
