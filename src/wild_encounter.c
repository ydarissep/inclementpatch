#include "global.h"
#include "wild_encounter.h"
#include "pokemon.h"
#include "metatile_behavior.h"
#include "fieldmap.h"
#include "random.h"
#include "field_player_avatar.h"
#include "event_data.h"
#include "safari_zone.h"
#include "overworld.h"
#include "pokeblock.h"
#include "battle_setup.h"
#include "roamer.h"
#include "tv.h"
#include "link.h"
#include "script.h"
#include "battle_ai_util.h"
#include "battle_util.h"
#include "battle_debug.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "constants/abilities.h"
#include "constants/battle_config.h"
#include "constants/game_stat.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/maps.h"
#include "constants/weather.h"

extern const u8 EventScript_RepelWoreOff[];

// this file's functions
static bool8 IsWildLevelAllowedByRepel(u8 level);
static void ApplyFluteEncounterRateMod(u32 *encRate);
static void ApplyCleanseTagEncounterRateMod(u32 *encRate);
static bool8 TryGetAbilityInfluencedWildMonIndex(const struct WildPokemon *wildMon, u8 type, u16 ability, u8 *monIndex);
static bool8 IsAbilityAllowingEncounter(u8 level);
static u8 GetMedianLevelOfPlayerParty(void);
static u16 GetRandomWildEncounterWithBST(u16 species);

// EWRAM vars
EWRAM_DATA static u8 sWildEncountersDisabled = 0;
EWRAM_DATA bool8 gIsFishingEncounter = 0;
EWRAM_DATA bool8 gIsSurfingEncounter = 0;

#include "data/wild_encounters.h"

//Special Feebas-related data.
const struct WildPokemon gWildFeebasRoute119Data = {20, 25, SPECIES_FEEBAS};

// code
void DisableWildEncounters(bool8 disabled)
{
    sWildEncountersDisabled = disabled;
}

// Feebas now works like it does in ORAS; it can always be found fishing
// under the bridge on Route 119.
static bool8 CheckFeebas(void)
{
    s16 x;
    s16 y;

    if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ROUTE119)
     && gSaveBlock1Ptr->location.mapNum == MAP_NUM(ROUTE119))
    {
        GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
        x -= 7;
        y -= 7;

        // Encounter Feebas if player is fishing under the bridge
        if (y == 35 || y == 36)
        {
            return TRUE;
        }
    }
    return FALSE;
}

static u8 ChooseWildMonIndex_Land(void)
{
    u8 rand = Random() % ENCOUNTER_CHANCE_LAND_MONS_TOTAL;

    if (rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_0)
        return 0;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_1)
        return 1;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_1 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_2)
        return 2;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_2 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_3)
        return 3;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_3 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_4)
        return 4;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_4 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_5)
        return 5;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_5 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_6)
        return 6;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_6 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_7)
        return 7;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_7 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_8)
        return 8;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_8 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_9)
        return 9;
    else if (rand >= ENCOUNTER_CHANCE_LAND_MONS_SLOT_9 && rand < ENCOUNTER_CHANCE_LAND_MONS_SLOT_10)
        return 10;
    else
        return 11;
}

static u8 ChooseWildMonIndex_WaterRock(void)
{
    u8 rand = Random() % ENCOUNTER_CHANCE_WATER_MONS_TOTAL;

    if (rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_0)
        return 0;
    else if (rand >= ENCOUNTER_CHANCE_WATER_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_1)
        return 1;
    else if (rand >= ENCOUNTER_CHANCE_WATER_MONS_SLOT_1 && rand < ENCOUNTER_CHANCE_WATER_MONS_SLOT_2)
        return 2;
    // Encounter slots for surf and rock smash reduced to 4 (60%, 30%, 5%, 5%)
    else
        return 3;
}

static u8 ChooseWildMonIndex_Honey(void)
{
    u8 rand = Random() % ENCOUNTER_CHANCE_HONEY_MONS_TOTAL;

    if (rand < ENCOUNTER_CHANCE_HONEY_MONS_SLOT_0)
        return 0;
    else if (rand >= ENCOUNTER_CHANCE_HONEY_MONS_SLOT_0 && rand < ENCOUNTER_CHANCE_HONEY_MONS_SLOT_1)
        return 1;
    else if (rand >= ENCOUNTER_CHANCE_HONEY_MONS_SLOT_1 && rand < ENCOUNTER_CHANCE_HONEY_MONS_SLOT_2)
        return 2;
    else if (rand >= ENCOUNTER_CHANCE_HONEY_MONS_SLOT_2 && rand < ENCOUNTER_CHANCE_HONEY_MONS_SLOT_3)
        return 3;
    else if (rand >= ENCOUNTER_CHANCE_HONEY_MONS_SLOT_3 && rand < ENCOUNTER_CHANCE_HONEY_MONS_SLOT_4)
        return 4;
    else
        return 5;
}


static u8 ChooseWildMonIndex_Fishing(u8 rod)
{
    u8 wildMonIndex = 0;
    u8 rand = Random() % max(max(ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_TOTAL, ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_TOTAL),
                             ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_TOTAL);

    switch (rod)
    {
    case OLD_ROD:
        if (rand < ENCOUNTER_CHANCE_FISHING_MONS_OLD_ROD_SLOT_0)
            wildMonIndex = 0;
        else
            wildMonIndex = 1;
        break;
    case GOOD_ROD:
        if (rand < ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_2)
            wildMonIndex = 2;
        if (rand >= ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_2 && rand < ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_3)
            wildMonIndex = 3;
        if (rand >= ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_3 && rand < ENCOUNTER_CHANCE_FISHING_MONS_GOOD_ROD_SLOT_4)
            wildMonIndex = 4;
        break;
    case SUPER_ROD:
        if (rand < ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_5)
            wildMonIndex = 5;
        if (rand >= ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_5 && rand < ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_6)
            wildMonIndex = 6;
        if (rand >= ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_6 && rand < ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_7)
            wildMonIndex = 7;
        if (rand >= ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_7 && rand < ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_8)
            wildMonIndex = 8;
        if (rand >= ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_8 && rand < ENCOUNTER_CHANCE_FISHING_MONS_SUPER_ROD_SLOT_9)
            wildMonIndex = 9;
        break;
    }
    return wildMonIndex;
}

// Used to scale wild Pokemon levels
static u8 GetMedianLevelOfPlayerParty(void)
{
    u8 i, j, temp, medianLevel, medianIndex = 0;
    u8 playerPartyCount = CalculatePlayerBattlerPartyCount();
    u8 partyLevels[PARTY_SIZE] = {0};

    // Don't calculate anything if party size is 1
    if (playerPartyCount == 1)
    {
        medianLevel = GetMonData(&gPlayerParty[0], MON_DATA_LEVEL, NULL);
        return medianLevel;
    }

    // Store player levels in partyLevels array
    for (i = 0 ; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL) != SPECIES_EGG)
        {
            partyLevels[i] = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL, NULL);
        }
        else
        {
            partyLevels[i] = 1; 
        }
    }

    // Sort player levels in ascending order
    for (i = 0 ; i < PARTY_SIZE ; i++)
    {
        for (j = 0 ; j < (PARTY_SIZE - 1) ; j++)
        {
            if (partyLevels[j] > partyLevels[j + 1])
            {
                temp = partyLevels[j];
                partyLevels[j] = partyLevels[j + 1];
                partyLevels[j + 1] = temp;
            }
        }
    }
/* 
    Get median level of Pokemon that aren't eggs. Examples:

    partyLevels = [1, 1, 1, 40, 40, 50]
    playerPartyCount = 3, want index 4
    playerPartyCount/2 + (PARTY_SIZE - playerPartyCount) = 1 + (6 - 3) = 4

    partyLevels = [1,  1, 40, 40, 42, 50]
    playerPartyCount = 4, want index 4
    playerPartyCount/2 + (PARTY_SIZE - playerPartyCount) = 2 + (6 - 4) = 4
*/
    medianIndex = (playerPartyCount / 2) + (PARTY_SIZE - playerPartyCount);

    medianLevel = partyLevels[medianIndex];
    
    return medianLevel;
}

