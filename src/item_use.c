#include "global.h"
#include "item_use.h"
#include "battle.h"
#include "battle_anim.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "berry.h"
#include "berry_powder.h"
#include "bike.h"
#include "coins.h"
#include "data.h"
#include "daycare.h"
#include "event_data.h"
#include "event_object_lock.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "item.h"
#include "item_menu.h"
#include "item_use.h"
#include "mail.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokeblock.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "wild_encounter.h"
#include "constants/event_bg.h"
#include "constants/event_objects.h"
#include "constants/item_effects.h"
#include "constants/items.h"
#include "constants/songs.h"

static void SetUpItemUseCallback(u8 taskId);
static void FieldCB_UseItemOnField(void);
static void Task_CallItemUseOnFieldCallback(u8 taskId);
static void Task_UseItemfinder(u8 taskId);
static void Task_CloseItemfinderMessage(u8 taskId);
static void Task_HiddenItemNearby(u8 taskId);
static void Task_StandingOnHiddenItem(u8 taskId);
static bool8 ItemfinderCheckForHiddenItems(const struct MapEvents *, u8);
static u8 GetDirectionToHiddenItem(s16 distanceX, s16 distanceY);
static void PlayerFaceHiddenItem(u8 a);
static void CheckForHiddenItemsInMapConnection(u8 taskId);
static void ItemUseOnFieldCB_PokeblockCase(u8 taskId);
static void Task_OpenRegisteredPokeblockCase(u8 taskId);
static void ItemUseOnFieldCB_Bike(u8 taskId);
static void ItemUseOnFieldCB_Rod(u8);
static void ItemUseOnFieldCB_Itemfinder(u8);
static void ItemUseOnFieldCB_Berry(u8 taskId);
static void ItemUseOnFieldCB_WailmerPailBerry(u8 taskId);
static void ItemUseOnFieldCB_WailmerPailSudowoodo(u8 taskId);
static bool8 TryToWaterSudowoodo(void);
static void BootUpSoundTMHM(u8 taskId);
static void Task_ShowTMHMContainedMessage(u8 taskId);
static void UseTMHMYesNo(u8 taskId);
static void UseTMHM(u8 taskId);
static void Task_StartUseRepel(u8 taskId);
static void Task_UseRepel(u8 taskId);
static void ItemUseOnFieldCB_PokeVial(u8 taskId);
static void Task_CloseCantUseKeyItemMessage(u8 taskId);
static void SetDistanceOfClosestHiddenItem(u8 taskId, s16 x, s16 y);
static void CB2_OpenPokeblockFromBag(void);
static void ItemUseOnFieldCB_Honey(u8 taskId);
static void ItemUseOnFieldCB_HoneyFail(u8 taskId);

// EWRAM variables
EWRAM_DATA static void(*sItemUseOnFieldCB)(u8 taskId) = NULL;

// Below is set TRUE by UseRegisteredKeyItemOnField
#define tUsingRegisteredKeyItem  data[3]

// UB here if an item with type ITEM_USE_MAIL or ITEM_USE_BAG_MENU uses SetUpItemUseCallback
// Never occurs in vanilla, but can occur with improperly created items
static const MainCallback sItemUseCallbacks[] =
{
    [ITEM_USE_PARTY_MENU - 1]  = CB2_ShowPartyMenuForItemUse,
    [ITEM_USE_FIELD - 1]       = CB2_ReturnToField,
    [ITEM_USE_PBLOCK_CASE - 1] = NULL,
};

static const u8 sClockwiseDirections[] = {DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST};

static const struct YesNoFuncTable sUseTMHMYesNoFuncTable =
{
    .yesFunc = UseTMHM,
    .noFunc = CloseItemMessage,
};

#define tEnigmaBerryType data[4]
static void SetUpItemUseCallback(u8 taskId)
{
    u8 type;
    if (gSpecialVar_ItemId == ITEM_ENIGMA_BERRY)
        type = gTasks[taskId].tEnigmaBerryType - 1;
    else
        type = ItemId_GetType(gSpecialVar_ItemId) - 1;
    if (!InBattlePyramid())
    {
        gBagMenu->newScreenCallback = sItemUseCallbacks[type];
        Task_FadeAndCloseBagMenu(taskId);
    }
    else
    {
        gPyramidBagMenu->newScreenCallback = sItemUseCallbacks[type];
        CloseBattlePyramidBag(taskId);
    }
}

static void SetUpItemUseOnFieldCallback(u8 taskId)
{
    if (gTasks[taskId].tUsingRegisteredKeyItem != TRUE)
    {
        gFieldCallback = FieldCB_UseItemOnField;
        SetUpItemUseCallback(taskId);
    }
    else
        sItemUseOnFieldCB(taskId);
}

static void FieldCB_UseItemOnField(void)
{
    FadeInFromBlack();
    CreateTask(Task_CallItemUseOnFieldCallback, 8);
}

static void Task_CallItemUseOnFieldCallback(u8 taskId)
{
    if (IsWeatherNotFadingIn() == 1)
        sItemUseOnFieldCB(taskId);
}

static void DisplayCannotUseItemMessage(u8 taskId, bool8 isUsingRegisteredKeyItemOnField, const u8 *str)
{
    StringExpandPlaceholders(gStringVar4, str);
    if (!isUsingRegisteredKeyItemOnField)
    {
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, gStringVar4, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, gText_DadsAdvice, Task_CloseBattlePyramidBagMessage);
    }
    else
        DisplayItemMessageOnField(taskId, gStringVar4, Task_CloseCantUseKeyItemMessage);
}

static void DisplayDadsAdviceCannotUseItemMessage(u8 taskId, bool8 isUsingRegisteredKeyItemOnField)
{
    DisplayCannotUseItemMessage(taskId, isUsingRegisteredKeyItemOnField, gText_DadsAdvice);
}

static void DisplayCannotDismountBikeMessage(u8 taskId, bool8 isUsingRegisteredKeyItemOnField)
{
    DisplayCannotUseItemMessage(taskId, isUsingRegisteredKeyItemOnField, gText_CantDismountBike);
}

static void Task_CloseCantUseKeyItemMessage(u8 taskId)
{
    ClearDialogWindowAndFrame(0, 1);
    DestroyTask(taskId);
    ScriptUnfreezeObjectEvents();
    ScriptContext2_Disable();
}

u8 CheckIfItemIsTMHMOrEvolutionStone(u16 itemId)
{
    if (ItemId_GetFieldFunc(itemId) == ItemUseOutOfBattle_TMHM)
        return 1;
    else if (ItemId_GetFieldFunc(itemId) == ItemUseOutOfBattle_EvolutionStone)
        return 2;
    else
        return 0;
}

// Mail in the bag menu can't have a message but it can be checked (view the mail background, no message)
static void CB2_CheckMail(void)
{
    struct MailStruct mail;
    mail.itemId = gSpecialVar_ItemId;
    ReadMail(&mail, CB2_ReturnToBagMenuPocket, 0);
}

void ItemUseOutOfBattle_Mail(u8 taskId)
{
    gBagMenu->newScreenCallback = CB2_CheckMail;
    Task_FadeAndCloseBagMenu(taskId);
}

void ItemUseOutOfBattle_Bike(u8 taskId)
{
    s16* data = gTasks[taskId].data;
    s16 coordsY;
    s16 coordsX;
    u8 behavior;
    PlayerGetDestCoords(&coordsX, &coordsY);
    behavior = MapGridGetMetatileBehaviorAt(coordsX, coordsY);
    if (FlagGet(FLAG_SYS_CYCLING_ROAD) == TRUE || MetatileBehavior_IsVerticalRail(behavior) == TRUE || MetatileBehavior_IsHorizontalRail(behavior) == TRUE || MetatileBehavior_IsIsolatedVerticalRail(behavior) == TRUE || MetatileBehavior_IsIsolatedHorizontalRail(behavior) == TRUE)
        DisplayCannotDismountBikeMessage(taskId, tUsingRegisteredKeyItem);
    else
    {
        if (Overworld_IsBikingAllowed() == TRUE && IsBikingDisallowedByPlayer() == 0)
        {
            sItemUseOnFieldCB = ItemUseOnFieldCB_Bike;
            SetUpItemUseOnFieldCallback(taskId);
        }
        else
            DisplayDadsAdviceCannotUseItemMessage(taskId, tUsingRegisteredKeyItem);
    }
}

