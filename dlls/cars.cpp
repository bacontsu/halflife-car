#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "UserMessages.h"

// ========================================================================
// Tires Entity
// ========================================================================
class CTires : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
};
LINK_ENTITY_TO_CLASS(car_tire, CTires);

void CTires::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/tires.mdl");
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
}

void CTires::Precache()
{
	PRECACHE_MODEL("models/tires.mdl");
}

// ========================================================================
// Car Body Entity
// ========================================================================
class CCarBody : public CBaseEntity
{
public:
	void Spawn() override;
	void Precache() override;
};
LINK_ENTITY_TO_CLASS(car_body, CCarBody);

void CCarBody::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/body.mdl");
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->scale = 5.0f;
}

void CCarBody::Precache()
{
	PRECACHE_MODEL("models/body.mdl");
}

// ========================================================================
// Car Logic Entity
// ========================================================================
class CCarMain : public CBaseEntity
{
public:

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;
	void Activate() override;
	void Precache() override;
	void Think() override;
	bool KeyValue(KeyValueData* pkvd) override;

	string_t m_szTires[4];
	EHANDLE m_pTires[4];
	EHANDLE pPlayer;
	string_t m_szBody;
	EHANDLE m_pBody;

	float m_flSpeed;
	float m_flSteer;

};
LINK_ENTITY_TO_CLASS(car_main, CCarMain);
TYPEDESCRIPTION CCarMain::m_SaveData[] =
{
	DEFINE_FIELD(CCarMain, m_szTires[0], FIELD_STRING),
	DEFINE_FIELD(CCarMain, m_szTires[1], FIELD_STRING),
	DEFINE_FIELD(CCarMain, m_szTires[2], FIELD_STRING),
	DEFINE_FIELD(CCarMain, m_szTires[3], FIELD_STRING),
};
IMPLEMENT_SAVERESTORE(CCarMain, CBaseEntity);

bool CCarMain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "tire1"))
	{
		m_szTires[0] = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "tire2"))
	{
		m_szTires[1] = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "tire3"))
	{
		m_szTires[2] = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "tire4"))
	{
		m_szTires[3] = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "body1"))
	{
		m_szBody = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else
		return CBaseEntity::KeyValue(pkvd);
}


void CCarMain::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "sprites/null.spr");
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
}

void CCarMain::Precache()
{
	PRECACHE_MODEL("sprites/null.spr");
}

void CCarMain::Activate()
{
	// grab all tires
	for (int i = 0; i < 4; i++)
	{
		if (!FStringNull(m_szTires[i]))
		{
			m_pTires[i] = UTIL_FindEntityByTargetname(nullptr, (char*)STRING(m_szTires[i]));
		}
	}

	m_pBody = UTIL_FindEntityByTargetname(nullptr, (char*)STRING(m_szBody));

	pev->nextthink = gpGlobals->time + 1.0f;
}

