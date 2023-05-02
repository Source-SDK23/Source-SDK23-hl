#ifndef PROPLASERRELAY_H
#define PROPLASERRELAY_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseentity.h"
#include "sdk23/sdk23_const.h"

class CPropLaserRelay : public CBaseAnimating {
	DECLARE_CLASS(CPropLaserRelay, CBaseAnimating)

public:
	CPropLaserRelay(void);

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
	bool m_bUseFilterColour;
	CNetworkColor32(m_clrFilterColour);
};

#endif // PROPLASERRELAY_H