static void ItemUseOnFieldCB_Bike(u8 taskId)
{
    if (ItemId_GetSecondaryId(gSpecialVar_ItemId) == MACH_BIKE)
        GetOnOffBike(PLAYER_AVATAR_FLAG_MACH_BIKE);
    else // ACRO_BIKE
        GetOnOffBike(PLAYER_AVATAR_FLAG_ACRO_BIKE);
    ScriptUnfreezeObjectEvents();
    ScriptContext2_Disable();
    DestroyTask(taskId);
}

static bool32 CanFish(void)
{
    s16 x, y;
    u16 tileBehavior;

    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    tileBehavior = MapGridGetMetatileBehaviorAt(x, y);

    if (MetatileBehavior_IsWaterfall(tileBehavior))
        return FALSE;

    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_UNDERWATER))
        return FALSE;

    if (!TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
    {
        if (IsPlayerFacingSurfableFishableWater())
            return TRUE;
    }
    else
    {
        if (MetatileBehavior_IsSurfableWaterOrUnderwater(tileBehavior) && !MapGridIsImpassableAt(x, y))
            return TRUE;
        if (MetatileBehavior_8089510(tileBehavior) == TRUE)
            return TRUE;
    }

    return FALSE;
}

void ItemUseOutOfBattle_Rod(u8 taskId)
{
    if (CanFish() == TRUE)
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_Rod;
        SetUpItemUseOnFieldCallback(taskId);
    }
    else
        DisplayDadsAdviceCannotUseItemMessage(taskId, gTasks[taskId].tUsingRegisteredKeyItem);
}

static void ItemUseOnFieldCB_Rod(u8 taskId)
{
    StartFishing(ItemId_GetSecondaryId(gSpecialVar_ItemId));
    DestroyTask(taskId);
}

void ItemUseOutOfBattle_Itemfinder(u8 var)
{
    IncrementGameStat(GAME_STAT_USED_ITEMFINDER);
    sItemUseOnFieldCB = ItemUseOnFieldCB_Itemfinder;
    SetUpItemUseOnFieldCallback(var);
}

static void ItemUseOnFieldCB_Itemfinder(u8 taskId)
{
    if (ItemfinderCheckForHiddenItems(gMapHeader.events, taskId) == TRUE)
        gTasks[taskId].func = Task_UseItemfinder;
    else
        DisplayItemMessageOnField(taskId, gText_ItemFinderNothing, Task_CloseItemfinderMessage);
}

// Define itemfinder task data
#define tItemDistanceX    data[0]
#define tItemDistanceY    data[1]
#define tItemFound        data[2]
#define tCounter          data[3] // Used to count delay between beeps and rotations during player spin
#define tItemfinderBeeps  data[4]
#define tFacingDir        data[5]

static void Task_UseItemfinder(u8 taskId)
{
    u8 playerDir;
    u8 playerDirToItem;
    u8 i;
    s16* data = gTasks[taskId].data;
    if (tCounter == 0)
    {
        if (tItemfinderBeeps == 4)
        {
            playerDirToItem = GetDirectionToHiddenItem(tItemDistanceX, tItemDistanceY);
            if (playerDirToItem != DIR_NONE)
            {
                PlayerFaceHiddenItem(sClockwiseDirections[playerDirToItem - 1]);
                gTasks[taskId].func = Task_HiddenItemNearby;
            }
            else
            {
                // Player is standing on hidden item
                playerDir = GetPlayerFacingDirection();
                for (i = 0; i < ARRAY_COUNT(sClockwiseDirections); i++)
                {
                    if (playerDir == sClockwiseDirections[i])
                        tFacingDir = (i + 1) & 3;
                }
                gTasks[taskId].func = Task_StandingOnHiddenItem;
                tCounter = 0;
                tItemFound = 0;
            }
            return;
        }
        PlaySE(SE_ITEMFINDER);
        tItemfinderBeeps++;
    }
    tCounter = (tCounter + 1) & 0x1F;
}

static void Task_CloseItemfinderMessage(u8 taskId)
{
    ClearDialogWindowAndFrame(0, 1);
    ScriptUnfreezeObjectEvents();
    ScriptContext2_Disable();
    DestroyTask(taskId);
}

static bool8 ItemfinderCheckForHiddenItems(const struct MapEvents *events, u8 taskId)
{
    int itemX, itemY;
    s16 playerX, playerY, i, distanceX, distanceY;
    PlayerGetDestCoords(&playerX, &playerY);
    gTasks[taskId].tItemFound = FALSE;

    for (i = 0; i < events->bgEventCount; i++)
    {
        // Check if there are any hidden items on the current map that haven't been picked up
        if (events->bgEvents[i].kind == BG_EVENT_HIDDEN_ITEM && !FlagGet(events->bgEvents[i].bgUnion.hiddenItem.hiddenItemId + FLAG_HIDDEN_ITEMS_START))
        {
            itemX = (u16)events->bgEvents[i].x + 7;
            distanceX = itemX - playerX;
            itemY = (u16)events->bgEvents[i].y + 7;
            distanceY = itemY - playerY;

            if ((u16)(distanceX + 7) < 15 && (distanceY >= -5) && (distanceY < 6))
                SetDistanceOfClosestHiddenItem(taskId, distanceX, distanceY);
        }
    }

    CheckForHiddenItemsInMapConnection(taskId);
    if (gTasks[taskId].tItemFound == TRUE)
        return TRUE;
    else
        return FALSE;
}

static bool8 IsHiddenItemPresentAtCoords(const struct MapEvents *events, s16 x, s16 y)
{
    u8 bgEventCount = events->bgEventCount;
    struct BgEvent *bgEvent = events->bgEvents;
    int i;

    for (i = 0; i < bgEventCount; i++)
    {
        if (bgEvent[i].kind == BG_EVENT_HIDDEN_ITEM && x == (u16)bgEvent[i].x && y == (u16)bgEvent[i].y) // hidden item and coordinates matches x and y passed?
        {
            if (!FlagGet(bgEvent[i].bgUnion.hiddenItem.hiddenItemId + FLAG_HIDDEN_ITEMS_START))
                return TRUE;
            else
                return FALSE;
        }
    }
    return FALSE;
}

static bool8 IsHiddenItemPresentInConnection(struct MapConnection *connection, int x, int y)
{

    u16 localX, localY;
    u32 localOffset;
    s32 localLength;

    struct MapHeader const *const mapHeader = GetMapHeaderFromConnection(connection);

    switch (connection->direction)
    {
    // same weird temp variable behavior seen in IsHiddenItemPresentAtCoords
    case 2:
        localOffset = connection->offset + 7;
        localX = x - localOffset;
        localLength = mapHeader->mapLayout->height - 7;
        localY = localLength + y; // additions are reversed for some reason
        break;
    case 1:
        localOffset = connection->offset + 7;
        localX = x - localOffset;
        localLength = gMapHeader.mapLayout->height + 7;
        localY = y - localLength;
        break;
    case 3:
        localLength = mapHeader->mapLayout->width - 7;
        localX = localLength + x; // additions are reversed for some reason
        localOffset = connection->offset + 7;
        localY = y - localOffset;
        break;
    case 4:
        localLength = gMapHeader.mapLayout->width + 7;
        localX = x - localLength;
        localOffset = connection->offset + 7;
        localY = y - localOffset;
        break;
    default:
        return FALSE;
    }
    return IsHiddenItemPresentAtCoords(mapHeader->events, localX, localY);
}