void CCarMain::Think()
{
	// get dir vectors n stuff
	Vector forward, right, up;
	AngleVectors(pev->angles, &forward, &right, &up);

	// move car to player's origin for testing
	if (!pPlayer)
	{
		pPlayer = CBaseEntity::Instance(INDEXENT(1));
	}
	else
	{
		//UTIL_SetOrigin(pev, pPlayer->pev->origin);
		//pev->angles.y = pPlayer->pev->angles.y;

		// inputs
		if (pPlayer->pev->button & IN_FORWARD)
			m_flSpeed = std::lerp(m_flSpeed, 1.5f, gpGlobals->frametime);
		else if (pPlayer->pev->button & IN_BACK)
			m_flSpeed = std::lerp(m_flSpeed, -1.5f, gpGlobals->frametime);
		else
			m_flSpeed = std::lerp(m_flSpeed, 0.0f, gpGlobals->frametime);

		// only turn when these pressed
		if (pPlayer->pev->button & IN_MOVERIGHT)
		{
			if (m_flSpeed > 0.01f)
				pev->angles.y -= 0.2 * m_flSpeed / 1.5f;
			else if (m_flSpeed < -0.01f)
				pev->angles.y -= 0.2 * m_flSpeed / 1.5f;

			//m_flSteer = -30;
			m_flSteer = std::lerp(m_flSteer, -30.0f, gpGlobals->frametime * 17);
		}
		else if (pPlayer->pev->button & IN_MOVELEFT)
		{
			if (m_flSpeed > 0.01f)
				pev->angles.y += 0.2 * m_flSpeed / 1.5f;
			else if (m_flSpeed < -0.01f)
				pev->angles.y += 0.2 * m_flSpeed / 1.5f;

			//m_flSteer = 30;
			m_flSteer = std::lerp(m_flSteer, 30.0f, gpGlobals->frametime * 17);
		}
		else
			m_flSteer = std::lerp(m_flSteer, 0.0f, gpGlobals->frametime * 17);


		// lock player
		pPlayer->pev->origin = pev->origin + up * 40;
		pPlayer->pev->takedamage = false;
		pPlayer->pev->movetype = MOVETYPE_FLY;
		pPlayer->pev->movetype = SOLID_NOT;
	}

	ALERT(at_console, "speed: %f steer: %f\n", m_flSpeed, m_flSteer);

	// push the car down
	TraceResult carTr;
	UTIL_TraceLine(pev->origin, pev->origin + up * -100, ignore_monsters, dont_ignore_glass, ENT(pev), &carTr);

	if (carTr.flFraction < 1)
		UTIL_SetOrigin(pev, carTr.vecEndPos + up * 50);
	else
		UTIL_SetOrigin(pev, pev->origin + up * -1);

	// move the car
	pev->origin = pev->origin + forward * m_flSpeed;

	// move all tires to its place
	for (int i = 0; i < 4; i++)
	{
		if (!FNullEnt(m_pTires[i]))
		{
			Vector start;
			Vector offset;
			TraceResult tr;
			switch (i)
			{
			case 0:
				offset = forward * 70 + right * 50;
				start = forward * 70 + right * 50 + up * 50;
				break;

			case 1:
				offset = forward * 70 + right * -50;
				start = forward * 70 + right * -50 + up * 50;
				break;

			case 2:
				offset = forward * -75 + right * 50;
				start = forward * -75 + right * 50 + up * 50;
				break;

			case 3:
				offset = forward * -75 + right * -50;
				start = forward * -75 + right * -50 + up * 50;
				break;
			}

			UTIL_TraceLine(pev->origin + start, pev->origin + offset + up * -100, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);
			UTIL_SetOrigin(m_pTires[i]->pev, tr.vecEndPos + up * 20);

			m_pTires[i]->pev->angles.y = (i < 2) ? m_flSteer + pev->angles.y : pev->angles.y;

			// spin the tires if this thing moves
			m_pTires[i]->pev->angles.x -= 1 * m_flSpeed / 1.5f;
		}
	}

	// move body to the car
	//Vector axleOrg;
	//axleOrg.x = pev->origin.x;
	//axleOrg.y = pev->origin.y;

	//float axle1 = (m_pTires[0]->pev->origin.z + ((m_pTires[3]->pev->origin.z - m_pTires[0]->pev->origin.z) / 2));
	//float axle2 = (m_pTires[1]->pev->origin.z + ((m_pTires[2]->pev->origin.z - m_pTires[1]->pev->origin.z) / 2));
	//float axlefinal = (m_pTires[0]->pev->origin.z + ((axle2 - axle1) / 2));

	//axleOrg.z = axlefinal;

	UTIL_SetOrigin(m_pBody->pev, pev->origin + up * -50);
	m_pBody->pev->angles.y = pev->angles.y;

	Vector tireforward = m_pTires[2]->pev->origin - m_pTires[0]->pev->origin;
	Vector tireangle;
	tireforward = tireforward.Normalize();
	VectorAngles(tireforward, tireangle);
	m_pBody->pev->angles.x = -tireangle.x;

	Vector tireright = m_pTires[1]->pev->origin - m_pTires[0]->pev->origin;
	Vector tireangle2;
	tireright = tireright.Normalize();
	VectorAngles(tireright, tireangle2);
	m_pBody->pev->angles.z = tireangle2.x;


	pev->nextthink = gpGlobals->time;
}