static u8 ChooseWildMonLevel(void)
{
    u8 playerMedianLevel = GetMedianLevelOfPlayerParty();
    u8 min;
    u8 max;
    u8 range;
    u8 rand;

    // ensure that min and max are reasonable values
    if (playerMedianLevel < 8)
    {
        min = 2;
        max = 4;
    }
    else
    {
        min = playerMedianLevel - 6;
        max = playerMedianLevel - 3;
    }

    range = max - min + 1;
    rand = Random() % range;

    // check ability for max level mon
    if (!GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
    {
        u16 ability = GetMonAbility(&gPlayerParty[0]);
        if (ability == ABILITY_HUSTLE || ability == ABILITY_VITAL_SPIRIT || ability == ABILITY_PRESSURE)
        {
            if (Random() % 2 == 0)
                return max;

            if (rand != 0)
                rand--;
        }
    }

    return min + rand;
}

u16 GetCurrentMapWildMonHeaderId(void)
{
    u16 i;

    for (i = 0; ; i++)
    {
        const struct WildPokemonHeader *wildHeader = &gWildMonHeaders[i];
        if (wildHeader->mapGroup == 0xFF)
            break;

        if (gWildMonHeaders[i].mapGroup == gSaveBlock1Ptr->location.mapGroup &&
            gWildMonHeaders[i].mapNum == gSaveBlock1Ptr->location.mapNum)
        {
            if (gSaveBlock1Ptr->location.mapGroup == MAP_GROUP(ALTERING_CAVE) &&
                gSaveBlock1Ptr->location.mapNum == MAP_NUM(ALTERING_CAVE))
            {
                u16 alteringCaveId = VarGet(VAR_ALTERING_CAVE_WILD_SET);
                if (alteringCaveId > 8)
                    alteringCaveId = 0;

                i += alteringCaveId;
            }

            return i;
        }
    }

    return -1;
}

static u8 PickWildMonNature(void)
{
    u8 i;
    u8 j;
    struct Pokeblock *safariPokeblock;
    u8 natures[NUM_NATURES];

    if (GetSafariZoneFlag() == TRUE && Random() % 100 < 80)
    {
        safariPokeblock = SafariZoneGetActivePokeblock();
        if (safariPokeblock != NULL)
        {
            for (i = 0; i < NUM_NATURES; i++)
                natures[i] = i;
            for (i = 0; i < NUM_NATURES - 1; i++)
            {
                for (j = i + 1; j < NUM_NATURES; j++)
                {
                    if (Random() & 1)
                    {
                        u8 temp;
                        SWAP(natures[i], natures[j], temp);
                    }
                }
            }
            for (i = 0; i < NUM_NATURES; i++)
            {
                if (PokeblockGetGain(natures[i], safariPokeblock) > 0)
                    return natures[i];
            }
        }
    }
    // check synchronize for a pokemon with the same ability
    if (!GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG)
        && GetMonAbility(&gPlayerParty[0]) == ABILITY_SYNCHRONIZE)
    {
        return GetMonData(&gPlayerParty[0], MON_DATA_NATURE);
    }

    // random nature
    return Random() % NUM_NATURES;
}

static void CreateWildMon(u16 species, u8 level)
{
    bool32 checkCuteCharm;

    ZeroEnemyPartyMons();
    checkCuteCharm = TRUE;
    
    species = GetRandomWildEncounterWithBST(species);

    switch (gBaseStats[species].genderRatio)
    {
    case MON_MALE:
    case MON_FEMALE:
    case MON_GENDERLESS:
        checkCuteCharm = FALSE;
        break;
    }

    if (checkCuteCharm
        && !GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG)
        && GetMonAbility(&gPlayerParty[0]) == ABILITY_CUTE_CHARM
        && Random() % 3 != 0)
    {
        u16 leadingMonSpecies = GetMonData(&gPlayerParty[0], MON_DATA_SPECIES);
        u32 leadingMonPersonality = GetMonData(&gPlayerParty[0], MON_DATA_PERSONALITY);
        u8 gender = GetGenderFromSpeciesAndPersonality(leadingMonSpecies, leadingMonPersonality);

        // misses mon is genderless check, although no genderless mon can have cute charm as ability
        if (gender == MON_FEMALE)
            gender = MON_MALE;
        else
            gender = MON_FEMALE;

        CreateMonWithGenderNatureLetter(&gEnemyParty[0], species, level, 32, gender, PickWildMonNature(), 0);
        return;
    }

    CreateMonWithNature(&gEnemyParty[0], species, level, 32, PickWildMonNature());
}

enum
{
    WILD_AREA_LAND,
    WILD_AREA_WATER,
    WILD_AREA_ROCKS,
    WILD_AREA_FISHING,
    WILD_AREA_HONEY,
};

#define WILD_CHECK_REPEL    0x1
#define WILD_CHECK_KEEN_EYE 0x2

bool8 TryGenerateWildMon(const struct WildPokemonInfo *wildMonInfo, u8 area, u8 flags)
{
    u8 wildMonIndex = 0;
    u8 level;
    u16 species;

    switch (area)
    {
    case WILD_AREA_LAND:
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_STEEL, ABILITY_MAGNET_PULL, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_STATIC, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_LIGHTNING_ROD, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_FIRE, ABILITY_FLASH_FIRE, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_GRASS, ABILITY_HARVEST, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_WATER, ABILITY_STORM_DRAIN, &wildMonIndex))
            break;

        wildMonIndex = ChooseWildMonIndex_Land();
        break;
    case WILD_AREA_WATER:
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_STEEL, ABILITY_MAGNET_PULL, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_STATIC, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_ELECTRIC, ABILITY_LIGHTNING_ROD, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_FIRE, ABILITY_FLASH_FIRE, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_GRASS, ABILITY_HARVEST, &wildMonIndex))
            break;
        if (TryGetAbilityInfluencedWildMonIndex(wildMonInfo->wildPokemon, TYPE_WATER, ABILITY_STORM_DRAIN, &wildMonIndex))
            break;

        wildMonIndex = ChooseWildMonIndex_WaterRock();
        break;
    case WILD_AREA_ROCKS:
        wildMonIndex = ChooseWildMonIndex_WaterRock();
        break;
    case WILD_AREA_HONEY:
        wildMonIndex = ChooseWildMonIndex_Honey();
        break;
    }

    level = ChooseWildMonLevel();
    if (flags & WILD_CHECK_REPEL && !IsWildLevelAllowedByRepel(level))
        return FALSE;
    if (gMapHeader.mapLayoutId != LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS && flags & WILD_CHECK_KEEN_EYE && !IsAbilityAllowingEncounter(level))
        return FALSE;

    species = wildMonInfo->wildPokemon[wildMonIndex].species;

    if (species == SPECIES_MINIOR)
    {
        switch (Random() % 7)
        {
            case 0:
                break;
            case 1:
                species = SPECIES_MINIOR_METEOR_ORANGE;
                break;
            case 2:
                species = SPECIES_MINIOR_METEOR_YELLOW;
                break;
            case 3:
                species = SPECIES_MINIOR_METEOR_GREEN;
                break;
            case 4:
                species = SPECIES_MINIOR_METEOR_BLUE;
                break;
            case 5:
                species = SPECIES_MINIOR_METEOR_INDIGO;
                break;
            case 6:
                species = SPECIES_MINIOR_METEOR_VIOLET;
                break;
        }
    }

    CreateWildMon(species, level);
    return TRUE;
}

static u16 GenerateFishingWildMon(const struct WildPokemonInfo *wildMonInfo, u8 rod)
{
    u8 wildMonIndex = ChooseWildMonIndex_Fishing(rod);
    u8 level = ChooseWildMonLevel();

    CreateWildMon(wildMonInfo->wildPokemon[wildMonIndex].species, level);
    return wildMonInfo->wildPokemon[wildMonIndex].species;
}

static bool8 SetUpMassOutbreakEncounter(u8 flags)
{
    u16 i;

    if (flags & WILD_CHECK_REPEL && !IsWildLevelAllowedByRepel(gSaveBlock1Ptr->outbreakPokemonLevel))
        return FALSE;

    CreateWildMon(gSaveBlock1Ptr->outbreakPokemonSpecies, gSaveBlock1Ptr->outbreakPokemonLevel);
    for (i = 0; i < 4; i++)
        SetMonMoveSlot(&gEnemyParty[0], gSaveBlock1Ptr->outbreakPokemonMoves[i], i);

    return TRUE;
}

static bool8 DoMassOutbreakEncounterTest(void)
{
    if (gSaveBlock1Ptr->outbreakPokemonSpecies != 0
     && gSaveBlock1Ptr->location.mapNum == gSaveBlock1Ptr->outbreakLocationMapNum
     && gSaveBlock1Ptr->location.mapGroup == gSaveBlock1Ptr->outbreakLocationMapGroup)
    {
        if (Random() % 100 < gSaveBlock1Ptr->outbreakPokemonProbability)
            return TRUE;
    }
    return FALSE;
}