static void CheckForHiddenItemsInMapConnection(u8 taskId)
{
    s16 playerX, playerY;
    s16 x, y;
    s16 width = gMapHeader.mapLayout->width + 7;
    s16 height = gMapHeader.mapLayout->height + 7;

    s16 var1 = 7;
    s16 var2 = 7;

    PlayerGetDestCoords(&playerX, &playerY);

    for (x = playerX - 7; x <= playerX + 7; x++)
    {
        for (y = playerY - 5; y <= playerY + 5; y++)
        {
            if (var1 > x
             || x >= width
             || var2 > y
             || y >= height)
            {
                struct MapConnection *conn = GetConnectionAtCoords(x, y);
                if (conn && IsHiddenItemPresentInConnection(conn, x, y) == TRUE)
                    SetDistanceOfClosestHiddenItem(taskId, x - playerX, y - playerY);
            }
        }
    }
}

static void SetDistanceOfClosestHiddenItem(u8 taskId, s16 itemDistanceX, s16 itemDistanceY)
{
    s16 *data = gTasks[taskId].data;
    s16 oldItemAbsX, oldItemAbsY, newItemAbsX, newItemAbsY;

    if (tItemFound == FALSE)
    {
        // No other items found yet, set this one
        tItemDistanceX = itemDistanceX;
        tItemDistanceY = itemDistanceY;
        tItemFound = TRUE;
    }
    else
    {
        // Other items have been found, check if this one is closer

        // Get absolute x distance of the already-found item
        if (tItemDistanceX < 0)
            oldItemAbsX = tItemDistanceX * -1; // WEST
        else
            oldItemAbsX = tItemDistanceX;      // EAST

        // Get absolute y distance of the already-found item
        if (tItemDistanceY < 0)
            oldItemAbsY = tItemDistanceY * -1; // NORTH
        else
            oldItemAbsY = tItemDistanceY;      // SOUTH

        // Get absolute x distance of the newly-found item
        if (itemDistanceX < 0)
            newItemAbsX = itemDistanceX * -1;
        else
            newItemAbsX = itemDistanceX;

        // Get absolute y distance of the newly-found item
        if (itemDistanceY < 0)
            newItemAbsY = itemDistanceY * -1;
        else
            newItemAbsY = itemDistanceY;


        if (oldItemAbsX + oldItemAbsY > newItemAbsX + newItemAbsY)
        {
            // New item is closer
            tItemDistanceX = itemDistanceX;
            tItemDistanceY = itemDistanceY;
        }
        else
        {
            if (oldItemAbsX + oldItemAbsY == newItemAbsX + newItemAbsY
            && (oldItemAbsY > newItemAbsY || (oldItemAbsY == newItemAbsY && tItemDistanceY < itemDistanceY)))
            {
                // If items are equal distance, use whichever is closer on the Y axis or further south
                tItemDistanceX = itemDistanceX;
                tItemDistanceY = itemDistanceY;
            }
        }
    }
}

static u8 GetDirectionToHiddenItem(s16 itemDistanceX, s16 itemDistanceY)
{
    s16 absX, absY;

    if (itemDistanceX == 0 && itemDistanceY == 0)
        return DIR_NONE; // player is standing on the item.

    // Get absolute X distance.
    if (itemDistanceX < 0)
        absX = itemDistanceX * -1;
    else
        absX = itemDistanceX;

    // Get absolute Y distance.
    if (itemDistanceY < 0)
        absY = itemDistanceY * -1;
    else
        absY = itemDistanceY;

    if (absX > absY)
    {
        if (itemDistanceX < 0)
            return DIR_EAST;
        else
            return DIR_NORTH;
    }
    else
    {
        if (absX < absY)
        {
            if (itemDistanceY < 0)
                return DIR_SOUTH;
            else
                return DIR_WEST;
        }
        if (absX == absY)
        {
            if (itemDistanceY < 0)
                return DIR_SOUTH;
            else
                return DIR_WEST;
        }
        return DIR_NONE; // Unreachable
    }
}

static void PlayerFaceHiddenItem(u8 direction)
{
    ObjectEventClearHeldMovementIfFinished(&gObjectEvents[GetObjectEventIdByLocalIdAndMap(OBJ_EVENT_ID_PLAYER, 0, 0)]);
    ObjectEventClearHeldMovement(&gObjectEvents[GetObjectEventIdByLocalIdAndMap(OBJ_EVENT_ID_PLAYER, 0, 0)]);
    UnfreezeObjectEvent(&gObjectEvents[GetObjectEventIdByLocalIdAndMap(OBJ_EVENT_ID_PLAYER, 0, 0)]);
    PlayerTurnInPlace(direction);
}

static void Task_HiddenItemNearby(u8 taskId)
{
    if (ObjectEventCheckHeldMovementStatus(&gObjectEvents[GetObjectEventIdByLocalIdAndMap(OBJ_EVENT_ID_PLAYER, 0, 0)]) == TRUE)
        DisplayItemMessageOnField(taskId, gText_ItemFinderNearby, Task_CloseItemfinderMessage);
}

static void Task_StandingOnHiddenItem(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (ObjectEventCheckHeldMovementStatus(&gObjectEvents[GetObjectEventIdByLocalIdAndMap(OBJ_EVENT_ID_PLAYER, 0, 0)]) == TRUE
    || tItemFound == FALSE)
    {
        // Spin player around on item
        PlayerFaceHiddenItem(sClockwiseDirections[tFacingDir]);
        tItemFound = TRUE;
        tFacingDir = (tFacingDir + 1) & 3;
        tCounter++;

        if (tCounter == 4)
            DisplayItemMessageOnField(taskId, gText_ItemFinderOnTop, Task_CloseItemfinderMessage);
    }
}

// Undefine itemfinder task data
#undef tItemDistanceX
#undef tItemDistanceY
#undef tItemFound
#undef tCounter
#undef tItemfinderBeeps
#undef tFacingDir

