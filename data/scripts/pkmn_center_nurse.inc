Common_EventScript_PkmnCenterNurse::
    lock
    faceplayer
    checkitem ITEM_POKE_VIAL, 1
    compare VAR_RESULT, 1
    goto_if_eq EventScript_PkmnCenterNurseHasVial
    msgbox gText_AreYouARookieTrainer, MSGBOX_DEFAULT
    giveitem ITEM_POKE_VIAL, 1
    setvar VAR_POKE_VIAL_MAX_CHARGES, 1
    setvar VAR_POKE_VIAL_CHARGES, 1
    msgbox gText_ExplainPokeVial, MSGBOX_YESNO
    compare VAR_RESULT, YES
    goto_if_eq EventScript_PkmnCenterNurse_HealPkmn
    compare VAR_RESULT, NO
    goto_if_eq EventScript_PkmnCenterNurse_Goodbye
    end

EventScript_PkmnCenterNurseHasVial::
    call_if_set FLAG_RECEIVED_OLD_ROD, Give_Pokeblock_Case
    checkitem ITEM_RARE_CANDY, 400
    compare VAR_RESULT, TRUE
    call_if_ne ADDITEM_RARE_CANDY
    setvar VAR_0x8006, 4
    special GetSettingsMonStats
    compare VAR_0x8005, 1
    call_if_eq Give_Nuzlocke_Items
    call EventScript_PkmnCenterNurse_HealPkmn
    end

EventScript_PkmnCenterNurse_Goodbye::
    message gText_WeHopeToSeeYouAgain
    return

@ VAR_0x8004 is 1 when player has Gold Card; jumps are identical
    EventScript_PkmnCenterNurse_HealPkmn::
    copyvar VAR_POKE_VIAL_CHARGES, VAR_POKE_VIAL_MAX_CHARGES @ refill Poke Vial
    incrementgamestat GAME_STAT_USED_POKECENTER
    call EventScript_PkmnCenterNurse_TakeAndHealPkmn
    goto EventScript_PkmnCenterNurse_CheckTrainerHillAndUnionRoom
    end

EventScript_PkmnCenterNurse_IllTakeYourPkmn::
    message gText_IllTakeYourPkmn
    return

EventScript_PkmnCenterNurse_IllTakeYourPkmn2::
    message gText_IllTakeYourPkmn2
    return

EventScript_PkmnCenterNurse_TakeAndHealPkmn::
    applymovement VAR_0x800B, Common_Movement_FaceLeft
    waitmovement 0
    dofieldeffect FLDEFF_POKECENTER_HEAL
    waitfieldeffect FLDEFF_POKECENTER_HEAL
    applymovement VAR_0x800B, Common_Movement_FaceDown
    waitmovement 0
    special HealPlayerParty
    return

EventScript_PkmnCenterNurse_CheckTrainerHillAndUnionRoom::
    specialvar VAR_RESULT, PlayerNotAtTrainerHillEntrance
    compare VAR_RESULT, 0
    goto_if_eq EventScript_PkmnCenterNurse_ReturnPkmn
    specialvar VAR_RESULT, BufferUnionRoomPlayerName
    copyvar VAR_0x8008, VAR_RESULT
    compare VAR_0x8008, 0
    goto_if_eq EventScript_PkmnCenterNurse_ReturnPkmn
    compare VAR_0x8008, 1
    goto_if_eq EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom
    end

@ VAR_0x8004 is 1 when player has Gold Card
EventScript_PkmnCenterNurse_ReturnPkmn::
    applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
    waitmovement 0
    return

EventScript_PkmnCenterNurse_ReturnPkmn2::
    message gText_ThankYouForWaiting
    waitmessage
    applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
    waitmovement 0
    message gText_WeHopeToSeeYouAgain2
    return

EventScript_PkmnCenterNurse_PlayerWaitingInUnionRoom::
    goto_if_set FLAG_NURSE_UNION_ROOM_REMINDER, EventScript_PkmnCenterNurse_ReturnPkmn
    msgbox gText_RestoredPkmnToFullHealth, MSGBOX_DEFAULT
    setflag FLAG_NURSE_UNION_ROOM_REMINDER
    message CableClub_Text_PlayerIsWaiting
    waitmessage
    applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
    waitmovement 0
    message gText_WeHopeToSeeYouAgain
    return

EventScript_PkmnCenterNurse_CheckPokerus::
    specialvar VAR_RESULT, IsPokerusInParty
    compare VAR_RESULT, TRUE
    goto_if_eq EventScript_PkmnCenterNurse_ExplainPokerus
    compare VAR_RESULT, FALSE
    goto_if_eq EventScript_PkmnCenterNurse_CheckTrainerHillAndUnionRoom
    end

