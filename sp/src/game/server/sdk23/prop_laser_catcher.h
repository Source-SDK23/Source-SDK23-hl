#ifndef PROPLASERCATCHER_H
#define PROPLASERCATCHER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "props.h"
#include "baseentity.h"
#include "sdk23/sdk23_const.h"

class CPropLaserCatcher : public CDynamicProp {
	DECLARE_CLASS(CPropLaserCatcher, CDynamicProp)

public:
	CPropLaserCatcher(void);

	void Spawn(void);
	void Precache(void);

	bool Toggle(bool state, int pR, int pG, int pB);

	bool GetState(void) { return m_bState; };

	void ISetSkin(inputdata_t& inputdata);
	void ISetFilterColor(inputdata_t& inputdata);

	COutputEvent	m_OnPowered;
	COutputEvent	m_OnUnpowered;
	DECLARE_DATADESC()
private:
	EHANDLE m_hParticles;

	bool m_bState;
	RustedSkinType m_iSkintype;
	bool m_bUseFilterColour;
	CNetworkColor32(m_clrFilterColour);
};

#endif // PROPLASERCATCHER_H