void ItemUseOutOfBattle_PokeblockCase(u8 taskId)
{
    static const u16 egglocke[][1] = {
        {SPECIES_BULBASAUR},
        {SPECIES_CHARMANDER},
        {SPECIES_SQUIRTLE},
        {SPECIES_CATERPIE},
        {SPECIES_WEEDLE},
        {SPECIES_PIDGEY},
        {SPECIES_RATTATA},
        {SPECIES_RATTATA_ALOLAN},
        {SPECIES_SPEAROW},
        {SPECIES_EKANS},
        {SPECIES_PICHU},
        {SPECIES_SANDSHREW},
        {SPECIES_SANDSHREW_ALOLAN},
        {SPECIES_NIDORAN_F},
        {SPECIES_NIDORAN_M},
        {SPECIES_CLEFFA},
        {SPECIES_VULPIX},
        {SPECIES_VULPIX_ALOLAN},
        {SPECIES_IGGLYBUFF},
        {SPECIES_ZUBAT},
        {SPECIES_ODDISH},
        {SPECIES_PARAS},
        {SPECIES_VENONAT},
        {SPECIES_DIGLETT},
        {SPECIES_DIGLETT_ALOLAN},
        {SPECIES_MEOWTH},
        {SPECIES_MEOWTH_ALOLAN},
        {SPECIES_PSYDUCK},
        {SPECIES_MANKEY},
        {SPECIES_GROWLITHE},
        {SPECIES_POLIWAG},
        {SPECIES_ABRA},
        {SPECIES_MACHOP},
        {SPECIES_BELLSPROUT},
        {SPECIES_TENTACOOL},
        {SPECIES_GEODUDE},
        {SPECIES_GEODUDE_ALOLAN},
        {SPECIES_PONYTA},
        {SPECIES_SLOWPOKE},
        {SPECIES_MAGNEMITE},
        {SPECIES_FARFETCHD},
        {SPECIES_DODUO},
        {SPECIES_SEEL},
        {SPECIES_GRIMER},
        {SPECIES_GRIMER_ALOLAN},
        {SPECIES_SHELLDER},
        {SPECIES_GASTLY},
        {SPECIES_ONIX},
        {SPECIES_DROWZEE},
        {SPECIES_KRABBY},
        {SPECIES_VOLTORB},
        {SPECIES_EXEGGCUTE},
        {SPECIES_CUBONE},
        {SPECIES_TYROGUE},
        {SPECIES_LICKITUNG},
        {SPECIES_KOFFING},
        {SPECIES_RHYHORN},
        {SPECIES_HAPPINY},
        {SPECIES_TANGELA},
        {SPECIES_KANGASKHAN},
        {SPECIES_HORSEA},
        {SPECIES_GOLDEEN},
        {SPECIES_STARYU},
        {SPECIES_MIME_JR},
        {SPECIES_SCYTHER},
        {SPECIES_SMOOCHUM},
        {SPECIES_ELEKID},
        {SPECIES_MAGBY},
        {SPECIES_PINSIR},
        {SPECIES_TAUROS},
        {SPECIES_MAGIKARP},
        {SPECIES_LAPRAS},
        {SPECIES_DITTO},
        {SPECIES_EEVEE},
        {SPECIES_PORYGON},
        {SPECIES_OMANYTE},
        {SPECIES_KABUTO},
        {SPECIES_AERODACTYL},
        {SPECIES_MUNCHLAX},
        {SPECIES_ARTICUNO},
        {SPECIES_ZAPDOS},
        {SPECIES_MOLTRES},
        {SPECIES_DRATINI},
        {SPECIES_MEWTWO},
        {SPECIES_MEW},
        {SPECIES_CHIKORITA},
        {SPECIES_CYNDAQUIL},
        {SPECIES_TOTODILE},
        {SPECIES_SENTRET},
        {SPECIES_HOOTHOOT},
        {SPECIES_LEDYBA},
        {SPECIES_SPINARAK},
        {SPECIES_CHINCHOU},
        {SPECIES_TOGEPI},
        {SPECIES_NATU},
        {SPECIES_MAREEP},
        {SPECIES_AZURILL},
        {SPECIES_BONSLY},
        {SPECIES_HOPPIP},
        {SPECIES_AIPOM},
        {SPECIES_SUNKERN},
        {SPECIES_YANMA},
        {SPECIES_WOOPER},
        {SPECIES_MURKROW},
        {SPECIES_MISDREAVUS},
        {SPECIES_WYNAUT},
        {SPECIES_GIRAFARIG},
        {SPECIES_PINECO},
        {SPECIES_DUNSPARCE},
        {SPECIES_GLIGAR},
        {SPECIES_SNUBBULL},
        {SPECIES_QWILFISH},
        {SPECIES_SHUCKLE},
        {SPECIES_HERACROSS},
        {SPECIES_SNEASEL},
        {SPECIES_TEDDIURSA},
        {SPECIES_SLUGMA},
        {SPECIES_SWINUB},
        {SPECIES_CORSOLA},
        {SPECIES_REMORAID},
        {SPECIES_DELIBIRD},
        {SPECIES_MANTYKE},
        {SPECIES_SKARMORY},
        {SPECIES_HOUNDOUR},
        {SPECIES_PHANPY},
        {SPECIES_STANTLER},
        {SPECIES_SMEARGLE},
        {SPECIES_MILTANK},
        {SPECIES_LARVITAR},
        {SPECIES_LUGIA},
        {SPECIES_HO_OH},
        {SPECIES_TREECKO},
        {SPECIES_TORCHIC},
        {SPECIES_MUDKIP},
        {SPECIES_POOCHYENA},
        {SPECIES_ZIGZAGOON},
        {SPECIES_WURMPLE},
        {SPECIES_LOTAD},
        {SPECIES_SEEDOT},
        {SPECIES_TAILLOW},
        {SPECIES_WINGULL},
        {SPECIES_RALTS},
        {SPECIES_SURSKIT},
        {SPECIES_SHROOMISH},
        {SPECIES_SLAKOTH},
        {SPECIES_NINCADA},
        {SPECIES_WHISMUR},
        {SPECIES_MAKUHITA},
        {SPECIES_NOSEPASS},
        {SPECIES_SKITTY},
        {SPECIES_SABLEYE},
        {SPECIES_MAWILE},
        {SPECIES_ARON},
        {SPECIES_MEDITITE},
        {SPECIES_ELECTRIKE},
        {SPECIES_PLUSLE},
        {SPECIES_MINUN},
        {SPECIES_VOLBEAT},
        {SPECIES_ILLUMISE},
        {SPECIES_BUDEW},
        {SPECIES_GULPIN},
        {SPECIES_CARVANHA},
        {SPECIES_WAILMER},
        {SPECIES_NUMEL},
        {SPECIES_TORKOAL},
        {SPECIES_SPOINK},
        {SPECIES_SPINDA},
        {SPECIES_TRAPINCH},
        {SPECIES_CACNEA},
        {SPECIES_SWABLU},
        {SPECIES_ZANGOOSE},
        {SPECIES_SEVIPER},
        {SPECIES_LUNATONE},
        {SPECIES_SOLROCK},
        {SPECIES_BARBOACH},
        {SPECIES_CORPHISH},
        {SPECIES_BALTOY},
        {SPECIES_LILEEP},
        {SPECIES_ANORITH},
        {SPECIES_FEEBAS},
        {SPECIES_CASTFORM},
        {SPECIES_CASTFORM_SUNNY},
        {SPECIES_CASTFORM_RAINY},
        {SPECIES_CASTFORM_SNOWY},
        {SPECIES_KECLEON},
        {SPECIES_SHUPPET},
        {SPECIES_DUSKULL},
        {SPECIES_TROPIUS},
        {SPECIES_CHINGLING},
        {SPECIES_ABSOL},
        {SPECIES_SNORUNT},
        {SPECIES_SPHEAL},
        {SPECIES_CLAMPERL},
        {SPECIES_RELICANTH},
        {SPECIES_LUVDISC},
        {SPECIES_BAGON},
        {SPECIES_BELDUM},
        {SPECIES_REGIROCK},
        {SPECIES_REGICE},
        {SPECIES_REGISTEEL},
        {SPECIES_LATIAS},
        {SPECIES_LATIOS},
        {SPECIES_KYOGRE},
        {SPECIES_GROUDON},
        {SPECIES_RAYQUAZA},
        {SPECIES_JIRACHI},
        {SPECIES_DEOXYS},
        {SPECIES_DEOXYS_ATTACK},
        {SPECIES_DEOXYS_DEFENSE},
        {SPECIES_DEOXYS_SPEED},
        {SPECIES_TURTWIG},
        {SPECIES_CHIMCHAR},
        {SPECIES_PIPLUP},
        {SPECIES_STARLY},
        {SPECIES_BIDOOF},
        {SPECIES_KRICKETOT},
        {SPECIES_SHINX},
        {SPECIES_CRANIDOS},
        {SPECIES_SHIELDON},
        {SPECIES_BURMY},
        {SPECIES_COMBEE},
        {SPECIES_PACHIRISU},
        {SPECIES_BUIZEL},
        {SPECIES_CHERUBI},
        {SPECIES_SHELLOS},
        {SPECIES_DRIFLOON},
        {SPECIES_BUNEARY},
        {SPECIES_GLAMEOW},
        {SPECIES_STUNKY},
        {SPECIES_BRONZOR},
        {SPECIES_CHATOT},
        {SPECIES_SPIRITOMB},
        {SPECIES_GIBLE},
        {SPECIES_RIOLU},
        {SPECIES_HIPPOPOTAS},
        {SPECIES_SKORUPI},
        {SPECIES_CROAGUNK},
        {SPECIES_CARNIVINE},
        {SPECIES_FINNEON},
        {SPECIES_SNOVER},
        {SPECIES_FROSLASS},
        {SPECIES_ROTOM},
        {SPECIES_ROTOM_HEAT},
        {SPECIES_ROTOM_WASH},
        {SPECIES_ROTOM_FROST},
        {SPECIES_ROTOM_FAN},
        {SPECIES_ROTOM_MOW},
        {SPECIES_HEATRAN},
        {SPECIES_REGIGIGAS},
        {SPECIES_SNIVY},
        {SPECIES_TEPIG},
        {SPECIES_OSHAWOTT},
        {SPECIES_PATRAT},
        {SPECIES_LILLIPUP},
        {SPECIES_PURRLOIN},
        {SPECIES_PANSAGE},
        {SPECIES_PANSEAR},
        {SPECIES_PANPOUR},
        {SPECIES_MUNNA},
        {SPECIES_PIDOVE},
        {SPECIES_BLITZLE},
        {SPECIES_ROGGENROLA},
        {SPECIES_WOOBAT},
        {SPECIES_DRILBUR},
        {SPECIES_AUDINO},
        {SPECIES_TIMBURR},
        {SPECIES_TYMPOLE},
        {SPECIES_THROH},
        {SPECIES_SAWK},
        {SPECIES_SEWADDLE},
        {SPECIES_VENIPEDE},
        {SPECIES_COTTONEE},
        {SPECIES_PETILIL},
        {SPECIES_BASCULIN},
        {SPECIES_SANDILE},
        {SPECIES_DARUMAKA},
        {SPECIES_MARACTUS},
        {SPECIES_DWEBBLE},
        {SPECIES_SCRAGGY},
        {SPECIES_SIGILYPH},
        {SPECIES_YAMASK},
        {SPECIES_TIRTOUGA},
        {SPECIES_ARCHEN},
        {SPECIES_TRUBBISH},
        {SPECIES_ZORUA},
        {SPECIES_MINCCINO},
        {SPECIES_GOTHITA},
        {SPECIES_SOLOSIS},
        {SPECIES_DUCKLETT},
        {SPECIES_VANILLITE},
        {SPECIES_DEERLING},
        {SPECIES_EMOLGA},
        {SPECIES_KARRABLAST},
        {SPECIES_FOONGUS},
        {SPECIES_FRILLISH},
        {SPECIES_ALOMOMOLA},
        {SPECIES_JOLTIK},
        {SPECIES_FERROSEED},
        {SPECIES_KLINK},
        {SPECIES_TYNAMO},
        {SPECIES_ELGYEM},
        {SPECIES_LITWICK},
        {SPECIES_AXEW},
        {SPECIES_CUBCHOO},
        {SPECIES_CRYOGONAL},
        {SPECIES_SHELMET},
        {SPECIES_STUNFISK},
        {SPECIES_MIENFOO},
        {SPECIES_DRUDDIGON},
        {SPECIES_GOLETT},
        {SPECIES_PAWNIARD},
        {SPECIES_BOUFFALANT},
        {SPECIES_RUFFLET},
        {SPECIES_VULLABY},
        {SPECIES_HEATMOR},
        {SPECIES_DURANT},
        {SPECIES_DEINO},
        {SPECIES_LARVESTA},
        {SPECIES_MELOETTA},
        {SPECIES_MELOETTA_PIROUETTE},
        {SPECIES_CHESPIN},
        {SPECIES_FENNEKIN},
        {SPECIES_FROAKIE},
        {SPECIES_BUNNELBY},
        {SPECIES_FLETCHLING},
        {SPECIES_SCATTERBUG},
        {SPECIES_LITLEO},
        {SPECIES_FLABEBE},
        {SPECIES_SKIDDO},
        {SPECIES_PANCHAM},
        {SPECIES_FURFROU},
        {SPECIES_ESPURR},
        {SPECIES_HONEDGE},
        {SPECIES_SPRITZEE},
        {SPECIES_SWIRLIX},
        {SPECIES_INKAY},
        {SPECIES_BINACLE},
        {SPECIES_SKRELP},
        {SPECIES_CLAUNCHER},
        {SPECIES_HELIOPTILE},
        {SPECIES_TYRUNT},
        {SPECIES_AMAURA},
        {SPECIES_HAWLUCHA},
        {SPECIES_DEDENNE},
        {SPECIES_CARBINK},
        {SPECIES_GOOMY},
        {SPECIES_KLEFKI},
        {SPECIES_PHANTUMP},
        {SPECIES_PUMPKABOO},
        {SPECIES_PUMPKABOO_SMALL},
        {SPECIES_PUMPKABOO_LARGE},
        {SPECIES_PUMPKABOO_SUPER},
        {SPECIES_BERGMITE},
        {SPECIES_NOIBAT},
        {SPECIES_DIANCIE},
        {SPECIES_ROWLET},
        {SPECIES_LITTEN},
        {SPECIES_POPPLIO},
        {SPECIES_PIKIPEK},
        {SPECIES_YUNGOOS},
        {SPECIES_GRUBBIN},
        {SPECIES_CRABRAWLER},
        {SPECIES_ORICORIO},
        {SPECIES_ORICORIO_POM_POM},
        {SPECIES_ORICORIO_PAU},
        {SPECIES_ORICORIO_SENSU},
        {SPECIES_CUTIEFLY},
        {SPECIES_ROCKRUFF},
        {SPECIES_WISHIWASHI},
        {SPECIES_WISHIWASHI_SCHOOL},
        {SPECIES_MAREANIE},
        {SPECIES_MUDBRAY},
        {SPECIES_DEWPIDER},
        {SPECIES_FOMANTIS},
        {SPECIES_MORELULL},
        {SPECIES_SALANDIT},
        {SPECIES_STUFFUL},
        {SPECIES_BOUNSWEET},
        {SPECIES_COMFEY},
        {SPECIES_ORANGURU},
        {SPECIES_PASSIMIAN},
        {SPECIES_WIMPOD},
        {SPECIES_SANDYGAST},
        {SPECIES_PYUKUMUKU},
        {SPECIES_MINIOR},
        {SPECIES_MINIOR_METEOR_ORANGE},
        {SPECIES_KOMALA},
        {SPECIES_TURTONATOR},
        {SPECIES_TOGEDEMARU},
        {SPECIES_MIMIKYU},
        {SPECIES_MIMIKYU_BUSTED},
        {SPECIES_BRUXISH},
        {SPECIES_DRAMPA},
        {SPECIES_DHELMISE},
        {SPECIES_JANGMO_O},
        {SPECIES_COSMOG},
        {SPECIES_MAGEARNA},
        {SPECIES_MELTAN}
    };

    u16 numEggSpecies = ARRAY_COUNT(egglocke);
    u16 randSpecies = Random() % numEggSpecies;

    struct Pokemon mon;
    u8 isEgg;
    u8 eggCycles;

    CreateEgg(&mon, randSpecies, TRUE);
    isEgg = TRUE;
    eggCycles = 0;
    SetMonData(&mon, MON_DATA_IS_EGG, &isEgg);
    SetMonData(&mon, MON_DATA_FRIENDSHIP, &eggCycles);


    GiveMonToPlayer(&mon);
    
    
    
    if (VarGet(VAR_POKE_VIAL_CHARGES) == 0)
    {
        if (!gTasks[taskId].tUsingRegisteredKeyItem)
        {
            DisplayItemMessage(taskId, 1, gText_PokeVialEmpty, CloseItemMessage);
        }
        else
        {
            DisplayItemMessageOnField(taskId, gText_PokeVialEmpty, Task_CloseCantUseKeyItemMessage);
        }
    }
    else
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_PokeblockCase;
        SetUpItemUseOnFieldCallback(taskId);
    }
    
    
    /*
    if (MenuHelpers_LinkSomething() == TRUE) // link func
    {
        DisplayDadsAdviceCannotUseItemMessage(taskId, gTasks[taskId].tUsingRegisteredKeyItem);
    }
    else if (gTasks[taskId].tUsingRegisteredKeyItem != TRUE)
    {
        gBagMenu->newScreenCallback = CB2_OpenPokeblockFromBag;
        Task_FadeAndCloseBagMenu(taskId);
    }
    else
    {
        gFieldCallback = FieldCB_ReturnToFieldNoScript;
        FadeScreen(FADE_TO_BLACK, 0);
        gTasks[taskId].func = Task_OpenRegisteredPokeblockCase;
    }
    */
}