EventScript_PkmnCenterNurse_ExplainPokerus::
    message gText_PokerusExplanation
    setflag FLAG_POKERUS_EXPLAINED
    return

EventScript_PkmnCenterNurse_GoldCard::
    goto_if_set FLAG_NURSE_MENTIONS_GOLD_CARD, EventScript_PkmnCenterNurse_AskForUsual
    setflag FLAG_NURSE_MENTIONS_GOLD_CARD
    msgbox gText_WelcomeCutShort, MSGBOX_DEFAULT
    playse SE_PIN
    applymovement VAR_0x800B, Common_Movement_ExclamationMark
    waitmovement 0
    applymovement VAR_0x800B, Common_Movement_Delay48
    waitmovement 0
    msgbox gText_NoticesGoldCard, MSGBOX_YESNO
    compare VAR_RESULT, YES
    goto_if_eq EventScript_PkmnCenterNurse_GoldCardHealPkmn
    message gText_WeHopeToSeeYouAgain2
    return

EventScript_PkmnCenterNurse_AskForUsual::
    msgbox gText_YouWantTheUsual, MSGBOX_YESNO
    compare VAR_RESULT, YES
    goto_if_eq EventScript_PkmnCenterNurse_GoldCardHealPkmn
    message gText_WeHopeToSeeYouAgain2
    return

EventScript_PkmnCenterNurse_GoldCardHealPkmn::
    setvar VAR_0x8004, 1
    goto EventScript_PkmnCenterNurse_HealPkmn
    end

Movement_PkmnCenterNurse_Bow:
    nurse_joy_bow
    delay_4
    step_end
    
    
Give_Pokeblock_Case:
	checkitem ITEM_POKEBLOCK_CASE, 1
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_POKEBLOCK_CASE
	return
	
	
Give_Nuzlocke_Items:
	call_if_set FLAG_BADGE01_GET, Give_Cut_Items_After_Roxanne
    	call_if_set FLAG_RECEIVED_TM46, Give_Thief_Items_After_Thief
    	call_if_set FLAG_HIDE_SLATEPORT_CITY_OCEANIC_MUSEUM_AQUA_GRUNTS, Give_Thief_Items_After_Museum
    	call_if_set FLAG_BADGE03_GET, Give_Thief_Items_After_Wattson
    	call_if_set FLAG_BADGE04_GET, Give_Thief_Items_After_Flannery
    	call_if_set FLAG_BADGE05_GET, Give_Thief_Items_After_Norman
    	call_if_set FLAG_BADGE06_GET, Give_Thief_Items_After_Winoma
	checkitem ITEM_BIG_PEARL, 400
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BIG_PEARL
	checkitem ITEM_MAX_REPEL, 250
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_MAX_REPEL
	return

Give_Cut_Items_After_Roxanne:
	checkitem ITEM_MIRACLE_SEED, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_MIRACLE_SEED
	checkitem ITEM_CHARCOAL, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_CHARCOAL
	checkitem ITEM_LEEK, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_LEEK
	checkitem ITEM_BIG_ROOT, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BIG_ROOT
	checkitem ITEM_MENTAL_HERB, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_MENTAL_HERB
	checkitem ITEM_POWER_HERB, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_POWER_HERB
	checkitem ITEM_WHITE_HERB, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_WHITE_HERB
	checkitem ITEM_ELECTRIC_SEED, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_ELECTRIC_SEED
	checkitem ITEM_PSYCHIC_SEED, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_PSYCHIC_SEED
	checkitem ITEM_GRASSY_SEED, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_GRASSY_SEED
	checkitem ITEM_MISTY_SEED, 100 
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_MISTY_SEED
	checkitem ITEM_CHERI_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_CHERI_BERRY
	checkitem ITEM_CHESTO_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_CHESTO_BERRY
	checkitem ITEM_PECHA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_PECHA_BERRY
	checkitem ITEM_RAWST_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_RAWST_BERRY
	checkitem ITEM_PERSIM_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_PERSIM_BERRY
	checkitem ITEM_OCCA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_OCCA_BERRY
	checkitem ITEM_PASSHO_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_PASSHO_BERRY
	checkitem ITEM_WACAN_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_WACAN_BERRY
	checkitem ITEM_RINDO_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_RINDO_BERRY
	checkitem ITEM_YACHE_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_YACHE_BERRY
	checkitem ITEM_CHOPLE_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_CHOPLE_BERRY
	checkitem ITEM_KEBIA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_KEBIA_BERRY
	checkitem ITEM_SHUCA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SHUCA_BERRY
	checkitem ITEM_COBA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_COBA_BERRY
	checkitem ITEM_PAYAPA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_PAYAPA_BERRY
	checkitem ITEM_TANGA_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_TANGA_BERRY
	checkitem ITEM_CHARTI_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_CHARTI_BERRY
	checkitem ITEM_KASIB_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_KASIB_BERRY
	checkitem ITEM_HABAN_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_HABAN_BERRY
	checkitem ITEM_COLBUR_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_COLBUR_BERRY
	checkitem ITEM_BABIRI_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BABIRI_BERRY
	checkitem ITEM_CHILAN_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_CHILAN_BERRY
	checkitem ITEM_ROSELI_BERRY, 100
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_ROSELI_BERRY
	return
	
	
