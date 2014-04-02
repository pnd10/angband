/*
 * File: src/list-object-flags.h
 * Purpose: object flags for all objects
 *
 * Changing flag order will break savefiles. Flags
 * below start from 0 on line 17, so a flag's sequence number is its line
 * number minus 17.
 *
 * index: the flag number
 * id: when the flag is IDd
 * type: what type of flag is it?
 * power: base power rating for the flag (0 means it is unused or derived)
 * message: what is printed when the flag is IDd (but see also identify.c and
 * list-slays.h)
 */
/* index       	id				type		power	message */
OF(NONE,        0,			 	0,			0,		"")
OF(SLAY_ANIMAL, OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_EVIL,   OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_UNDEAD, OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_DEMON,  OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_ORC,    OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_TROLL,  OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_GIANT,  OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(SLAY_DRAGON, OFID_NORMAL,	OFT_SLAY,	0,		"")
OF(KILL_DRAGON, OFID_NORMAL,	OFT_KILL,	0,		"")
OF(KILL_DEMON,  OFID_NORMAL,	OFT_KILL,	0,		"")
OF(KILL_UNDEAD, OFID_NORMAL,	OFT_KILL,	0,		"")
OF(BRAND_POIS,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_ACID,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_ELEC,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_FIRE,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_COLD,  OFID_WIELD, 	OFT_BRAND,	0,		"")
OF(SUST_STR,    OFID_NORMAL,	OFT_SUST,	9,		"Your %s glows.")
OF(SUST_INT,    OFID_NORMAL,	OFT_SUST,	4,		"Your %s glows.")
OF(SUST_WIS,    OFID_NORMAL,	OFT_SUST,	4,		"Your %s glows.")
OF(SUST_DEX,    OFID_NORMAL,	OFT_SUST,	7,		"Your %s glows.")
OF(SUST_CON,    OFID_NORMAL,	OFT_SUST,	8,		"Your %s glows.")
OF(VULN_ACID,   OFID_NORMAL,	OFT_VULN,	-6,		"Your %s glows.")
OF(VULN_ELEC,   OFID_NORMAL,	OFT_VULN,	-6,		"Your %s glows.")
OF(VULN_FIRE,   OFID_NORMAL,	OFT_VULN,	-6,		"Your %s glows.")
OF(VULN_COLD,   OFID_NORMAL,	OFT_VULN,	-6,		"Your %s glows.")
OF(IM_ACID,     OFID_NORMAL,	OFT_IMM,	38,		"Your %s glows.")
OF(IM_ELEC,     OFID_NORMAL,	OFT_IMM,	35,		"Your %s glows.")
OF(IM_FIRE,     OFID_NORMAL,	OFT_IMM,	40,		"Your %s glows.")
OF(IM_COLD,     OFID_NORMAL,	OFT_IMM,	37,		"Your %s glows.")
OF(RES_ACID,    OFID_NORMAL,	OFT_LRES,	5,		"Your %s glows.")
OF(RES_ELEC,    OFID_NORMAL,	OFT_LRES,	6,		"Your %s glows.")
OF(RES_FIRE,    OFID_NORMAL,	OFT_LRES,	6,		"Your %s glows.")
OF(RES_COLD,    OFID_NORMAL,	OFT_LRES,	6,		"Your %s glows.")
OF(RES_POIS,    OFID_NORMAL,	OFT_HRES,	28,		"Your %s glows.")
OF(RES_FEAR,    OFID_NORMAL,	OFT_PROT,	6,		"Your %s glows.")
OF(RES_LIGHT,   OFID_NORMAL,	OFT_HRES,	6,		"Your %s glows.")
OF(RES_DARK,    OFID_NORMAL,	OFT_HRES,	16,		"Your %s glows.")
OF(RES_BLIND,   OFID_NORMAL,	OFT_PROT,	16,		"Your %s glows.")
OF(RES_CONFU,   OFID_NORMAL,	OFT_PROT,	24,		"Your %s glows.")
OF(RES_SOUND,   OFID_NORMAL,	OFT_HRES,	14,		"Your %s glows.")
OF(RES_SHARD,   OFID_NORMAL,	OFT_HRES,	8,		"Your %s glows.")
OF(RES_NEXUS,   OFID_NORMAL,	OFT_HRES,	15,		"Your %s glows.")
OF(RES_NETHR,   OFID_NORMAL,	OFT_HRES,	20,		"Your %s glows.")
OF(RES_CHAOS,   OFID_NORMAL,	OFT_HRES,	20,		"Your %s glows.")
OF(RES_DISEN,   OFID_NORMAL,	OFT_HRES,	20,		"Your %s glows.")
OF(SLOW_DIGEST, OFID_TIMED,		OFT_MISC,	2,		"You feel your %s slow your metabolism.")
OF(FEATHER,     OFID_NORMAL,	OFT_MISC,	1,		"Your %s slows your fall.")
OF(REGEN,       OFID_TIMED,		OFT_MISC,	5,		"You feel your %s speed up your recovery.")
OF(TELEPATHY,   OFID_WIELD,		OFT_MISC,	35,		"")
OF(SEE_INVIS,   OFID_WIELD,		OFT_MISC,	6,		"")
OF(FREE_ACT,    OFID_NORMAL,	OFT_MISC,	8,		"Your %s glows.")
OF(HOLD_LIFE,   OFID_NORMAL,	OFT_MISC,	5,		"Your %s glows.")
OF(NO_FUEL,     OFID_WIELD,		OFT_LIGHT,	5,		"")
OF(IMPAIR_HP,   OFID_TIMED,		OFT_BAD,	-9,		"You feel your %s slow your recovery.")
OF(IMPAIR_MANA, OFID_TIMED,		OFT_BAD,	-9,		"You feel your %s slow your mana recovery.")
OF(AFRAID,      OFID_WIELD,		OFT_BAD,	-20,	"")
OF(IMPACT,      OFID_NORMAL,	OFT_MELEE,	10,		"Your %s causes an earthquake!")
OF(TELEPORT,    OFID_NORMAL,	OFT_BAD,	-20,	"Your %s teleports you.")
OF(AGGRAVATE,   OFID_TIMED,		OFT_BAD,	-20,	"You feel your %s aggravate things around you.")
OF(DRAIN_EXP,   OFID_TIMED,		OFT_BAD,	-5,		"You feel your %s drain your life.")
OF(IGNORE_ACID, OFID_NORMAL,	OFT_IGNORE,	3,		"")
OF(IGNORE_ELEC, OFID_NORMAL,	OFT_IGNORE,	1,		"")
OF(IGNORE_FIRE, OFID_NORMAL,	OFT_IGNORE,	3,		"")
OF(IGNORE_COLD, OFID_NORMAL,	OFT_IGNORE,	1,		"")
OF(RES_STUN,    OFID_NORMAL,	OFT_PROT,	12,		"Your %s glows.")
OF(BLESSED,     OFID_WIELD,		OFT_MELEE,	1,		"")
OF(LIGHT_CURSE, OFID_WIELD,		OFT_CURSE,	-5,     "")
OF(HEAVY_CURSE, OFID_WIELD,		OFT_CURSE,	-15,    "")
OF(PERMA_CURSE, OFID_WIELD,		OFT_CURSE,	-25,    "")
OF(HATES_ACID,  OFID_NONE,		OFT_HATES,	0,      "")
OF(HATES_ELEC,  OFID_NONE,		OFT_HATES,	0,      "")
OF(HATES_FIRE,  OFID_NONE,		OFT_HATES,	0,      "")
OF(HATES_COLD,  OFID_NONE,		OFT_HATES,	0,      "")
OF(SPELLS_OK,   OFID_NONE,		OFT_INT,	0,      "")
OF(SHOW_DICE,   OFID_NONE,		OFT_INT,	0,      "")
OF(SHOW_MULT,	OFID_NONE,		OFT_INT,	0,      "")
OF(BRAND_ICKY,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_FIZZ,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_BUZZ,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_WARM,  OFID_WIELD,		OFT_BRAND,	0,		"")
OF(BRAND_COOL,  OFID_WIELD, 	OFT_BRAND,	0,		"")
OF(MAX,			0,				0,			0,      "")