static void ItemUseOnFieldCB_PokeblockCase(u8 taskId)
{
    PlaySE(SE_USE_ITEM);
    HealPlayerParty();
    VarSet(VAR_POKE_VIAL_CHARGES, VarGet(VAR_POKE_VIAL_CHARGES) - 1);
    DisplayItemMessageOnField(taskId, gText_UsedPokeVial, Task_CloseCantUseKeyItemMessage);
}




static void CB2_OpenPokeblockFromBag(void)
{
    OpenPokeblockCase(PBLOCK_CASE_FIELD, CB2_ReturnToBagMenuPocket);
}

static void Task_OpenRegisteredPokeblockCase(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        CleanupOverworldWindowsAndTilemaps();
        OpenPokeblockCase(PBLOCK_CASE_FIELD, CB2_ReturnToField);
        DestroyTask(taskId);
    }
}

void ItemUseOutOfBattle_CoinCase(u8 taskId)
{
    ConvertIntToDecimalStringN(gStringVar1, GetCoins(), STR_CONV_MODE_LEFT_ALIGN, 4);
    StringExpandPlaceholders(gStringVar4, gText_CoinCase);

    if (!gTasks[taskId].tUsingRegisteredKeyItem)
    {
        DisplayItemMessage(taskId, 1, gStringVar4, CloseItemMessage);
    }
    else
    {
        DisplayItemMessageOnField(taskId, gStringVar4, Task_CloseCantUseKeyItemMessage);
    }
}