Give_Thief_Items_After_Thief:
	checkitem ITEM_SITRUS_BERRY, 400
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SITRUS_BERRY
	checkitem ITEM_LEFTOVERS, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_LEFTOVERS
	checkitem ITEM_BRIGHT_POWDER, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BRIGHT_POWDER
	checkitem ITEM_HARD_STONE, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_HARD_STONE
	checkitem ITEM_MYSTIC_WATER, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_MYSTIC_WATER
	checkitem ITEM_SILVER_POWDER, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SILVER_POWDER
	checkitem ITEM_POISON_BARB, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_POISON_BARB
	checkitem ITEM_SOFT_SAND, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SOFT_SAND
	checkitem ITEM_BLACK_BELT, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BLACK_BELT
	return


Give_Thief_Items_After_Museum:
	checkitem ITEM_SILK_SCARF, 400
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SILK_SCARF
	checkitem ITEM_SHARP_BEAK, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SHARP_BEAK
	checkitem ITEM_NEVER_MELT_ICE, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_NEVER_MELT_ICE
	checkitem ITEM_TWISTED_SPOON, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_TWISTED_SPOON
	return

	
Give_Thief_Items_After_Wattson:
	checkitem ITEM_MAGNET, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_MAGNET
	setvar VAR_0x8006, 0
    	special GetSettingsMonStats
    	compare VAR_0x8005, 1
    	call_if_eq Give_Bottle_Cap	
	return


Give_Thief_Items_After_Flannery:
	checkitem ITEM_BLACK_GLASSES, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BLACK_GLASSES
	checkitem ITEM_SPELL_TAG, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_SPELL_TAG
	return

Give_Thief_Items_After_Norman:
	checkitem ITEM_DRAGON_FANG, 10
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_DRAGON_FANG
	return

Give_Thief_Items_After_Winoma:
	checkitem ITEM_PP_MAX, 50
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_PP_MAX
	return

Give_Bottle_Cap:
	checkitem ITEM_BOTTLE_CAP, 50
	compare VAR_RESULT, TRUE
	call_if_ne ADDITEM_BOTTLE_CAP
	return

ADDITEM_POKEBLOCK_CASE:
	additem ITEM_POKEBLOCK_CASE, 1
	return

ADDITEM_RARE_CANDY:
	additem ITEM_RARE_CANDY, 400
	return
	
ADDITEM_BIG_PEARL:
	additem ITEM_BIG_PEARL, 400
	return
	
ADDITEM_MAX_REPEL:
	additem ITEM_MAX_REPEL, 250
	return

ADDITEM_MIRACLE_SEED:
	additem ITEM_MIRACLE_SEED, 10
	return

ADDITEM_CHARCOAL:
	additem ITEM_CHARCOAL, 10
	return

ADDITEM_LEEK:
	additem ITEM_LEEK, 10
	return

ADDITEM_BIG_ROOT:
	additem ITEM_BIG_ROOT, 10
	return

ADDITEM_MENTAL_HERB:
	additem ITEM_MENTAL_HERB, 100
	return

ADDITEM_POWER_HERB:
	additem ITEM_POWER_HERB, 100
	return

ADDITEM_WHITE_HERB:
	additem ITEM_WHITE_HERB, 100
	return

ADDITEM_ELECTRIC_SEED:
	additem ITEM_ELECTRIC_SEED, 100
	return

ADDITEM_PSYCHIC_SEED:
	additem ITEM_PSYCHIC_SEED, 100
	return

ADDITEM_GRASSY_SEED:
	additem ITEM_GRASSY_SEED, 100
	return