static bool8 DoWildEncounterRateDiceRoll(u16 encounterRate)
{
    if (Random() % 2880 < encounterRate)
        return TRUE;
    else
        return FALSE;
}

static bool8 DoWildEncounterRateTest(u32 encounterRate, bool8 ignoreAbility)
{
    encounterRate *= 16;
    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
        encounterRate = encounterRate * 80 / 100;
    ApplyFluteEncounterRateMod(&encounterRate);
    ApplyCleanseTagEncounterRateMod(&encounterRate);
    if (!ignoreAbility && !GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
    {
        u32 ability = GetMonAbility(&gPlayerParty[0]);

        if (ability == ABILITY_STENCH && gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR)
            encounterRate = encounterRate * 3 / 4;
        else if (ability == ABILITY_STENCH)
            encounterRate /= 2;
        else if (ability == ABILITY_ILLUMINATE)
            encounterRate *= 2;
        else if (ability == ABILITY_WHITE_SMOKE)
            encounterRate /= 2;
        else if (ability == ABILITY_ARENA_TRAP)
            encounterRate *= 2;
        else if (ability == ABILITY_SAND_VEIL && gSaveBlock1Ptr->weather == WEATHER_SANDSTORM)
            encounterRate /= 2;
        else if (ability == ABILITY_SNOW_CLOAK && gSaveBlock1Ptr->weather == WEATHER_SNOW)
            encounterRate /= 2;
        else if (ability == ABILITY_QUICK_FEET)
            encounterRate /= 2;
        else if (ability == ABILITY_INFILTRATOR)
            encounterRate /= 2;
        else if (ability == ABILITY_NO_GUARD)
            encounterRate = encounterRate * 3 / 2;
    }
    if (encounterRate > 2880)
        encounterRate = 2880;
    return DoWildEncounterRateDiceRoll(encounterRate);
}

static bool8 DoGlobalWildEncounterDiceRoll(void)
{
    if (Random() % 100 >= 60)
        return FALSE;
    else
        return TRUE;
}

static bool8 AreLegendariesInSootopolisPreventingEncounters(void)
{
    if (gSaveBlock1Ptr->location.mapGroup != MAP_GROUP(SOOTOPOLIS_CITY)
     || gSaveBlock1Ptr->location.mapNum != MAP_NUM(SOOTOPOLIS_CITY))
    {
        return FALSE;
    }

    return FlagGet(FLAG_LEGENDARIES_IN_SOOTOPOLIS);
}

bool8 StandardWildEncounter(u16 currMetaTileBehavior, u16 previousMetaTileBehavior)
{
    u16 headerId;
    struct Roamer *roamer;

    if (sWildEncountersDisabled == TRUE)
        return FALSE;

    headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == 0xFFFF)
    {
        if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS)
        {
            headerId = GetBattlePikeWildMonHeaderId();
            if (previousMetaTileBehavior != currMetaTileBehavior && !DoGlobalWildEncounterDiceRoll())
                return FALSE;
            else if (DoWildEncounterRateTest(gBattlePikeWildMonHeaders[headerId].landMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;
            else if (TryGenerateWildMon(gBattlePikeWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, WILD_CHECK_KEEN_EYE) != TRUE)
                return FALSE;
            else if (!TryGenerateBattlePikeWildMon(TRUE))
                return FALSE;

            BattleSetup_StartBattlePikeWildBattle();
            return TRUE;
        }
        if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR)
        {
            headerId = gSaveBlock2Ptr->frontier.curChallengeBattleNum;
            if (previousMetaTileBehavior != currMetaTileBehavior && !DoGlobalWildEncounterDiceRoll())
                return FALSE;
            else if (DoWildEncounterRateTest(gBattlePyramidWildMonHeaders[headerId].landMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;
            else if (TryGenerateWildMon(gBattlePyramidWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, WILD_CHECK_KEEN_EYE) != TRUE)
                return FALSE;

            GenerateBattlePyramidWildMon();
            BattleSetup_StartWildBattle();
            return TRUE;
        }
    }
    else
    {
        if (MetatileBehavior_IsLandWildEncounter(currMetaTileBehavior) == TRUE)
        {
            if (gWildMonHeaders[headerId].landMonsInfo == NULL)
                return FALSE;
            else if (previousMetaTileBehavior != currMetaTileBehavior && !DoGlobalWildEncounterDiceRoll())
                return FALSE;
            else if (DoWildEncounterRateTest(gWildMonHeaders[headerId].landMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;

            if (TryStartRoamerEncounter() == TRUE)
            {
                roamer = &gSaveBlock1Ptr->roamer;
                if (!IsWildLevelAllowedByRepel(roamer->level))
                    return FALSE;

                BattleSetup_StartRoamerBattle();
                return TRUE;
            }
            else
            {
                if (DoMassOutbreakEncounterTest() == TRUE && SetUpMassOutbreakEncounter(WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
                {
                    BattleSetup_StartWildBattle();
                    return TRUE;
                }

                // try a regular wild land encounter
                if (TryGenerateWildMon(gWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
                {
                    if (TryDoDoubleWildBattle())
                    {
                        struct Pokemon mon1 = gEnemyParty[0];
                        TryGenerateWildMon(gWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, WILD_CHECK_KEEN_EYE);
                        gEnemyParty[1] = mon1;
                        BattleSetup_StartDoubleWildBattle();
                    }
                    else
                    {
                        BattleSetup_StartWildBattle();
                    }
                    return TRUE;
                }

                return FALSE;
            }
        }
        else if (MetatileBehavior_IsWaterWildEncounter(currMetaTileBehavior) == TRUE
                 || (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING) && MetatileBehavior_IsBridge(currMetaTileBehavior) == TRUE))
        {
            if (AreLegendariesInSootopolisPreventingEncounters() == TRUE)
                return FALSE;
            else if (gWildMonHeaders[headerId].waterMonsInfo == NULL)
                return FALSE;
            else if (previousMetaTileBehavior != currMetaTileBehavior && !DoGlobalWildEncounterDiceRoll())
                return FALSE;
            else if (DoWildEncounterRateTest(gWildMonHeaders[headerId].waterMonsInfo->encounterRate, FALSE) != TRUE)
                return FALSE;

            if (TryStartRoamerEncounter() == TRUE)
            {
                roamer = &gSaveBlock1Ptr->roamer;
                if (!IsWildLevelAllowedByRepel(roamer->level))
                    return FALSE;

                BattleSetup_StartRoamerBattle();
                return TRUE;
            }
            else // try a regular surfing encounter
            {
                if (TryGenerateWildMon(gWildMonHeaders[headerId].waterMonsInfo, WILD_AREA_WATER, WILD_CHECK_REPEL | WILD_CHECK_KEEN_EYE) == TRUE)
                {
                    gIsSurfingEncounter = TRUE;
                    if (TryDoDoubleWildBattle())
                    {
                        struct Pokemon mon1 = gEnemyParty[0];
                        TryGenerateWildMon(gWildMonHeaders[headerId].waterMonsInfo, WILD_AREA_WATER, WILD_CHECK_KEEN_EYE);
                        gEnemyParty[1] = mon1;
                        BattleSetup_StartDoubleWildBattle();
                    }
                    else
                    {
                        BattleSetup_StartWildBattle();
                    }
                    return TRUE;
                }

                return FALSE;
            }
        }
    }

    return FALSE;
}

void RockSmashWildEncounter(void)
{
    u16 headerId = GetCurrentMapWildMonHeaderId();

    if (headerId != 0xFFFF)
    {
        const struct WildPokemonInfo *wildPokemonInfo = gWildMonHeaders[headerId].rockSmashMonsInfo;

        if (wildPokemonInfo == NULL)
        {
            gSpecialVar_Result = FALSE;
        }
        // Encounter chance now determined in script
        else
        {
            TryGenerateWildMon(wildPokemonInfo, WILD_AREA_ROCKS, 0);
            BattleSetup_StartWildBattle();
            gSpecialVar_Result = TRUE;
        }
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

void BerryWildEncounter(u8 headerId)
{
    if (gBerryTreeWildMonHeaders[headerId].landMonsInfo != NULL)
    {
        TryGenerateWildMon(gBerryTreeWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, 0);
        BattleSetup_StartWildBattle();
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

bool8 SweetScentWildEncounter(void)
{
    s16 x, y;
    u16 headerId;

    PlayerGetDestCoords(&x, &y);
    headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == 0xFFFF)
    {
        if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PIKE_ROOM_WILD_MONS)
        {
            headerId = GetBattlePikeWildMonHeaderId();
            if (TryGenerateWildMon(gBattlePikeWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, 0) != TRUE)
                return FALSE;

            TryGenerateBattlePikeWildMon(FALSE);
            BattleSetup_StartBattlePikeWildBattle();
            return TRUE;
        }
        if (gMapHeader.mapLayoutId == LAYOUT_BATTLE_FRONTIER_BATTLE_PYRAMID_FLOOR)
        {
            headerId = gSaveBlock2Ptr->frontier.curChallengeBattleNum;
            if (TryGenerateWildMon(gBattlePyramidWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, 0) != TRUE)
                return FALSE;

            GenerateBattlePyramidWildMon();
            BattleSetup_StartWildBattle();
            return TRUE;
        }
    }
    else
    {
        if (MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(x, y)) == TRUE)
        {
            if (gWildMonHeaders[headerId].landMonsInfo == NULL)
                return FALSE;

            if (TryStartRoamerEncounter() == TRUE)
            {
                BattleSetup_StartRoamerBattle();
                return TRUE;
            }

            if (DoMassOutbreakEncounterTest() == TRUE)
                SetUpMassOutbreakEncounter(0);
            else
                TryGenerateWildMon(gWildMonHeaders[headerId].landMonsInfo, WILD_AREA_LAND, 0);

            BattleSetup_StartWildBattle();
            return TRUE;
        }
        else if (MetatileBehavior_IsWaterWildEncounter(MapGridGetMetatileBehaviorAt(x, y)) == TRUE)
        {
            if (AreLegendariesInSootopolisPreventingEncounters() == TRUE)
                return FALSE;
            if (gWildMonHeaders[headerId].waterMonsInfo == NULL)
                return FALSE;

            if (TryStartRoamerEncounter() == TRUE)
            {
                BattleSetup_StartRoamerBattle();
                return TRUE;
            }

            TryGenerateWildMon(gWildMonHeaders[headerId].waterMonsInfo, WILD_AREA_WATER, 0);
            BattleSetup_StartWildBattle();
            return TRUE;
        }
    }

    return FALSE;
}

bool8 DoesCurrentMapHaveFishingMons(void)
{
    u16 headerId = GetCurrentMapWildMonHeaderId();

    if (headerId != 0xFFFF && gWildMonHeaders[headerId].fishingMonsInfo != NULL)
        return TRUE;
    else
        return FALSE;
}

void FishingWildEncounter(u8 rod)
{
    u16 species;

    if (CheckFeebas() == TRUE)
    {
        u8 level = ChooseWildMonLevel();

        species = gWildFeebasRoute119Data.species;
        CreateWildMon(species, level);
    }
    else
    {
        species = GenerateFishingWildMon(gWildMonHeaders[GetCurrentMapWildMonHeaderId()].fishingMonsInfo, rod);
    }
    IncrementGameStat(GAME_STAT_FISHING_CAPTURES);
    SetPokemonAnglerSpecies(species);
    gIsFishingEncounter = TRUE;
    BattleSetup_StartWildBattle();
}

u16 GetLocalWildMon(bool8 *isWaterMon)
{
    u16 headerId;
    const struct WildPokemonInfo *landMonsInfo;
    const struct WildPokemonInfo *waterMonsInfo;

    *isWaterMon = FALSE;
    headerId = GetCurrentMapWildMonHeaderId();
    if (headerId == 0xFFFF)
        return SPECIES_NONE;
    landMonsInfo = gWildMonHeaders[headerId].landMonsInfo;
    waterMonsInfo = gWildMonHeaders[headerId].waterMonsInfo;
    // Neither
    if (landMonsInfo == NULL && waterMonsInfo == NULL)
        return SPECIES_NONE;
    // Land Pokemon
    else if (landMonsInfo != NULL && waterMonsInfo == NULL)
        return landMonsInfo->wildPokemon[ChooseWildMonIndex_Land()].species;
    // Water Pokemon
    else if (landMonsInfo == NULL && waterMonsInfo != NULL)
    {
        *isWaterMon = TRUE;
        return waterMonsInfo->wildPokemon[ChooseWildMonIndex_WaterRock()].species;
    }
    // Either land or water Pokemon
    if ((Random() % 100) < 80)
    {
        return landMonsInfo->wildPokemon[ChooseWildMonIndex_Land()].species;
    }
    else
    {
        *isWaterMon = TRUE;
        return waterMonsInfo->wildPokemon[ChooseWildMonIndex_WaterRock()].species;
    }
}

u16 GetLocalWaterMon(void)
{
    u16 headerId = GetCurrentMapWildMonHeaderId();

    if (headerId != 0xFFFF)
    {
        const struct WildPokemonInfo *waterMonsInfo = gWildMonHeaders[headerId].waterMonsInfo;

        if (waterMonsInfo)
            return waterMonsInfo->wildPokemon[ChooseWildMonIndex_WaterRock()].species;
    }
    return SPECIES_NONE;
}

bool8 UpdateRepelCounter(void)
{
    u16 steps;

    if (InBattlePike() || InBattlePyramid())
        return FALSE;
    if (InUnionRoom() == TRUE)
        return FALSE;

    steps = VarGet(VAR_REPEL_STEP_COUNT);

    if (steps != 0)
    {
        steps--;
        VarSet(VAR_REPEL_STEP_COUNT, steps);
        if (steps == 0)
        {
            ScriptContext1_SetupScript(EventScript_RepelWoreOff);
            return TRUE;
        }
    }
    return FALSE;
}

static bool8 IsWildLevelAllowedByRepel(u8 wildLevel)
{
    u8 i;

    if (!VarGet(VAR_REPEL_STEP_COUNT))
        return TRUE;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_HP) && !GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG))
        {
            u8 ourLevel = GetMonData(&gPlayerParty[i], MON_DATA_LEVEL);

            if (wildLevel < ourLevel)
                return FALSE;
            else
                return TRUE;
        }
    }

    return FALSE;
}

static bool8 IsAbilityAllowingEncounter(u8 level)
{
    u16 ability;

    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        return TRUE;

    ability = GetMonAbility(&gPlayerParty[0]);
    if (ability == ABILITY_KEEN_EYE || ability == ABILITY_INTIMIDATE)
    {
        u8 playerMonLevel = GetMonData(&gPlayerParty[0], MON_DATA_LEVEL);
        if (playerMonLevel > 5 && level <= playerMonLevel - 5 && !(Random() % 2))
            return FALSE;
    }

    return TRUE;
}

static bool8 TryGetRandomWildMonIndexByType(const struct WildPokemon *wildMon, u8 type, u8 numMon, u8 *monIndex)
{
    u8 validIndexes[numMon]; // variable length array, an interesting feature
    u8 i, validMonCount;

    for (i = 0; i < numMon; i++)
        validIndexes[i] = 0;

    for (validMonCount = 0, i = 0; i < numMon; i++)
    {
        if (gBaseStats[wildMon[i].species].type1 == type || gBaseStats[wildMon[i].species].type2 == type)
            validIndexes[validMonCount++] = i;
    }

    if (validMonCount == 0 || validMonCount == numMon)
        return FALSE;

    *monIndex = validIndexes[Random() % validMonCount];
    return TRUE;
}

static bool8 TryGetAbilityInfluencedWildMonIndex(const struct WildPokemon *wildMon, u8 type, u16 ability, u8 *monIndex)
{
    if (GetMonData(&gPlayerParty[0], MON_DATA_SANITY_IS_EGG))
        return FALSE;
    else if (GetMonAbility(&gPlayerParty[0]) != ability)
        return FALSE;
    else if (Random() % 2 != 0)
        return FALSE;

    return TryGetRandomWildMonIndexByType(wildMon, type, LAND_WILD_COUNT, monIndex);
}

static void ApplyFluteEncounterRateMod(u32 *encRate)
{
    if (FlagGet(FLAG_SYS_ENC_UP_ITEM) == TRUE)
        *encRate += *encRate / 2;
    else if (FlagGet(FLAG_SYS_ENC_DOWN_ITEM) == TRUE)
        *encRate = *encRate / 2;
}

static void ApplyCleanseTagEncounterRateMod(u32 *encRate)
{
    if (GetMonData(&gPlayerParty[0], MON_DATA_HELD_ITEM) == ITEM_CLEANSE_TAG)
        *encRate = *encRate * 2 / 3;
}

bool8 TryDoDoubleWildBattle(void)
{
    if (GetSafariZoneFlag() || GetMonsStateToDoubles() != PLAYER_HAS_TWO_USABLE_MONS)
        return FALSE;
    else if (B_FLAG_FORCE_DOUBLE_WILD != 0 && FlagGet(B_FLAG_FORCE_DOUBLE_WILD))
        return TRUE;
    #if B_DOUBLE_WILD_CHANCE != 0
    else if ((Random() % 100) + 1 < B_DOUBLE_WILD_CHANCE)
        return TRUE;
    #endif
    return FALSE;
}

void HoneyWildEncounter(void)
{
    u16 headerId = GetCurrentMapWildMonHeaderId();

    const struct WildPokemonInfo *wildPokemonInfo = gWildMonHeaders[headerId].honeyMonsInfo;

    TryGenerateWildMon(wildPokemonInfo, WILD_AREA_HONEY, 0);
    BattleSetup_StartWildBattle();
}



static const u16 possibleWildEncounter[][1] = //used by GetRandomWildEncounterWithBST, list all possible encounters
{
    SPECIES_BULBASAUR,
    SPECIES_IVYSAUR,
    SPECIES_VENUSAUR,
    SPECIES_CHARMANDER,
    SPECIES_CHARMELEON,
    SPECIES_CHARIZARD,
    SPECIES_SQUIRTLE,
    SPECIES_WARTORTLE,
    SPECIES_BLASTOISE,
    SPECIES_CATERPIE,
    SPECIES_METAPOD,
    SPECIES_BUTTERFREE,
    SPECIES_WEEDLE,
    SPECIES_KAKUNA,
    SPECIES_BEEDRILL,
    SPECIES_PIDGEY,
    SPECIES_PIDGEOTTO,
    SPECIES_PIDGEOT,
    SPECIES_RATTATA,
    SPECIES_RATTATA_ALOLAN,
    SPECIES_RATICATE,
    SPECIES_RATICATE_ALOLAN,
    SPECIES_SPEAROW,
    SPECIES_FEAROW,
    SPECIES_EKANS,
    SPECIES_ARBOK,
    SPECIES_PIKACHU,
    SPECIES_RAICHU,
    SPECIES_RAICHU_ALOLAN,
    SPECIES_SANDSHREW,
    SPECIES_SANDSHREW_ALOLAN,
    SPECIES_SANDSLASH,
    SPECIES_SANDSLASH_ALOLAN,
    SPECIES_NIDORAN_F,
    SPECIES_NIDORINA,
    SPECIES_NIDOQUEEN,
    SPECIES_NIDORAN_M,
    SPECIES_NIDORINO,
    SPECIES_NIDOKING,
    SPECIES_CLEFAIRY,
    SPECIES_CLEFABLE,
    SPECIES_VULPIX,
    SPECIES_VULPIX_ALOLAN,
    SPECIES_NINETALES,
    SPECIES_NINETALES_ALOLAN,
    SPECIES_JIGGLYPUFF,
    SPECIES_WIGGLYTUFF,
    SPECIES_ZUBAT,
    SPECIES_GOLBAT,
    SPECIES_ODDISH,
    SPECIES_GLOOM,
    SPECIES_VILEPLUME,
    SPECIES_PARAS,
    SPECIES_PARASECT,
    SPECIES_VENONAT,
    SPECIES_VENOMOTH,
    SPECIES_DIGLETT,
    SPECIES_DIGLETT_ALOLAN,
    SPECIES_DUGTRIO,
    SPECIES_DUGTRIO_ALOLAN,
    SPECIES_MEOWTH,
    SPECIES_MEOWTH_ALOLAN,
    SPECIES_PERSIAN,
    SPECIES_PERSIAN_ALOLAN,
    SPECIES_PSYDUCK,
    SPECIES_GOLDUCK,
    SPECIES_MANKEY,
    SPECIES_PRIMEAPE,
    SPECIES_GROWLITHE,
    SPECIES_ARCANINE,
    SPECIES_POLIWAG,
    SPECIES_POLIWHIRL,
    SPECIES_POLIWRATH,
    SPECIES_ABRA,
    SPECIES_KADABRA,
    SPECIES_ALAKAZAM,
    SPECIES_MACHOP,
    SPECIES_MACHOKE,
    SPECIES_MACHAMP,
    SPECIES_BELLSPROUT,
    SPECIES_WEEPINBELL,
    SPECIES_VICTREEBEL,
    SPECIES_TENTACOOL,
    SPECIES_TENTACRUEL,
    SPECIES_GEODUDE,
    SPECIES_GEODUDE_ALOLAN,
    SPECIES_GRAVELER,
    SPECIES_GRAVELER_ALOLAN,
    SPECIES_GOLEM,
    SPECIES_GOLEM_ALOLAN,
    SPECIES_PONYTA,
    SPECIES_RAPIDASH,
    SPECIES_SLOWPOKE,
    SPECIES_SLOWBRO,
    SPECIES_MAGNEMITE,
    SPECIES_MAGNETON,
    SPECIES_FARFETCHD,
    SPECIES_DODUO,
    SPECIES_DODRIO,
    SPECIES_SEEL,
    SPECIES_DEWGONG,
    SPECIES_GRIMER,
    SPECIES_GRIMER_ALOLAN,
    SPECIES_MUK,
    SPECIES_MUK_ALOLAN,
    SPECIES_SHELLDER,
    SPECIES_CLOYSTER,
    SPECIES_GASTLY,
    SPECIES_HAUNTER,
    SPECIES_GENGAR,
    SPECIES_ONIX,
    SPECIES_DROWZEE,
    SPECIES_HYPNO,
    SPECIES_KRABBY,
    SPECIES_KINGLER,
    SPECIES_VOLTORB,
    SPECIES_ELECTRODE,
    SPECIES_EXEGGCUTE,
    SPECIES_EXEGGUTOR,
    SPECIES_EXEGGUTOR_ALOLAN,
    SPECIES_CUBONE,
    SPECIES_MAROWAK,
    SPECIES_MAROWAK_ALOLAN,
    SPECIES_HITMONLEE,
    SPECIES_HITMONCHAN,
    SPECIES_LICKITUNG,
    SPECIES_KOFFING,
    SPECIES_WEEZING,
    SPECIES_RHYHORN,
    SPECIES_RHYDON,
    SPECIES_CHANSEY,
    SPECIES_TANGELA,
    SPECIES_KANGASKHAN,
    SPECIES_HORSEA,
    SPECIES_SEADRA,
    SPECIES_GOLDEEN,
    SPECIES_SEAKING,
    SPECIES_STARYU,
    SPECIES_STARMIE,
    SPECIES_MR_MIME,
    SPECIES_SCYTHER,
    SPECIES_JYNX,
    SPECIES_ELECTABUZZ,
    SPECIES_MAGMAR,
    SPECIES_PINSIR,
    SPECIES_TAUROS,
    SPECIES_MAGIKARP,
    SPECIES_GYARADOS,
    SPECIES_LAPRAS,
    SPECIES_DITTO,
    SPECIES_EEVEE,
    SPECIES_VAPOREON,
    SPECIES_JOLTEON,
    SPECIES_FLAREON,
    SPECIES_PORYGON,
    SPECIES_OMANYTE,
    SPECIES_OMASTAR,
    SPECIES_KABUTO,
    SPECIES_KABUTOPS,
    SPECIES_AERODACTYL,
    SPECIES_SNORLAX,
    SPECIES_ARTICUNO,
    SPECIES_ZAPDOS,
    SPECIES_MOLTRES,
    SPECIES_DRATINI,
    SPECIES_DRAGONAIR,
    SPECIES_DRAGONITE,
    SPECIES_MEWTWO,
    SPECIES_MEW,
    SPECIES_CHIKORITA,
    SPECIES_BAYLEEF,
    SPECIES_MEGANIUM,
    SPECIES_CYNDAQUIL,
    SPECIES_QUILAVA,
    SPECIES_TYPHLOSION,
    SPECIES_TOTODILE,
    SPECIES_CROCONAW,
    SPECIES_FERALIGATR,
    SPECIES_SENTRET,
    SPECIES_FURRET,
    SPECIES_HOOTHOOT,
    SPECIES_NOCTOWL,
    SPECIES_LEDYBA,
    SPECIES_LEDIAN,
    SPECIES_SPINARAK,
    SPECIES_ARIADOS,
    SPECIES_CROBAT,
    SPECIES_CHINCHOU,
    SPECIES_LANTURN,
    SPECIES_PICHU,
    SPECIES_CLEFFA,
    SPECIES_IGGLYBUFF,
    SPECIES_TOGEPI,
    SPECIES_TOGETIC,
    SPECIES_NATU,
    SPECIES_XATU,
    SPECIES_MAREEP,
    SPECIES_FLAAFFY,
    SPECIES_AMPHAROS,
    SPECIES_BELLOSSOM,
    SPECIES_MARILL,
    SPECIES_AZUMARILL,
    SPECIES_SUDOWOODO,
    SPECIES_POLITOED,
    SPECIES_HOPPIP,
    SPECIES_SKIPLOOM,
    SPECIES_JUMPLUFF,
    SPECIES_AIPOM,
    SPECIES_SUNKERN,
    SPECIES_SUNFLORA,
    SPECIES_YANMA,
    SPECIES_WOOPER,
    SPECIES_QUAGSIRE,
    SPECIES_ESPEON,
    SPECIES_UMBREON,
    SPECIES_MURKROW,
    SPECIES_SLOWKING,
    SPECIES_MISDREAVUS,
    SPECIES_WOBBUFFET,
    SPECIES_GIRAFARIG,
    SPECIES_PINECO,
    SPECIES_FORRETRESS,
    SPECIES_DUNSPARCE,
    SPECIES_GLIGAR,
    SPECIES_STEELIX,
    SPECIES_SNUBBULL,
    SPECIES_GRANBULL,
    SPECIES_QWILFISH,
    SPECIES_SCIZOR,
    SPECIES_SHUCKLE,
    SPECIES_HERACROSS,
    SPECIES_SNEASEL,
    SPECIES_TEDDIURSA,
    SPECIES_URSARING,
    SPECIES_SLUGMA,
    SPECIES_MAGCARGO,
    SPECIES_SWINUB,
    SPECIES_PILOSWINE,
    SPECIES_CORSOLA,
    SPECIES_REMORAID,
    SPECIES_OCTILLERY,
    SPECIES_DELIBIRD,
    SPECIES_MANTINE,
    SPECIES_SKARMORY,
    SPECIES_HOUNDOUR,
    SPECIES_HOUNDOOM,
    SPECIES_KINGDRA,
    SPECIES_PHANPY,
    SPECIES_DONPHAN,
    SPECIES_PORYGON2,
    SPECIES_STANTLER,
    SPECIES_SMEARGLE,
    SPECIES_TYROGUE,
    SPECIES_HITMONTOP,
    SPECIES_SMOOCHUM,
    SPECIES_ELEKID,
    SPECIES_MAGBY,
    SPECIES_MILTANK,
    SPECIES_BLISSEY,
    SPECIES_LARVITAR,
    SPECIES_PUPITAR,
    SPECIES_TYRANITAR,
    SPECIES_LUGIA,
    SPECIES_HO_OH,
    SPECIES_TREECKO,
    SPECIES_GROVYLE,
    SPECIES_SCEPTILE,
    SPECIES_TORCHIC,
    SPECIES_COMBUSKEN,
    SPECIES_BLAZIKEN,
    SPECIES_MUDKIP,
    SPECIES_MARSHTOMP,
    SPECIES_SWAMPERT,
    SPECIES_POOCHYENA,
    SPECIES_MIGHTYENA,
    SPECIES_ZIGZAGOON,
    SPECIES_LINOONE,
    SPECIES_WURMPLE,
    SPECIES_SILCOON,
    SPECIES_BEAUTIFLY,
    SPECIES_CASCOON,
    SPECIES_DUSTOX,
    SPECIES_LOTAD,
    SPECIES_LOMBRE,
    SPECIES_LUDICOLO,
    SPECIES_SEEDOT,
    SPECIES_NUZLEAF,
    SPECIES_SHIFTRY,
    SPECIES_TAILLOW,
    SPECIES_SWELLOW,
    SPECIES_WINGULL,
    SPECIES_PELIPPER,
    SPECIES_RALTS,
    SPECIES_KIRLIA,
    SPECIES_GARDEVOIR,
    SPECIES_SURSKIT,
    SPECIES_MASQUERAIN,
    SPECIES_SHROOMISH,
    SPECIES_BRELOOM,
    SPECIES_SLAKOTH,
    SPECIES_VIGOROTH,
    SPECIES_SLAKING,
    SPECIES_NINCADA,
    SPECIES_NINJASK,
    SPECIES_SHEDINJA,
    SPECIES_WHISMUR,
    SPECIES_LOUDRED,
    SPECIES_EXPLOUD,
    SPECIES_MAKUHITA,
    SPECIES_HARIYAMA,
    SPECIES_AZURILL,
    SPECIES_NOSEPASS,
    SPECIES_SKITTY,
    SPECIES_DELCATTY,
    SPECIES_SABLEYE,
    SPECIES_MAWILE,
    SPECIES_ARON,
    SPECIES_LAIRON,
    SPECIES_AGGRON,
    SPECIES_MEDITITE,
    SPECIES_MEDICHAM,
    SPECIES_ELECTRIKE,
    SPECIES_MANECTRIC,
    SPECIES_PLUSLE,
    SPECIES_MINUN,
    SPECIES_VOLBEAT,
    SPECIES_ILLUMISE,
    SPECIES_ROSELIA,
    SPECIES_GULPIN,
    SPECIES_SWALOT,
    SPECIES_CARVANHA,
    SPECIES_SHARPEDO,
    SPECIES_WAILMER,
    SPECIES_WAILORD,
    SPECIES_NUMEL,
    SPECIES_CAMERUPT,
    SPECIES_TORKOAL,
    SPECIES_SPOINK,
    SPECIES_GRUMPIG,
    SPECIES_SPINDA,
    SPECIES_TRAPINCH,
    SPECIES_VIBRAVA,
    SPECIES_FLYGON,
    SPECIES_CACNEA,
    SPECIES_CACTURNE,
    SPECIES_SWABLU,
    SPECIES_ALTARIA,
    SPECIES_ZANGOOSE,
    SPECIES_SEVIPER,
    SPECIES_LUNATONE,
    SPECIES_SOLROCK,
    SPECIES_BARBOACH,
    SPECIES_WHISCASH,
    SPECIES_CORPHISH,
    SPECIES_CRAWDAUNT,
    SPECIES_BALTOY,
    SPECIES_CLAYDOL,
    SPECIES_LILEEP,
    SPECIES_CRADILY,
    SPECIES_ANORITH,
    SPECIES_ARMALDO,
    SPECIES_FEEBAS,
    SPECIES_MILOTIC,
    SPECIES_CASTFORM,
    SPECIES_KECLEON,
    SPECIES_SHUPPET,
    SPECIES_BANETTE,
    SPECIES_DUSKULL,
    SPECIES_DUSCLOPS,
    SPECIES_TROPIUS,
    SPECIES_CHIMECHO,
    SPECIES_ABSOL,
    SPECIES_WYNAUT,
    SPECIES_SNORUNT,
    SPECIES_GLALIE,
    SPECIES_SPHEAL,
    SPECIES_SEALEO,
    SPECIES_WALREIN,
    SPECIES_CLAMPERL,
    SPECIES_HUNTAIL,
    SPECIES_GOREBYSS,
    SPECIES_RELICANTH,
    SPECIES_LUVDISC,
    SPECIES_BAGON,
    SPECIES_SHELGON,
    SPECIES_SALAMENCE,
    SPECIES_BELDUM,
    SPECIES_METANG,
    SPECIES_METAGROSS,
    SPECIES_REGIROCK,
    SPECIES_REGICE,
    SPECIES_REGISTEEL,
    SPECIES_LATIAS,
    SPECIES_LATIOS,
    SPECIES_KYOGRE,
    SPECIES_GROUDON,
    SPECIES_RAYQUAZA,
    SPECIES_JIRACHI,
    SPECIES_DEOXYS,
    SPECIES_TURTWIG,
    SPECIES_GROTLE,
    SPECIES_TORTERRA,
    SPECIES_CHIMCHAR,
    SPECIES_MONFERNO,
    SPECIES_INFERNAPE,
    SPECIES_PIPLUP,
    SPECIES_PRINPLUP,
    SPECIES_EMPOLEON,
    SPECIES_STARLY,
    SPECIES_STARAVIA,
    SPECIES_STARAPTOR,
    SPECIES_BIDOOF,
    SPECIES_BIBAREL,
    SPECIES_KRICKETOT,
    SPECIES_KRICKETUNE,
    SPECIES_SHINX,
    SPECIES_LUXIO,
    SPECIES_LUXRAY,
    SPECIES_BUDEW,
    SPECIES_ROSERADE,
    SPECIES_CRANIDOS,
    SPECIES_RAMPARDOS,
    SPECIES_SHIELDON,
    SPECIES_BASTIODON,
    SPECIES_BURMY,
    SPECIES_WORMADAM,
    SPECIES_WORMADAM_SANDY_CLOAK,
    SPECIES_WORMADAM_TRASH_CLOAK,
    SPECIES_MOTHIM,
    SPECIES_COMBEE,
    SPECIES_VESPIQUEN,
    SPECIES_PACHIRISU,
    SPECIES_BUIZEL,
    SPECIES_FLOATZEL,
    SPECIES_CHERUBI,
    SPECIES_CHERRIM,
    SPECIES_SHELLOS,
    SPECIES_GASTRODON,
    SPECIES_AMBIPOM,
    SPECIES_DRIFLOON,
    SPECIES_DRIFBLIM,
    SPECIES_BUNEARY,
    SPECIES_LOPUNNY,
    SPECIES_MISMAGIUS,
    SPECIES_HONCHKROW,
    SPECIES_GLAMEOW,
    SPECIES_PURUGLY,
    SPECIES_CHINGLING,
    SPECIES_STUNKY,
    SPECIES_SKUNTANK,
    SPECIES_BRONZOR,
    SPECIES_BRONZONG,
    SPECIES_BONSLY,
    SPECIES_MIME_JR,
    SPECIES_HAPPINY,
    SPECIES_CHATOT,
    SPECIES_SPIRITOMB,
    SPECIES_GIBLE,
    SPECIES_GABITE,
    SPECIES_GARCHOMP,
    SPECIES_MUNCHLAX,
    SPECIES_RIOLU,
    SPECIES_LUCARIO,
    SPECIES_HIPPOPOTAS,
    SPECIES_HIPPOWDON,
    SPECIES_SKORUPI,
    SPECIES_DRAPION,
    SPECIES_CROAGUNK,
    SPECIES_TOXICROAK,
    SPECIES_CARNIVINE,
    SPECIES_FINNEON,
    SPECIES_LUMINEON,
    SPECIES_MANTYKE,
    SPECIES_SNOVER,
    SPECIES_ABOMASNOW,
    SPECIES_WEAVILE,
    SPECIES_MAGNEZONE,
    SPECIES_LICKILICKY,
    SPECIES_RHYPERIOR,
    SPECIES_TANGROWTH,
    SPECIES_ELECTIVIRE,
    SPECIES_MAGMORTAR,
    SPECIES_TOGEKISS,
    SPECIES_YANMEGA,
    SPECIES_LEAFEON,
    SPECIES_GLACEON,
    SPECIES_GLISCOR,
    SPECIES_MAMOSWINE,
    SPECIES_PORYGON_Z,
    SPECIES_GALLADE,
    SPECIES_PROBOPASS,
    SPECIES_DUSKNOIR,
    SPECIES_FROSLASS,
    SPECIES_ROTOM,
    SPECIES_HEATRAN,
    SPECIES_REGIGIGAS,
    SPECIES_SNIVY,
    SPECIES_SERVINE,
    SPECIES_SERPERIOR,
    SPECIES_TEPIG,
    SPECIES_PIGNITE,
    SPECIES_EMBOAR,
    SPECIES_OSHAWOTT,
    SPECIES_DEWOTT,
    SPECIES_SAMUROTT,
    SPECIES_PATRAT,
    SPECIES_WATCHOG,
    SPECIES_LILLIPUP,
    SPECIES_HERDIER,
    SPECIES_STOUTLAND,
    SPECIES_PURRLOIN,
    SPECIES_LIEPARD,
    SPECIES_PANSAGE,
    SPECIES_SIMISAGE,
    SPECIES_PANSEAR,
    SPECIES_SIMISEAR,
    SPECIES_PANPOUR,
    SPECIES_SIMIPOUR,
    SPECIES_MUNNA,
    SPECIES_MUSHARNA,
    SPECIES_PIDOVE,
    SPECIES_TRANQUILL,
    SPECIES_UNFEZANT,
    SPECIES_BLITZLE,
    SPECIES_ZEBSTRIKA,
    SPECIES_ROGGENROLA,
    SPECIES_BOLDORE,
    SPECIES_GIGALITH,
    SPECIES_WOOBAT,
    SPECIES_SWOOBAT,
    SPECIES_DRILBUR,
    SPECIES_EXCADRILL,
    SPECIES_AUDINO,
    SPECIES_TIMBURR,
    SPECIES_GURDURR,
    SPECIES_CONKELDURR,
    SPECIES_TYMPOLE,
    SPECIES_PALPITOAD,
    SPECIES_SEISMITOAD,
    SPECIES_THROH,
    SPECIES_SAWK,
    SPECIES_SEWADDLE,
    SPECIES_SWADLOON,
    SPECIES_LEAVANNY,
    SPECIES_VENIPEDE,
    SPECIES_WHIRLIPEDE,
    SPECIES_SCOLIPEDE,
    SPECIES_COTTONEE,
    SPECIES_WHIMSICOTT,
    SPECIES_PETILIL,
    SPECIES_LILLIGANT,
    SPECIES_BASCULIN,
    SPECIES_SANDILE,
    SPECIES_KROKOROK,
    SPECIES_KROOKODILE,
    SPECIES_DARUMAKA,
    SPECIES_DARMANITAN,
    SPECIES_MARACTUS,
    SPECIES_DWEBBLE,
    SPECIES_CRUSTLE,
    SPECIES_SCRAGGY,
    SPECIES_SCRAFTY,
    SPECIES_SIGILYPH,
    SPECIES_YAMASK,
    SPECIES_COFAGRIGUS,
    SPECIES_TIRTOUGA,
    SPECIES_CARRACOSTA,
    SPECIES_ARCHEN,
    SPECIES_ARCHEOPS,
    SPECIES_TRUBBISH,
    SPECIES_GARBODOR,
    SPECIES_ZORUA,
    SPECIES_ZOROARK,
    SPECIES_MINCCINO,
    SPECIES_CINCCINO,
    SPECIES_GOTHITA,
    SPECIES_GOTHORITA,
    SPECIES_GOTHITELLE,
    SPECIES_SOLOSIS,
    SPECIES_DUOSION,
    SPECIES_REUNICLUS,
    SPECIES_DUCKLETT,
    SPECIES_SWANNA,
    SPECIES_VANILLITE,
    SPECIES_VANILLISH,
    SPECIES_VANILLUXE,
    SPECIES_DEERLING,
    SPECIES_SAWSBUCK,
    SPECIES_EMOLGA,
    SPECIES_KARRABLAST,
    SPECIES_ESCAVALIER,
    SPECIES_FOONGUS,
    SPECIES_AMOONGUSS,
    SPECIES_FRILLISH,
    SPECIES_JELLICENT,
    SPECIES_ALOMOMOLA,
    SPECIES_JOLTIK,
    SPECIES_GALVANTULA,
    SPECIES_FERROSEED,
    SPECIES_FERROTHORN,
    SPECIES_KLINK,
    SPECIES_KLANG,
    SPECIES_KLINKLANG,
    SPECIES_TYNAMO,
    SPECIES_EELEKTRIK,
    SPECIES_EELEKTROSS,
    SPECIES_ELGYEM,
    SPECIES_BEHEEYEM,
    SPECIES_LITWICK,
    SPECIES_LAMPENT,
    SPECIES_CHANDELURE,
    SPECIES_AXEW,
    SPECIES_FRAXURE,
    SPECIES_HAXORUS,
    SPECIES_CUBCHOO,
    SPECIES_BEARTIC,
    SPECIES_CRYOGONAL,
    SPECIES_SHELMET,
    SPECIES_ACCELGOR,
    SPECIES_STUNFISK,
    SPECIES_MIENFOO,
    SPECIES_MIENSHAO,
    SPECIES_DRUDDIGON,
    SPECIES_GOLETT,
    SPECIES_GOLURK,
    SPECIES_PAWNIARD,
    SPECIES_BISHARP,
    SPECIES_BOUFFALANT,
    SPECIES_RUFFLET,
    SPECIES_BRAVIARY,
    SPECIES_VULLABY,
    SPECIES_MANDIBUZZ,
    SPECIES_HEATMOR,
    SPECIES_DURANT,
    SPECIES_DEINO,
    SPECIES_ZWEILOUS,
    SPECIES_HYDREIGON,
    SPECIES_LARVESTA,
    SPECIES_VOLCARONA,
    SPECIES_MELOETTA,
    SPECIES_CHESPIN,
    SPECIES_QUILLADIN,
    SPECIES_CHESNAUGHT,
    SPECIES_FENNEKIN,
    SPECIES_BRAIXEN,
    SPECIES_DELPHOX,
    SPECIES_FROAKIE,
    SPECIES_FROGADIER,
    SPECIES_GRENINJA,
    SPECIES_BUNNELBY,
    SPECIES_DIGGERSBY,
    SPECIES_FLETCHLING,
    SPECIES_FLETCHINDER,
    SPECIES_TALONFLAME,
    SPECIES_SCATTERBUG,
    SPECIES_SPEWPA,
    SPECIES_VIVILLON,
    SPECIES_VIVILLON_POKE_BALL,
    SPECIES_LITLEO,
    SPECIES_PYROAR,
    SPECIES_FLABEBE,
    SPECIES_FLOETTE,
    SPECIES_FLORGES,
    SPECIES_SKIDDO,
    SPECIES_GOGOAT,
    SPECIES_PANCHAM,
    SPECIES_PANGORO,
    SPECIES_FURFROU,
    SPECIES_ESPURR,
    SPECIES_MEOWSTIC,
    SPECIES_MEOWSTIC_FEMALE,
    SPECIES_HONEDGE,
    SPECIES_DOUBLADE,
    SPECIES_AEGISLASH,
    SPECIES_SPRITZEE,
    SPECIES_AROMATISSE,
    SPECIES_SWIRLIX,
    SPECIES_SLURPUFF,
    SPECIES_INKAY,
    SPECIES_MALAMAR,
    SPECIES_BINACLE,
    SPECIES_BARBARACLE,
    SPECIES_SKRELP,
    SPECIES_DRAGALGE,
    SPECIES_CLAUNCHER,
    SPECIES_CLAWITZER,
    SPECIES_HELIOPTILE,
    SPECIES_HELIOLISK,
    SPECIES_TYRUNT,
    SPECIES_TYRANTRUM,
    SPECIES_AMAURA,
    SPECIES_AURORUS,
    SPECIES_SYLVEON,
    SPECIES_HAWLUCHA,
    SPECIES_DEDENNE,
    SPECIES_CARBINK,
    SPECIES_GOOMY,
    SPECIES_SLIGGOO,
    SPECIES_GOODRA,
    SPECIES_KLEFKI,
    SPECIES_PHANTUMP,
    SPECIES_TREVENANT,
    SPECIES_PUMPKABOO,
    SPECIES_PUMPKABOO_SMALL,
    SPECIES_PUMPKABOO_LARGE,
    SPECIES_PUMPKABOO_SUPER,
    SPECIES_GOURGEIST,
    SPECIES_GOURGEIST_SMALL,
    SPECIES_GOURGEIST_LARGE,
    SPECIES_GOURGEIST_SUPER,
    SPECIES_BERGMITE,
    SPECIES_AVALUGG,
    SPECIES_NOIBAT,
    SPECIES_NOIVERN,
    SPECIES_DIANCIE,
    SPECIES_ROWLET,
    SPECIES_DARTRIX,
    SPECIES_DECIDUEYE,
    SPECIES_LITTEN,
    SPECIES_TORRACAT,
    SPECIES_INCINEROAR,
    SPECIES_POPPLIO,
    SPECIES_BRIONNE,
    SPECIES_PRIMARINA,
    SPECIES_PIKIPEK,
    SPECIES_TRUMBEAK,
    SPECIES_TOUCANNON,
    SPECIES_YUNGOOS,
    SPECIES_GUMSHOOS,
    SPECIES_GRUBBIN,
    SPECIES_CHARJABUG,
    SPECIES_VIKAVOLT,
    SPECIES_CRABRAWLER,
    SPECIES_CRABOMINABLE,
    SPECIES_ORICORIO,
    SPECIES_ORICORIO_POM_POM,
    SPECIES_ORICORIO_PAU,
    SPECIES_ORICORIO_SENSU,
    SPECIES_CUTIEFLY,
    SPECIES_RIBOMBEE,
    SPECIES_ROCKRUFF,
    SPECIES_LYCANROC,
    SPECIES_LYCANROC_MIDNIGHT,
    SPECIES_LYCANROC_DUSK,
    SPECIES_WISHIWASHI,
    SPECIES_MAREANIE,
    SPECIES_TOXAPEX,
    SPECIES_MUDBRAY,
    SPECIES_MUDSDALE,
    SPECIES_DEWPIDER,
    SPECIES_ARAQUANID,
    SPECIES_FOMANTIS,
    SPECIES_LURANTIS,
    SPECIES_MORELULL,
    SPECIES_SHIINOTIC,
    SPECIES_SALANDIT,
    SPECIES_SALAZZLE,
    SPECIES_STUFFUL,
    SPECIES_BEWEAR,
    SPECIES_BOUNSWEET,
    SPECIES_STEENEE,
    SPECIES_TSAREENA,
    SPECIES_COMFEY,
    SPECIES_ORANGURU,
    SPECIES_PASSIMIAN,
    SPECIES_WIMPOD,
    SPECIES_GOLISOPOD,
    SPECIES_SANDYGAST,
    SPECIES_PALOSSAND,
    SPECIES_PYUKUMUKU,
    SPECIES_MINIOR,
    SPECIES_KOMALA,
    SPECIES_TURTONATOR,
    SPECIES_TOGEDEMARU,
    SPECIES_MIMIKYU,
    SPECIES_BRUXISH,
    SPECIES_DRAMPA,
    SPECIES_DHELMISE,
    SPECIES_JANGMO_O,
    SPECIES_HAKAMO_O,
    SPECIES_KOMMO_O,
    SPECIES_COSMOG,
    SPECIES_COSMOEM,
    SPECIES_SOLGALEO,
    SPECIES_LUNALA,
    SPECIES_MAGEARNA,
    SPECIES_MELTAN,
    SPECIES_MELMETAL
};


static u16 GetRandomWildEncounterWithBST (u16 species)
{
    u16 BST = GetTotalBaseStat(species);
    u16 maxBST = 400; 
    u16 rand = 0;
    u16 i = 0;
    u16 j = 0;
    u16 speciesBST = GetTotalBaseStat(species);
    u16 minTargetBST = 0;
    u16 maxTargetBST = 0;
    bool8 keepType = FALSE;
    u8 increment = 25;
    
    u16 speciesInBSTRange[][1] ={ 
    };
    
    // Check player's progression to update maxBST (400 + increment for each badge) // no limit after E4
    if (FlagGet(FLAG_BADGE01_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE02_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE03_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE04_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE05_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE06_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE07_GET))
        maxBST += increment;
    if (FlagGet(FLAG_BADGE08_GET))
        maxBST += increment;
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        maxBST += 5000;
    
    
    // set minTargetBST and maxTargetBST
    if (speciesBST - increment >= 6) // theorically useless
        minTargetBST = speciesBST - increment;
    else
        return species; // cope
    
    if (speciesBST + increment > maxBST)
        if (speciesBST > maxBST) 
        {
            keepType = TRUE; // Will force random encounters to share at least one type with species
            maxTargetBST = speciesBST;
        }
        else
            maxTargetBST = maxBST;
    else
        maxTargetBST = speciesBST + increment;
    
    // Dynamically updated allowedWildEncounter to contain all Pokemon within the speciesBST +/- increment (up to maxBST) 
    // or speciesBST-increment/speciesBST and share one type with species if speciesBST is above maxBST
    for (i = 0; i < ARRAY_COUNT(possibleWildEncounter); i++)
    {
        if (keepType) // Go to the next loop iteration if there's no type in common and keepType is TRUE
        {
            if (!(gBaseStats[species].type1 == gBaseStats[possibleWildEncounter[i][0]].type1
            || gBaseStats[species].type1 == gBaseStats[possibleWildEncounter[i][0]].type2
            || gBaseStats[species].type2 == gBaseStats[possibleWildEncounter[i][0]].type1
            || gBaseStats[species].type2 == gBaseStats[possibleWildEncounter[i][0]].type2))
                continue;
        }
        // if possibleWildEncounter[i][0] is between desired BST range add it to speciesInBSTRange
        if (GetTotalBaseStat(possibleWildEncounter[i][0]) >= minTargetBST && GetTotalBaseStat(possibleWildEncounter[i][0]) <= maxTargetBST) 
        {
                speciesInBSTRange[j][0] = possibleWildEncounter[i][0];
                j++;
        }
    }
    /*
    if (j <= 1) //theorically useless
        return species; // cope
        
    // Choose and return random species
    rand = Random() % j; 
    */
    //return speciesInBSTRange[rand][0];
    return possibleWildEncounter[j][0];
}