void ItemUseOutOfBattle_PowderJar(u8 taskId)
{
    ConvertIntToDecimalStringN(gStringVar1, GetBerryPowder(), STR_CONV_MODE_LEFT_ALIGN, 5);
    StringExpandPlaceholders(gStringVar4, gText_PowderQty);

    if (!gTasks[taskId].tUsingRegisteredKeyItem)
    {
        DisplayItemMessage(taskId, 1, gStringVar4, CloseItemMessage);
    }
    else
    {
        DisplayItemMessageOnField(taskId, gStringVar4, Task_CloseCantUseKeyItemMessage);
    }
}

void ItemUseOutOfBattle_Berry(u8 taskId)
{
    if (IsPlayerFacingEmptyBerryTreePatch() == TRUE)
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_Berry;
        gFieldCallback = FieldCB_UseItemOnField;
        gBagMenu->newScreenCallback = CB2_ReturnToField;
        Task_FadeAndCloseBagMenu(taskId);
    }
    else
    {
        ItemId_GetFieldFunc(gSpecialVar_ItemId)(taskId);
    }
}

static void ItemUseOnFieldCB_Berry(u8 taskId)
{
    RemoveBagItem(gSpecialVar_ItemId, 1);
    ScriptContext2_Enable();
    ScriptContext1_SetupScript(BerryTree_EventScript_ItemUsePlantBerry);
    DestroyTask(taskId);
}

void ItemUseOutOfBattle_WailmerPail(u8 taskId)
{
    if (TryToWaterSudowoodo() == TRUE)
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_WailmerPailSudowoodo;
        SetUpItemUseOnFieldCallback(taskId);
    }
    else if (TryToWaterBerryTree() == TRUE)
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_WailmerPailBerry;
        SetUpItemUseOnFieldCallback(taskId);
    }
    else
    {
        DisplayDadsAdviceCannotUseItemMessage(taskId, gTasks[taskId].tUsingRegisteredKeyItem);
    }
}

static void ItemUseOnFieldCB_WailmerPailBerry(u8 taskId)
{
    ScriptContext2_Enable();
    ScriptContext1_SetupScript(BerryTree_EventScript_ItemUseWailmerPail);
    DestroyTask(taskId);
}

static bool8 TryToWaterSudowoodo(void)
{
    u16 x, y;
    u8 z;
    u8 objId;
    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    z = PlayerGetZCoord();
    objId = GetObjectEventIdByXYZ(x, y, z);
    if (objId == OBJECT_EVENTS_COUNT || gObjectEvents[objId].graphicsId != OBJ_EVENT_GFX_SUDOWOODO)
        return FALSE;
    else
        return TRUE;
}

static void ItemUseOnFieldCB_WailmerPailSudowoodo(u8 taskId)
{
    ScriptContext2_Enable();
    ScriptContext1_SetupScript(BattleFrontier_OutsideEast_EventScript_WaterSudowoodo);
    DestroyTask(taskId);
}

