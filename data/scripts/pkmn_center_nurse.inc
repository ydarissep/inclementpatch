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
    compare VAR_RESULT, FALSE
    call_if_eq ADDITEM_ITEM_RARE_CANDY
    removeitem ITEM_BIG_PEARL, 400
    additem ITEM_BIG_PEARL, 400
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
	removeitem ITEM_MIRACLE_SEED, 10	
	removeitem ITEM_CHARCOAL, 10
	removeitem ITEM_LEEK, 10
	removeitem ITEM_BIG_ROOT, 10
	removeitem ITEM_MENTAL_HERB, 100
	removeitem ITEM_POWER_HERB, 100
	removeitem ITEM_WHITE_HERB, 100
	removeitem ITEM_ELECTRIC_SEED, 100
	removeitem ITEM_PSYCHIC_SEED, 100
	removeitem ITEM_GRASSY_SEED, 100
	removeitem ITEM_MISTY_SEED, 100
	removeitem ITEM_CHERI_BERRY, 100
	removeitem ITEM_CHESTO_BERRY, 100
	removeitem ITEM_PECHA_BERRY, 100
	removeitem ITEM_RAWST_BERRY, 100
	removeitem ITEM_PERSIM_BERRY, 100
	removeitem ITEM_OCCA_BERRY, 100
	removeitem ITEM_PASSHO_BERRY, 100
	removeitem ITEM_WACAN_BERRY, 100
	removeitem ITEM_RINDO_BERRY, 100
	removeitem ITEM_YACHE_BERRY, 100
	removeitem ITEM_CHOPLE_BERRY, 100
	removeitem ITEM_KEBIA_BERRY, 100
	removeitem ITEM_SHUCA_BERRY, 100
	removeitem ITEM_COBA_BERRY, 100
	removeitem ITEM_PAYAPA_BERRY, 100
	removeitem ITEM_TANGA_BERRY, 100
	removeitem ITEM_CHARTI_BERRY, 100
	removeitem ITEM_KASIB_BERRY, 100
	removeitem ITEM_HABAN_BERRY, 100
	removeitem ITEM_COLBUR_BERRY, 100
	removeitem ITEM_BABIRI_BERRY, 100
	removeitem ITEM_CHILAN_BERRY, 100
	removeitem ITEM_ROSELI_BERRY, 100
	additem ITEM_MIRACLE_SEED, 10	
	additem ITEM_CHARCOAL, 10
	additem ITEM_LEEK, 10
	additem ITEM_BIG_ROOT, 10
	additem ITEM_MENTAL_HERB, 100
	additem ITEM_POWER_HERB, 100
	additem ITEM_WHITE_HERB, 100
	additem ITEM_ELECTRIC_SEED, 100
	additem ITEM_PSYCHIC_SEED, 100
	additem ITEM_GRASSY_SEED, 100
	additem ITEM_MISTY_SEED, 100
	additem ITEM_CHERI_BERRY, 100
	additem ITEM_CHESTO_BERRY, 100
	additem ITEM_PECHA_BERRY, 100
	additem ITEM_RAWST_BERRY, 100
	additem ITEM_PERSIM_BERRY, 100
	additem ITEM_OCCA_BERRY, 100
	additem ITEM_PASSHO_BERRY, 100
	additem ITEM_WACAN_BERRY, 100
	additem ITEM_RINDO_BERRY, 100
	additem ITEM_YACHE_BERRY, 100
	additem ITEM_CHOPLE_BERRY, 100
	additem ITEM_KEBIA_BERRY, 100
	additem ITEM_SHUCA_BERRY, 100
	additem ITEM_COBA_BERRY, 100
	additem ITEM_PAYAPA_BERRY, 100
	additem ITEM_TANGA_BERRY, 100
	additem ITEM_CHARTI_BERRY, 100
	additem ITEM_KASIB_BERRY, 100
	additem ITEM_HABAN_BERRY, 100
	additem ITEM_COLBUR_BERRY, 100
	additem ITEM_BABIRI_BERRY, 100
	additem ITEM_CHILAN_BERRY, 100
	additem ITEM_ROSELI_BERRY, 100
	return
	
Give_Thief_Items_After_Thief:
	removeitem ITEM_SITRUS_BERRY, 400
	removeitem ITEM_LEFTOVERS, 10
	removeitem ITEM_BRIGHT_POWDER, 10
	removeitem ITEM_HARD_STONE, 10
	removeitem ITEM_MYSTIC_WATER, 10
	removeitem ITEM_SILVER_POWDER, 10
	removeitem ITEM_POISON_BARB, 10
	removeitem ITEM_SOFT_SAND, 10
	removeitem ITEM_BLACK_BELT, 10
	additem ITEM_SITRUS_BERRY, 400
	additem ITEM_LEFTOVERS, 10
	additem ITEM_BRIGHT_POWDER, 10
	additem ITEM_HARD_STONE, 10
	additem ITEM_MYSTIC_WATER, 10
	additem ITEM_SILVER_POWDER, 10
	additem ITEM_POISON_BARB, 10
	additem ITEM_SOFT_SAND, 10
	additem ITEM_BLACK_BELT, 10
	return


Give_Thief_Items_After_Museum:
	removeitem ITEM_SILK_SCARF, 10
	removeitem ITEM_SHARP_BEAK, 10
	removeitem ITEM_NEVER_MELT_ICE, 10
	removeitem ITEM_TWISTED_SPOON, 10
	additem ITEM_SILK_SCARF, 10
	additem ITEM_SHARP_BEAK, 10
	additem ITEM_NEVER_MELT_ICE, 10
	additem ITEM_TWISTED_SPOON, 10
	return
	
Give_Thief_Items_After_Wattson:
	removeitem ITEM_MAGNET, 10
	additem ITEM_MAGNET, 10
	return


Give_Thief_Items_After_Flannery:
	removeitem ITEM_BLACK_GLASSES, 10
	removeitem ITEM_SPELL_TAG, 10
	additem ITEM_BLACK_GLASSES, 10
	additem ITEM_SPELL_TAG, 10
	return

Give_Thief_Items_After_Norman:
	removeitem ITEM_DRAGON_FANG, 10
	additem ITEM_DRAGON_FANG, 10
	return
	
ADDITEM_ITEM_RARE_CANDY:
	additem ITEM_RARE_CANDY, 400
	return