ADDITEM_MISTY_SEED:
	additem ITEM_MISTY_SEED, 100
	return

ADDITEM_CHERI_BERRY:
	additem ITEM_CHERI_BERRY, 100
	return

ADDITEM_CHESTO_BERRY:
	additem ITEM_CHESTO_BERRY, 100
	return

ADDITEM_PECHA_BERRY:
	additem ITEM_PECHA_BERRY, 100
	return

ADDITEM_RAWST_BERRY:
	additem ITEM_RAWST_BERRY, 100
	return

ADDITEM_PERSIM_BERRY:
	additem ITEM_PERSIM_BERRY, 100
	return

ADDITEM_OCCA_BERRY:
	additem ITEM_OCCA_BERRY, 100
	return

ADDITEM_PASSHO_BERRY:
	additem ITEM_PASSHO_BERRY, 100
	return

ADDITEM_WACAN_BERRY:
	additem ITEM_WACAN_BERRY, 100
	return

ADDITEM_RINDO_BERRY:
	additem ITEM_RINDO_BERRY, 100
	return

ADDITEM_YACHE_BERRY:
	additem ITEM_YACHE_BERRY, 100
	return

ADDITEM_CHOPLE_BERRY:
	additem ITEM_CHOPLE_BERRY, 100
	return

ADDITEM_KEBIA_BERRY:
	additem ITEM_KEBIA_BERRY, 100
	return

ADDITEM_SHUCA_BERRY:
	additem ITEM_SHUCA_BERRY, 100
	return

ADDITEM_COBA_BERRY:
	additem ITEM_COBA_BERRY, 100
	return

ADDITEM_PAYAPA_BERRY:
	additem ITEM_PAYAPA_BERRY, 100
	return

ADDITEM_TANGA_BERRY:
	additem ITEM_TANGA_BERRY, 100
	return

ADDITEM_CHARTI_BERRY:
	additem ITEM_CHARTI_BERRY, 100
	return

ADDITEM_KASIB_BERRY:
	additem ITEM_KASIB_BERRY, 100
	return

ADDITEM_HABAN_BERRY:
	additem ITEM_HABAN_BERRY, 100
	return

ADDITEM_COLBUR_BERRY:
	additem ITEM_COLBUR_BERRY, 100
	return

ADDITEM_BABIRI_BERRY:
	additem ITEM_BABIRI_BERRY, 100
	return

ADDITEM_CHILAN_BERRY:
	additem ITEM_CHILAN_BERRY, 100
	return

ADDITEM_ROSELI_BERRY:
	additem ITEM_ROSELI_BERRY, 100
	return

ADDITEM_SITRUS_BERRY:
	additem ITEM_SITRUS_BERRY, 400
	return

ADDITEM_LEFTOVERS:
	additem ITEM_LEFTOVERS, 10
	return

ADDITEM_BRIGHT_POWDER:
	additem ITEM_BRIGHT_POWDER, 10
	return

ADDITEM_HARD_STONE:
	additem ITEM_HARD_STONE, 10
	return

ADDITEM_MYSTIC_WATER:
	additem ITEM_MYSTIC_WATER, 10
	return

ADDITEM_SILVER_POWDER:
	additem ITEM_SILVER_POWDER, 10
	return

ADDITEM_POISON_BARB:
	additem ITEM_POISON_BARB, 10
	return

ADDITEM_SOFT_SAND:
	additem ITEM_SOFT_SAND, 10
	return

ADDITEM_BLACK_BELT:
	additem ITEM_BLACK_BELT, 10
	return

ADDITEM_SILK_SCARF:
	additem ITEM_SILK_SCARF, 10
	return

ADDITEM_SHARP_BEAK:
	additem ITEM_SHARP_BEAK, 10
	return

ADDITEM_NEVER_MELT_ICE:
	additem ITEM_NEVER_MELT_ICE, 10
	return

ADDITEM_TWISTED_SPOON:
	additem ITEM_TWISTED_SPOON, 10
	return

ADDITEM_MAGNET:
	additem ITEM_MAGNET, 10
	return

ADDITEM_BLACK_GLASSES:
	additem ITEM_BLACK_GLASSES, 10
	return

ADDITEM_SPELL_TAG:
	additem ITEM_SPELL_TAG, 10
	return

ADDITEM_DRAGON_FANG:
	additem ITEM_DRAGON_FANG, 10
	return

ADDITEM_PP_MAX:
	additem ITEM_PP_MAX, 50
	return
	
ADDITEM_BOTTLE_CAP:
	additem ITEM_BOTTLE_CAP, 50
	return