void ItemUseOutOfBattle_Medicine(u8 taskId)
{
    gItemUseCB = ItemUseCB_Medicine;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_AbilityCapsule(u8 taskId)
{
    gItemUseCB = ItemUseCB_AbilityCapsule;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_ReduceEV(u8 taskId)
{
    gItemUseCB = ItemUseCB_ReduceEV;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_SacredAsh(u8 taskId)
{
    gItemUseCB = ItemUseCB_SacredAsh;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_PPRecovery(u8 taskId)
{
    gItemUseCB = ItemUseCB_PPRecovery;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_PPUp(u8 taskId)
{
    gItemUseCB = ItemUseCB_PPUp;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_RareCandy(u8 taskId)
{
    gItemUseCB = ItemUseCB_RareCandy;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_TMHM(u8 taskId)
{
    if (gSpecialVar_ItemId >= ITEM_HM01_CUT)
        DisplayItemMessage(taskId, 1, gText_BootedUpHM, BootUpSoundTMHM); // HM
    else
        DisplayItemMessage(taskId, 1, gText_BootedUpTM, BootUpSoundTMHM); // TM
}

static void BootUpSoundTMHM(u8 taskId)
{
    PlaySE(SE_PC_LOGIN);
    gTasks[taskId].func = Task_ShowTMHMContainedMessage;
}

static void Task_ShowTMHMContainedMessage(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        StringCopy(gStringVar1, gMoveNamesLong[ItemIdToBattleMoveId(gSpecialVar_ItemId)]);
        StringExpandPlaceholders(gStringVar4, gText_TMHMContainedVar1);
        DisplayItemMessage(taskId, 1, gStringVar4, UseTMHMYesNo);
    }
}

static void UseTMHMYesNo(u8 taskId)
{
    BagMenu_YesNo(taskId, ITEMWIN_YESNO_HIGH, &sUseTMHMYesNoFuncTable);
}

static void UseTMHM(u8 taskId)
{
    gItemUseCB = ItemUseCB_TMHM;
    SetUpItemUseCallback(taskId);
}

static void RemoveUsedItem(void)
{
    RemoveBagItem(gSpecialVar_ItemId, 1);
    CopyItemName(gSpecialVar_ItemId, gStringVar2);
    StringExpandPlaceholders(gStringVar4, gText_PlayerUsedVar2);
    if (!InBattlePyramid())
    {
        UpdatePocketItemList(ItemId_GetPocket(gSpecialVar_ItemId));
        UpdatePocketListPosition(ItemId_GetPocket(gSpecialVar_ItemId));
    }
    else
    {
        UpdatePyramidBagList();
        UpdatePyramidBagCursorPos();
    }
}

void ItemUseOutOfBattle_Repel(u8 taskId)
{
    if (VarGet(VAR_REPEL_STEP_COUNT) == 0)
        gTasks[taskId].func = Task_StartUseRepel;
    else if (!InBattlePyramid())
        DisplayItemMessage(taskId, 1, gText_RepelEffectsLingered, CloseItemMessage);
    else
        DisplayItemMessageInBattlePyramid(taskId, gText_RepelEffectsLingered, Task_CloseBattlePyramidBagMessage);
}

static void Task_StartUseRepel(u8 taskId)
{
    s16* data = gTasks[taskId].data;

    if (++data[8] > 7)
    {
        data[8] = 0;
        PlaySE(SE_REPEL);
        gTasks[taskId].func = Task_UseRepel;
    }
}

static void Task_UseRepel(u8 taskId)
{
    if (!IsSEPlaying())
    {
        VarSet(VAR_REPEL_STEP_COUNT, ItemId_GetHoldEffectParam(gSpecialVar_ItemId));
        RemoveUsedItem();
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, gStringVar4, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, gStringVar4, Task_CloseBattlePyramidBagMessage);
    }
}

static void Task_UsedBlackWhiteFlute(u8 taskId)
{
    if(++gTasks[taskId].data[8] > 7)
    {
        PlaySE(SE_GLASS_FLUTE);
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, gStringVar4, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, gStringVar4, Task_CloseBattlePyramidBagMessage);
    }
}

void ItemUseOutOfBattle_BlackWhiteFlute(u8 taskId)
{
    CopyItemName(gSpecialVar_ItemId, gStringVar2);
    if (gSpecialVar_ItemId == ITEM_WHITE_FLUTE)
    {
        FlagSet(FLAG_SYS_ENC_UP_ITEM);
        FlagClear(FLAG_SYS_ENC_DOWN_ITEM);
        StringExpandPlaceholders(gStringVar4, gText_UsedVar2WildLured);
    }
    else
    {
        FlagSet(FLAG_SYS_ENC_DOWN_ITEM);
        FlagClear(FLAG_SYS_ENC_UP_ITEM);
        StringExpandPlaceholders(gStringVar4, gText_UsedVar2WildRepelled);
    }
    gTasks[taskId].data[8] = 0;
    gTasks[taskId].func = Task_UsedBlackWhiteFlute;
}

void Task_UseDigEscapeRopeOnField(u8 taskId)
{
    ResetInitialPlayerAvatarState();
    StartEscapeRopeFieldEffect();
    DestroyTask(taskId);
}

static void ItemUseOnFieldCB_EscapeRope(u8 taskId)
{
    Overworld_ResetStateAfterDigEscRope();
    #if I_KEY_ESCAPE_ROPE < GEN_8
        RemoveUsedItem();
    #endif
    gTasks[taskId].data[0] = 0;
    DisplayItemMessageOnField(taskId, gStringVar4, Task_UseDigEscapeRopeOnField);
}

bool8 CanUseDigOrEscapeRopeOnCurMap(void)
{
    if (gMapHeader.allowEscaping)
        return TRUE;
    else
        return FALSE;
}

void ItemUseOutOfBattle_EscapeRope(u8 taskId)
{
    if (CanUseDigOrEscapeRopeOnCurMap() == TRUE)
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_EscapeRope;
        SetUpItemUseOnFieldCallback(taskId);
    }
    else
    {
        DisplayDadsAdviceCannotUseItemMessage(taskId, gTasks[taskId].tUsingRegisteredKeyItem);
    }
}

void ItemUseOutOfBattle_EvolutionStone(u8 taskId)
{
    gItemUseCB = ItemUseCB_EvolutionStone;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_Nectar(u8 taskId)
{
    gItemUseCB = ItemUseCB_Nectar;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_PokeVial(u8 taskId)
{
    if (VarGet(VAR_POKE_VIAL_CHARGES) == 0)
    {
        if (!gTasks[taskId].tUsingRegisteredKeyItem)
        {
            DisplayItemMessage(taskId, 1, gText_PokeVialEmpty, CloseItemMessage);
        }
        else
        {
            DisplayItemMessageOnField(taskId, gText_PokeVialEmpty, Task_CloseCantUseKeyItemMessage);
        }
    }
    else
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_PokeVial;
        SetUpItemUseOnFieldCallback(taskId);
    }
}

static void ItemUseOnFieldCB_PokeVial(u8 taskId)
{
    PlaySE(SE_USE_ITEM);
    HealPlayerParty();
    VarSet(VAR_POKE_VIAL_CHARGES, VarGet(VAR_POKE_VIAL_CHARGES) - 1);
    DisplayItemMessageOnField(taskId, gText_UsedPokeVial, Task_CloseCantUseKeyItemMessage);
}

void ItemUseOutOfBattle_Honey(u8 taskId)
{
    s16 x, y;
    u16 headerId = GetCurrentMapWildMonHeaderId();;

    PlayerGetDestCoords(&x, &y);

    if (MetatileBehavior_IsLandWildEncounter(MapGridGetMetatileBehaviorAt(x, y)) == TRUE // Player is on land encounter tile
        && headerId != 0xFFFF // Map has wild Pokemon 
        && gWildMonHeaders[headerId].honeyMonsInfo != NULL) // Map has honey encounters
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_Honey;
    }
    else // Honey fails to start encounter
    {
        sItemUseOnFieldCB = ItemUseOnFieldCB_HoneyFail;
    }
    gFieldCallback = FieldCB_UseItemOnField;
    gBagMenu->newScreenCallback = CB2_ReturnToField;
    Task_FadeAndCloseBagMenu(taskId);
}

static void ItemUseOnFieldCB_Honey(u8 taskId)
{
    RemoveBagItem(gSpecialVar_ItemId, 1);
    ScriptContext2_Enable();
    ScriptContext1_SetupScript(EventScript_HoneyEncounter);
    DestroyTask(taskId);
}

static void ItemUseOnFieldCB_HoneyFail(u8 taskId)
{
    RemoveBagItem(gSpecialVar_ItemId, 1);
    ScriptContext2_Enable();
    ScriptContext1_SetupScript(EventScript_FailSweetScent);
    DestroyTask(taskId);
}

u32 CanThrowBall(void)
{
    if (IsBattlerAlive(GetBattlerAtPosition(B_POSITION_OPPONENT_LEFT))
        && IsBattlerAlive(GetBattlerAtPosition(B_POSITION_OPPONENT_RIGHT))) 
    {
        return 1;   // There are two present pokemon.
    }
    else if (IsPlayerPartyAndPokemonStorageFull() == TRUE)
    {
        return 2;   // No room for mon
    }
    #if B_SEMI_INVULNERABLE_CATCH >= GEN_4
    else if (gStatuses3[GetCatchingBattler()] & STATUS3_SEMI_INVULNERABLE)
    {
        return 3;   // in semi-invulnerable state
    }
    #endif
    
    return 0;   // usable 
}

static const u8 sText_CantThrowPokeBall_TwoMons[] = _("Cannot throw a ball!\nThere are two Pokémon out there!\p");
static const u8 sText_CantThrowPokeBall_SemiInvulnerable[] = _("Cannot throw a ball!\nThere's no Pokémon in sight!\p");
void ItemUseInBattle_PokeBall(u8 taskId)
{
    switch (CanThrowBall())
    {
    case 0: // usable
    default:
        RemoveBagItem(gSpecialVar_ItemId, 1);
        if (!InBattlePyramid())
            Task_FadeAndCloseBagMenu(taskId);
        else
            CloseBattlePyramidBag(taskId);
        break;
    case 1:  // There are two present pokemon.
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, sText_CantThrowPokeBall_TwoMons, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, sText_CantThrowPokeBall_TwoMons, Task_CloseBattlePyramidBagMessage);
        break;
    case 2: // No room for mon
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, gText_BoxFull, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, gText_BoxFull, Task_CloseBattlePyramidBagMessage);
        break;
    #if B_SEMI_INVULNERABLE_CATCH >= GEN_4
    case 3: // Semi-Invulnerable
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, sText_CantThrowPokeBall_SemiInvulnerable, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, sText_CantThrowPokeBall_SemiInvulnerable, Task_CloseBattlePyramidBagMessage);
        break;
    #endif
    }
}

