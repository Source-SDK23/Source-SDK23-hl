//-----------------------------------------------------------------------------
// Purpose: Laser catcher entity
//-----------------------------------------------------------------------------

#include "cbase.h"
#include "prop_laser_catcher.h"
#include <particle_system.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


LINK_ENTITY_TO_CLASS(prop_laser_catcher, CPropLaserCatcher);

BEGIN_DATADESC(CPropLaserCatcher)

// Keyfields
DEFINE_KEYFIELD(m_iSkintype, FIELD_INTEGER, "skintype"),

DEFINE_KEYFIELD(m_clrFilterColour, FIELD_COLOR32, "filtercolor"),
DEFINE_KEYFIELD(m_bUseFilterColour, FIELD_BOOLEAN, "uselaserfilter"),

// Fields
DEFINE_FIELD(m_hParticles, FIELD_EHANDLE),
DEFINE_FIELD(m_bState, FIELD_BOOLEAN),

// Input functions
//DEFINE_INPUTFUNC(FIELD_VOID, "TurnOn", InputTurnOn),
//DEFINE_INPUTFUNC(FIELD_VOID, "TurnOff", InputTurnOff),
//DEFINE_INPUTFUNC(FIELD_VOID, "Toggle", InputToggle),

END_DATADESC()

CPropLaserCatcher::CPropLaserCatcher(void) {
	UseClientSideAnimation();
}


void CPropLaserCatcher::Spawn(void) {
	Precache();
	SetPlaybackRate(1);

	SetModel(GetModelName().ToCStr());

	// If rusted skin, set it accordigly
	if (m_iSkintype == SKIN_RUSTED) {
		SetSkin(2);
	}


	SetSolid(SOLID_VPHYSICS);
	
	// Colour the catcher accordingly
	if (m_bUseFilterColour) {
		SetRenderColor(m_clrFilterColour->r, m_clrFilterColour->g, m_clrFilterColour->b);
	}

	// Create particles
	CParticleSystem* particles = dynamic_cast<CParticleSystem*>(CreateEntityByName("info_particle_system"));
	particles->KeyValue("effect_name", "laser_relay_powered");
	particles->KeyValue("start_active", "0");

	Vector loc; // Move particle to entity
	QAngle ang;
	GetAttachment(LookupAttachment("particle_emitter"), loc, ang);
	particles->SetAbsAngles(ang);
	particles->SetAbsOrigin(loc);
	particles->SetParent(this);

	DispatchSpawn(particles);
	particles->Activate();

	
	m_hParticles = particles;
	BaseClass::Spawn();
}

void CPropLaserCatcher::Precache(void) {
	const char* pModelName = STRING(GetModelName());
	if (pModelName) {
		PrecacheModel(pModelName);
	}

	BaseClass::Precache();
}

// Change laser catcher state
bool CPropLaserCatcher::Toggle(bool state, int pR, int pG, int pB) {
	if (m_bState == state) {
		Msg("Catcher state already set");
		return false;
	}

	if (m_bUseFilterColour) {
		if (m_clrFilterColour->r != pR || m_clrFilterColour->g != pG || m_clrFilterColour->b != pB) {
			return false;
		}
	}

	CParticleSystem* particle = dynamic_cast<CParticleSystem*>(m_hParticles.Get());
	if (state) {
		if (m_iSkintype == SKIN_RUSTED) {
			SetSkin(3);
		}
		else {
			SetSkin(1);
		}

		SetSequence(LookupSequence("spin"));
		particle->StartParticleSystem();
	}
	else {
		if (m_iSkintype == SKIN_RUSTED) {
			SetSkin(2);
		}
		else {
			SetSkin(0);
		}

		SetSequence(LookupSequence("idle"));
		particle->StopParticleSystem();
	}

	m_bState = state;
	return true;
}