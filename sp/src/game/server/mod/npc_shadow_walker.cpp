//========= Copyright Valve Corporation, All rights reserved. ============//
// This is a skeleton file for use when creating a new 
// NPC. Copy and rename this file for the new
// NPC and add the copy to the build.
//
// Leave this file in the build until we ship! Allowing 
// this file to be rebuilt with the rest of the game ensures
// that it stays up to date with the rest of the NPC code.
//
// Replace occurances of CNPC_ShadowWalker with the new NPC's
// classname. Don't forget the lower-case occurance in 
// LINK_ENTITY_TO_CLASS()
//
//
// ASSUMPTIONS MADE:
//
// You're making a character based on CAI_BaseNPC. If this 
// is not true, make sure you replace all occurances
// of 'CAI_BaseNPC' in this file with the appropriate 
// parent class.
//
// You're making a human-sized NPC that walks.
//
//=============================================================================//
#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "basehlcombatweapon_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// Private activities
//=========================================================
//int	ACT_MYCUSTOMACTIVITY = -1;

//=========================================================
// Custom schedules
//=========================================================
//enum
//{
//	SCHED_MYCUSTOMSCHEDULE = LAST_SHARED_SCHEDULE,
//};

//=========================================================
// Custom tasks
//=========================================================
//enum 
//{
//	TASK_MYCUSTOMTASK = LAST_SHARED_TASK,
//};


//=========================================================
// Custom Conditions
//=========================================================
//enum 
//{
//	COND_MYCUSTOMCONDITION = LAST_SHARED_CONDITION,
//};


//=========================================================
//=========================================================
class CNPC_ShadowWalker : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_ShadowWalker, CAI_BaseNPC );

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
	int				SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);
	int 			SelectScheduleRetrieveItem();
	int 			SelectSchedule();
	int				SelectIdleSchedule();
	int				SelectAlertSchedule();
	int				SelectCombatSchedule();
	DECLARE_DATADESC();

	// This is a dummy field. In order to provide save/restore
	// code in this file, we must have at least one field
	// for the code to operate on. Delete this field when
	// you are ready to do your own save/restore for this
	// character.
	bool		m_bHasWeapon;

	DEFINE_CUSTOM_AI;
};


LINK_ENTITY_TO_CLASS( npc_shadow_walker, CNPC_ShadowWalker );
IMPLEMENT_CUSTOM_AI( npc_citizen,CNPC_ShadowWalker );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_ShadowWalker )

	DEFINE_FIELD(m_bHasWeapon, FIELD_BOOLEAN),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Initialize the custom schedules
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InitCustomSchedules(void) 
{
	INIT_CUSTOM_AI(CNPC_ShadowWalker);

	//ADD_CUSTOM_TASK(CNPC_ShadowWalker,		TASK_MYCUSTOMTASK);

	//ADD_CUSTOM_SCHEDULE(CNPC_ShadowWalker,	SCHED_MYCUSTOMSCHEDULE);

	//ADD_CUSTOM_ACTIVITY(CNPC_ShadowWalker,	ACT_MYCUSTOMACTIVITY);

	//ADD_CUSTOM_CONDITION(CNPC_ShadowWalker,	COND_MYCUSTOMCONDITION);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::Precache( void )
{
	PrecacheModel( "models/monster/subject.mdl" ); // Replace this with setting from Hammer

	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::Spawn( void )
{
	Precache();

	SetModel( "models/monster/subject.mdl" ); // Replace this with setting from Hammer
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 50; // Replace this with setting from Hammer
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;

	CapabilitiesClear();

	if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		// CapabilitiesAdd(bits_CAP_ANIMATEDFACE);
		CapabilitiesAdd(bits_CAP_TURN_HEAD);
		CapabilitiesAdd(bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN | bits_CAP_MOVE_SHOOT);
		CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);
		CapabilitiesAdd(bits_CAP_DUCK | bits_CAP_DOORS_GROUP);
		CapabilitiesAdd(bits_CAP_USE_SHOT_REGULATOR);
	}

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	SetMoveType(MOVETYPE_STEP);

	NPCInit();
}