static void Task_CloseStatIncreaseMessage(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        if (!InBattlePyramid())
            Task_FadeAndCloseBagMenu(taskId);
        else
            CloseBattlePyramidBag(taskId);
    }
}

static void Task_UseStatIncreaseItem(u8 taskId)
{
    if(++gTasks[taskId].data[8] > 7)
    {
        PlaySE(SE_USE_ITEM);
        RemoveBagItem(gSpecialVar_ItemId, 1);
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, UseStatIncreaseItem(gSpecialVar_ItemId), Task_CloseStatIncreaseMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, UseStatIncreaseItem(gSpecialVar_ItemId), Task_CloseStatIncreaseMessage);
    }
}

// e.g. X Attack, Guard Spec
void ItemUseInBattle_StatIncrease(u8 taskId)
{
    u16 partyId = gBattlerPartyIndexes[gBattlerInMenuId];

    if (ExecuteTableBasedItemEffect(&gPlayerParty[partyId], gSpecialVar_ItemId, partyId, 0) != FALSE)
    {
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, gText_WontHaveEffect, CloseItemMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, gText_WontHaveEffect, Task_CloseBattlePyramidBagMessage);
    }
    else
    {
        gTasks[taskId].func = Task_UseStatIncreaseItem;
        gTasks[taskId].data[8] = 0;
    }
}

static void ItemUseInBattle_ShowPartyMenu(u8 taskId)
{
    if (!InBattlePyramid())
    {
        gBagMenu->newScreenCallback = ChooseMonForInBattleItem;
        Task_FadeAndCloseBagMenu(taskId);
    }
    else
    {
        gPyramidBagMenu->newScreenCallback = ChooseMonForInBattleItem;
        CloseBattlePyramidBag(taskId);
    }
}

void ItemUseInBattle_Medicine(u8 taskId)
{
    gItemUseCB = ItemUseCB_Medicine;
    ItemUseInBattle_ShowPartyMenu(taskId);
}

// Unused. Sacred Ash cannot be used in battle
void ItemUseInBattle_SacredAsh(u8 taskId)
{
    gItemUseCB = ItemUseCB_SacredAsh;
    ItemUseInBattle_ShowPartyMenu(taskId);
}

void ItemUseInBattle_PPRecovery(u8 taskId)
{
    gItemUseCB = ItemUseCB_PPRecovery;
    ItemUseInBattle_ShowPartyMenu(taskId);
}

// Fluffy Tail / Poke Doll
void ItemUseInBattle_Escape(u8 taskId)
{

    if((gBattleTypeFlags & BATTLE_TYPE_TRAINER) == FALSE)
    {
        RemoveUsedItem();
        if (!InBattlePyramid())
            DisplayItemMessage(taskId, 1, gStringVar4, Task_FadeAndCloseBagMenu);
        else
            DisplayItemMessageInBattlePyramid(taskId, gStringVar4, CloseBattlePyramidBag);
    }
    else
    {
        DisplayDadsAdviceCannotUseItemMessage(taskId, gTasks[taskId].tUsingRegisteredKeyItem);
    }
}

void ItemUseOutOfBattle_EnigmaBerry(u8 taskId)
{
    switch (GetItemEffectType(gSpecialVar_ItemId))
    {
    case ITEM_EFFECT_HEAL_HP:
    case ITEM_EFFECT_CURE_POISON:
    case ITEM_EFFECT_CURE_SLEEP:
    case ITEM_EFFECT_CURE_BURN:
    case ITEM_EFFECT_CURE_FREEZE:
    case ITEM_EFFECT_CURE_PARALYSIS:
    case ITEM_EFFECT_CURE_ALL_STATUS:
    case ITEM_EFFECT_ATK_EV:
    case ITEM_EFFECT_HP_EV:
    case ITEM_EFFECT_SPATK_EV:
    case ITEM_EFFECT_SPDEF_EV:
    case ITEM_EFFECT_SPEED_EV:
    case ITEM_EFFECT_DEF_EV:
        gTasks[taskId].tEnigmaBerryType = ITEM_USE_PARTY_MENU;
        ItemUseOutOfBattle_Medicine(taskId);
        break;
    case ITEM_EFFECT_SACRED_ASH:
        gTasks[taskId].tEnigmaBerryType = ITEM_USE_PARTY_MENU;
        ItemUseOutOfBattle_SacredAsh(taskId);
        break;
    case ITEM_EFFECT_RAISE_LEVEL:
        gTasks[taskId].tEnigmaBerryType = ITEM_USE_PARTY_MENU;
        ItemUseOutOfBattle_RareCandy(taskId);
        break;
    case ITEM_EFFECT_PP_UP:
    case ITEM_EFFECT_PP_MAX:
        gTasks[taskId].tEnigmaBerryType = ITEM_USE_PARTY_MENU;
        ItemUseOutOfBattle_PPUp(taskId);
        break;
    case ITEM_EFFECT_HEAL_PP:
        gTasks[taskId].tEnigmaBerryType = ITEM_USE_PARTY_MENU;
        ItemUseOutOfBattle_PPRecovery(taskId);
        break;
    default:
        gTasks[taskId].tEnigmaBerryType = ITEM_USE_BAG_MENU;
        ItemUseOutOfBattle_CannotUse(taskId);
        break;
    }
}

void ItemUseInBattle_EnigmaBerry(u8 taskId)
{
    switch (GetItemEffectType(gSpecialVar_ItemId))
    {
    case ITEM_EFFECT_X_ITEM:
        ItemUseInBattle_StatIncrease(taskId);
        break;
    case ITEM_EFFECT_HEAL_HP:
    case ITEM_EFFECT_CURE_POISON:
    case ITEM_EFFECT_CURE_SLEEP:
    case ITEM_EFFECT_CURE_BURN:
    case ITEM_EFFECT_CURE_FREEZE:
    case ITEM_EFFECT_CURE_PARALYSIS:
    case ITEM_EFFECT_CURE_ALL_STATUS:
    case ITEM_EFFECT_CURE_CONFUSION:
    case ITEM_EFFECT_CURE_INFATUATION:
        ItemUseInBattle_Medicine(taskId);
        break;
    case ITEM_EFFECT_HEAL_PP:
        ItemUseInBattle_PPRecovery(taskId);
        break;
    default:
        ItemUseOutOfBattle_CannotUse(taskId);
        break;
    }
}

void ItemUseOutOfBattle_FormChange(u8 taskId) 
{
    gItemUseCB = ItemUseCB_FormChange;
    gTasks[taskId].data[0] = FALSE;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_FormChange_ConsumedOnUse(u8 taskId)
{
    gItemUseCB = ItemUseCB_FormChange_ConsumedOnUse;
    gTasks[taskId].data[0] = TRUE;
    SetUpItemUseCallback(taskId);
}

void ItemUseOutOfBattle_CannotUse(u8 taskId)
{
    DisplayDadsAdviceCannotUseItemMessage(taskId, gTasks[taskId].tUsingRegisteredKeyItem);
}

#undef tUsingRegisteredKeyItem
