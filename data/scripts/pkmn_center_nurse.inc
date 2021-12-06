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
    checkitem ITEM_RARE_CANDY, 400
    compare VAR_RESULT, TRUE
    call_if_ne ADDITEM_RARE_CANDY
    call_if_set FLAG_BADGE01_GET, Give_Cut_Items_After_Roxanne
    call_if_set FLAG_RECEIVED_TM46, Give_Thief_Items_After_Thief
    call_if_set FLAG_HIDE_SLATEPORT_CITY_OCEANIC_MUSEUM_AQUA_GRUNTS, Give_Thief_Items_After_Museum
    call_if_set FLAG_BADGE03_GET, Give_Thief_Items_After_Wattson
    call_if_set FLAG_BADGE04_GET, Give_Thief_Items_After_Flannery
    call_if_set FLAG_BADGE05_GET, Give_Thief_Items_After_Norman
    setvar VAR_0x8004, 0
    specialvar VAR_RESULT, CountPlayerTrainerStars
    compare VAR_RESULT, 4
    goto_if_eq EventScript_PkmnCenterNurse_GoldCard
    msgbox gText_WouldYouLikeToRestYourPkmn, MSGBOX_YESNO
    compare VAR_RESULT, YES
    goto_if_eq EventScript_PkmnCenterNurse_HealPkmn
    compare VAR_RESULT, NO
    goto_if_eq EventScript_PkmnCenterNurse_Goodbye
    end

EventScript_PkmnCenterNurse_Goodbye::
    message gText_WeHopeToSeeYouAgain
    return

@ VAR_0x8004 is 1 when player has Gold Card; jumps are identical
EventScript_PkmnCenterNurse_HealPkmn::
    copyvar VAR_POKE_VIAL_CHARGES, VAR_POKE_VIAL_MAX_CHARGES @ refill Poke Vial
    incrementgamestat GAME_STAT_USED_POKECENTER
    compare VAR_0x8004, 0
    call_if_eq EventScript_PkmnCenterNurse_IllTakeYourPkmn
    compare VAR_0x8004, 1
    call_if_eq EventScript_PkmnCenterNurse_IllTakeYourPkmn2
    waitmessage
    call EventScript_PkmnCenterNurse_TakeAndHealPkmn
    goto_if_unset FLAG_POKERUS_EXPLAINED, EventScript_PkmnCenterNurse_CheckPokerus
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
    compare VAR_0x8004, 1
    goto_if_eq EventScript_PkmnCenterNurse_ReturnPkmn2
    message gText_RestoredPkmnToFullHealth
    waitmessage
    applymovement VAR_0x800B, Movement_PkmnCenterNurse_Bow
    waitmovement 0
    message gText_WeHopeToSeeYouAgain
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

Give_Cut_Items_After_Roxanne:
	return
	
	
Give_Thief_Items_After_Thief:
	return


Give_Thief_Items_After_Museum:
	return

	
Give_Thief_Items_After_Wattson:
	return


Give_Thief_Items_After_Flannery:
	return

Give_Thief_Items_After_Norman:
	return

ADDITEM_RARE_CANDY:
	additem ITEM_RARE_CANDY, 400
	return
	
ADDITEM_BIG_PEARL:
	additem ITEM_BIG_PEARL, 400
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