//-----------------------------------------------------------------------------
// Purpose: Choose a schedule after schedule failed
// Copied from npc_citizen
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode)
{
	switch (failedSchedule)
	{
	case SCHED_NEW_WEAPON:
		// If failed trying to pick up a weapon, try again in one second. This is because other AI code
		// has put this off for 10 seconds under the assumption that the citizen would be able to 
		// pick up the weapon that they found. 
		m_flNextWeaponSearchTime = gpGlobals->curtime + 1.0f;
		break;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		// I can't take cover, so I need to run away!
		return SCHED_RUN_FROM_ENEMY;
	}

	return BaseClass::SelectFailSchedule(failedSchedule, failedTask, taskFailCode);
}

//-----------------------------------------------------------------------------
// Copied from npc_citizen
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectScheduleRetrieveItem()
{
	if (HasCondition(COND_BETTER_WEAPON_AVAILABLE))
	{
		CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>(Weapon_FindUsable(WEAPON_SEARCH_DELTA));
		if (pWeapon)
		{
			m_flNextWeaponSearchTime = gpGlobals->curtime + 10.0;
			// Now lock the weapon for several seconds while we go to pick it up.
			pWeapon->Lock(10.0, this);
			SetTarget(pWeapon);
			return SCHED_NEW_WEAPON;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Copied from npc_citizen - used as SelectSchedule instead of SelectPriorityAction
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectSchedule()
{
	int schedule;
	// Top priority - If there is a weapon available, grab it!
	//schedule = SelectScheduleRetrieveItem();
	//if (schedule != SCHED_NONE)
	//	return schedule;

	// Can I see the enemy?
	//if (HasCondition(COND_SEE_ENEMY) && HasCondition(COND_ENEMY_FACING_ME))
	//{
	//	// Enemy can't see me
	//	if (!HasCondition(COND_HAVE_ENEMY_LOS)) {
	//		return SCHED_CHASE_ENEMY;
	//	}
	//
	//	return SCHED_RUN_FROM_ENEMY;
	//}
	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
		AssertMsgOnce(GetEnemy() == NULL, "NPC has enemy but is not in combat state?");
		return SelectIdleSchedule();

	case NPC_STATE_ALERT:
		AssertMsgOnce(GetEnemy() == NULL, "NPC has enemy but is not in combat state?");
		return SelectAlertSchedule();

	//case NPC_STATE_COMBAT:
		//return SelectCombatSchedule();

	default:
		return BaseClass::SelectSchedule();
	}
}

//-----------------------------------------------------------------------------
// Idle schedule selection
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectIdleSchedule()
{
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	if (HasCondition(COND_HEAR_DANGER) ||
		HasCondition(COND_HEAR_COMBAT) ||
		HasCondition(COND_HEAR_WORLD) ||
		HasCondition(COND_HEAR_BULLET_IMPACT) ||
		HasCondition(COND_HEAR_PLAYER))
	{
		// Investigate sound source
		return SCHED_INVESTIGATE_SOUND;
	}

	// no valid route! Wander instead
	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
		return SCHED_IDLE_WANDER;

	// valid route. Get moving
	return SCHED_IDLE_WALK;
}

//-----------------------------------------------------------------------------
// Alert schedule selection
// Copied from baseNPC
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectAlertSchedule()
{
	// Per default base NPC, check flinch schedule first
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	// Scan around for new enemies
	if (HasCondition(COND_ENEMY_DEAD) && SelectWeightedSequence(ACT_VICTORY_DANCE) != ACTIVITY_NOT_AVAILABLE)
		return SCHED_ALERT_SCAN;

	if (HasCondition(COND_HEAR_DANGER) ||
		HasCondition(COND_HEAR_PLAYER) ||
		HasCondition(COND_HEAR_WORLD) ||
		HasCondition(COND_HEAR_BULLET_IMPACT) ||
		HasCondition(COND_HEAR_COMBAT))
	{
		// Investigate sound source
		return SCHED_INVESTIGATE_SOUND;
	}

	// no valid route! Wander instead
	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
		return SCHED_IDLE_WANDER;

	// valid route. Get moving
	return SCHED_ALERT_WALK;
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_ShadowWalker::Classify( void )
{
	return	CLASS_ZOMBIE;
}
