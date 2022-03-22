.set LOCALID_TWIN, 1
.set LOCALID_MOM, 4
.set LOCALID_RIVAL, 7
.set LOCALID_BIRCH, 8
	

LittlerootTown_MapScripts:: @ 81E7DCB
	map_script MAP_SCRIPT_ON_TRANSITION, LittlerootTown_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, LittlerootTown_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, LittlerootTown_OnWarp
	.byte 0

	@ State descriptions for VAR_LITTLEROOT_TOWN_STATE
	@ 1: Met Rival
	@ 2: Saving Birch
	@ 3: Received Pokedex
	@ 4: Received Running Shoes

	@ State descriptions for VAR_LITTLEROOT_INTRO_STATE
	@ 1: In truck male
	@ 2: In truck female
	@ 3: Entered house
	@ 4: Told to go set clock
	@ 5: Entered 2F to set clock
	@ 6: Set clock
	@ 7: Told to go meet rival

	@ State descriptions for VAR_LITTLEROOT_HOUSES_STATE_MAY
	@                    and VAR_LITTLEROOT_HOUSES_STATE_BRENDAN
	@ 1: On Truck (and is corresponding gender)
	@ 2: Met Rival's Mom (and is corresponding gender)
	@ 3: Entered Hall of Fame
	@ 4: Completed Lati TV / SS Ticket event
	@ 5: Exited house after above event

	@ State descriptions for VAR_LITTLEROOT_RIVAL_STATE
	@ 1: Never occurs
	@ 2: Entered Rival's bedroom
	@ 3: Met Rival
	@ 4: Received Pokedex


LittlerootTown_OnTransition: @ 81E7DDB
	setflag FLAG_VISITED_LITTLEROOT_TOWN
	call Common_EventScript_SetupRivalGfxId
	compare VAR_LITTLEROOT_INTRO_STATE, 2
	call_if_eq LittlerootTown_EventScript_MoveMomToMaysDoor
	call_if_unset FLAG_RESCUED_BIRCH, LittlerootTown_EventScript_SetTwinPos
	compare VAR_LITTLEROOT_TOWN_STATE, 3
	call_if_eq LittlerootTown_EventScript_SetMomStandingInFrontOfDoorPos
	compare VAR_LITTLEROOT_HOUSES_STATE_MAY, 4
	call_if_eq LittlerootTown_EventScript_SetExitedHouseAfterLatiSSTicketEvent
	compare VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 4
	call_if_eq LittlerootTown_EventScript_SetExitedHouseAfterLatiSSTicketEvent
	compare VAR_OLDALE_RIVAL_STATE, 1
	call_if_eq LittlerootTown_EventScript_MoveRivalFromOldale
	compare VAR_LITTLEROOT_RIVAL_STATE, 3
	call_if_eq LittlerootTown_EventScript_SetRivalLeftForRoute103
	compare VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1
	call_if_eq LittlerootTown_EventScript_HideMapNamePopup
	compare VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 2
	call_if_eq LittlerootTown_EventScript_LeftLabAfterDexUpgrade
	end

LittlerootTown_EventScript_LeftLabAfterDexUpgrade:: @ 81E7E45
	setvar VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 3
	return

LittlerootTown_EventScript_HideMapNamePopup:: @ 81E7E4B
	setflag FLAG_HIDE_MAP_NAME_POPUP
	return

LittlerootTown_EventScript_SetRivalLeftForRoute103:: @ 81E7E4F
	setflag FLAG_RIVAL_LEFT_FOR_ROUTE103
	return

LittlerootTown_EventScript_MoveRivalFromOldale:: @ 81E7E53
	setvar VAR_OLDALE_RIVAL_STATE, 2
	setflag FLAG_HIDE_OLDALE_TOWN_RIVAL
	return

LittlerootTown_EventScript_SetExitedHouseAfterLatiSSTicketEvent:: @ 81E7E5C
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 5
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 5
	return

LittlerootTown_EventScript_MoveMomToMaysDoor:: @ 81E7E67
	setobjectxyperm LOCALID_MOM, 14, 8
	return

LittlerootTown_EventScript_SetTwinPos:: @ 81E7E6F
	compare VAR_LITTLEROOT_TOWN_STATE, 0
	goto_if_eq LittlerootTown_EventScript_SetTwinGuardingRoutePos
	setobjectxyperm LOCALID_TWIN, 10, 1
	setobjectmovementtype LOCALID_TWIN, MOVEMENT_TYPE_FACE_UP
	return

LittlerootTown_EventScript_SetTwinGuardingRoutePos:: @ 81E7E86
	setobjectxyperm LOCALID_TWIN, 7, 2
	setobjectmovementtype LOCALID_TWIN, MOVEMENT_TYPE_FACE_DOWN
	return

LittlerootTown_EventScript_SetMomStandingInFrontOfDoorPos:: @ 81E7E92
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setobjectmovementtype LOCALID_MOM, MOVEMENT_TYPE_FACE_DOWN
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_SetMomInFrontOfDoorMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_SetMomInFrontOfDoorFemale
	return

LittlerootTown_EventScript_SetMomInFrontOfDoorMale:: @ 81E7EB1
	setobjectxyperm LOCALID_MOM, 5, 9
	return

LittlerootTown_EventScript_SetMomInFrontOfDoorFemale:: @ 81E7EB9
	setobjectxyperm LOCALID_MOM, 14, 9
	return

LittlerootTown_OnFrame: @ 81E7EC1
	map_script_2 VAR_LITTLEROOT_INTRO_STATE, 1, LittlerootTown_EventScript_StepOffTruckMale
	map_script_2 VAR_LITTLEROOT_INTRO_STATE, 2, LittlerootTown_EventScript_StepOffTruckFemale
	map_script_2 VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1, LittlerootTown_EventScript_BeginDexUpgradeScene
	.2byte 0

LittlerootTown_EventScript_StepOffTruckMale:: @ 81E7EDB
	lockall
	setvar VAR_0x8004, 5
	setvar VAR_0x8005, 8
	call LittlerootTown_EventScript_GoInsideWithMom
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	warpsilent MAP_LITTLEROOT_TOWN_BRENDANS_HOUSE_1F, 255, 8, 8
	waitstate
	releaseall
	end

LittlerootTown_EventScript_StepOffTruckFemale:: @ 81E7EF9
	lockall
	setvar VAR_0x8004, 14
	setvar VAR_0x8005, 8
	call LittlerootTown_EventScript_GoInsideWithMom
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	warpsilent MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F, 255, 2, 8
	waitstate
	releaseall
	end

LittlerootTown_EventScript_GoInsideWithMom:: @ 81E7F17
	delay 15
	playse SE_LEDGE
	applymovement OBJ_EVENT_ID_PLAYER, LittlerootTown_Movement_PlayerStepOffTruck
	waitmovement 0
	opendoor VAR_0x8004, VAR_0x8005
	waitdooranim
	addobject LOCALID_MOM
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitHouse
	waitmovement 0
	closedoor VAR_0x8004, VAR_0x8005
	waitdooranim
	delay 10
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerAtTruck
	waitmovement 0
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	msgbox LittlerootTown_Text_OurNewHomeLetsGoInside, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachDoor
	applymovement OBJ_EVENT_ID_PLAYER, LittlerootTown_Movement_PlayerApproachDoor
	waitmovement 0
	opendoor VAR_0x8004, VAR_0x8005
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomEnterHouse
	applymovement OBJ_EVENT_ID_PLAYER, LittlerootTown_Movement_PlayerEnterHouse
	waitmovement 0
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE
	setvar VAR_LITTLEROOT_INTRO_STATE, 3
	hideobjectat OBJ_EVENT_ID_PLAYER, 0
	closedoor VAR_0x8004, VAR_0x8005
	waitdooranim
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_FAT_MAN
	clearflag FLAG_HIDE_MAP_NAME_POPUP
	return

LittlerootTown_Movement_MomExitHouse: @ 81E7F98
	walk_down
	step_end

LittlerootTown_Movement_MomApproachPlayerAtTruck: @ 81E7F9A
	walk_down
	walk_in_place_fastest_left
	step_end

LittlerootTown_Movement_MomApproachDoor: @ 81E7F9D
	delay_16
	delay_8
	walk_up
	step_end

LittlerootTown_Movement_MomEnterHouse: @ 81E7FA1
	walk_up
	set_invisible
	step_end

LittlerootTown_Movement_PlayerApproachDoor: @ 81E7FA4
	delay_16
	delay_8
	walk_right
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_PlayerEnterHouse: @ 81E7FA9
	walk_up
	walk_up
	step_end

LittlerootTown_Movement_PlayerStepOffTruck: @ 81E7FAC
	jump_right
	delay_16
	delay_16
	delay_16
	step_end

LittlerootTown_EventScript_BeginDexUpgradeScene:: @ 81E7FB1
	lockall
	playse SE_PIN
	applymovement LOCALID_BIRCH, Common_Movement_ExclamationMark
	waitmovement 0
	delay 80
	msgbox LittlerootTown_Text_BirchSomethingToShowYouAtLab, MSGBOX_DEFAULT
	closemessage
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_RIVAL
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BIRCH
	delay 20
	clearflag FLAG_HIDE_MAP_NAME_POPUP
	warp MAP_LITTLEROOT_TOWN_PROFESSOR_BIRCHS_LAB, 255, 6, 5
	waitstate
	releaseall
	end

LittlerootTown_OnWarp: @ 81E7FE2
	map_script_2 VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 1, LittlerootTown_EventScript_SetRivalBirchPosForDexUpgrade
	.2byte 0

LittlerootTown_EventScript_SetRivalBirchPosForDexUpgrade:: @ 81E7FEC
	addobject LOCALID_BIRCH
	addobject LOCALID_RIVAL
	checkplayergender
	compare VAR_RESULT, MALE
	goto_if_eq LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeMale
	goto LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeFemale
	end

LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeMale:: @ 81E8004
	setobjectxy LOCALID_RIVAL, 6, 10
	setobjectxy LOCALID_BIRCH, 5, 10
	end

LittlerootTown_EventScript_SetRivalBirchPosForDexUpgradeFemale:: @ 81E8013
	setobjectxy LOCALID_RIVAL, 13, 10
	setobjectxy LOCALID_BIRCH, 14, 10
	end

LittlerootTown_EventScript_FatMan:: @ 81E8022
	msgbox LittlerootTown_Text_CanUsePCToStoreItems, MSGBOX_NPC
	end

LittlerootTown_EventScript_Boy:: @ 81E802B
	msgbox LittlerootTown_Text_BirchSpendsDaysInLab, MSGBOX_NPC
	end

LittlerootTown_EventScript_Twin:: @ 81E8034
	lock
	faceplayer
	goto_if_set FLAG_ADVENTURE_STARTED, LittlerootTown_EventScript_GoodLuck
	goto_if_set FLAG_RESCUED_BIRCH, LittlerootTown_EventScript_YouSavedBirch
	compare VAR_LITTLEROOT_TOWN_STATE, 0
	goto_if_ne LittlerootTown_EventScript_GoSaveBirch
	msgbox LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_GoSaveBirch:: @ 81E805D
	special GetPlayerBigGuyGirlString
	msgbox LittlerootTown_Text_CanYouGoSeeWhatsHappening, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_TWIN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

LittlerootTown_EventScript_YouSavedBirch:: @ 81E807A
	special GetPlayerBigGuyGirlString
	msgbox LittlerootTown_Text_YouSavedBirch, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_GoodLuck:: @ 81E8087
	msgbox LittlerootTown_Text_GoodLuckCatchingPokemon, MSGBOX_DEFAULT
	release
	end

LittlerootTown_EventScript_NeedPokemonTriggerLeft:: @ 81E8091
	lockall
	applymovement LOCALID_TWIN, LittlerootTown_Movement_TwinApproachPlayerLeft
	waitmovement 0
	call LittlerootTown_EventScript_DangerousWithoutPokemon
	applymovement LOCALID_TWIN, LittlerootTown_Movement_TwinReturnLeft
	waitmovement 0
	releaseall
	end

LittlerootTown_EventScript_DangerousWithoutPokemon:: @ 81E80AD
	msgbox LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_TWIN, LittlerootTown_Movement_TwinPushPlayerFromRoute
	applymovement OBJ_EVENT_ID_PLAYER, LittlerootTown_Movement_PushPlayerBackFromRoute
	waitmovement 0
	msgbox LittlerootTown_Text_DangerousIfYouDontHavePokemon, MSGBOX_DEFAULT
	closemessage
	return

LittlerootTown_Movement_TwinApproachPlayerLeft: @ 81E80D1
	face_right
	delay_8
	disable_jump_landing_ground_effect
	jump_in_place_right
	delay_8
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_left
	face_down
	step_end

LittlerootTown_Movement_TwinPushPlayerFromRoute: @ 81E80DF
	walk_down
	step_end

LittlerootTown_Movement_TwinReturnLeft: @ 81E80E1
	walk_right
	walk_down
	walk_down
	walk_left
	walk_left
	walk_left
	walk_left
	walk_up
	walk_in_place_fastest_down
	step_end

LittlerootTown_Movement_PushPlayerBackFromRoute: @ 81E80EB
	lock_facing_direction
	walk_down
	unlock_facing_direction
	step_end

LittlerootTown_EventScript_NeedPokemonTriggerRight:: @ 81E80EF
	lockall
	applymovement LOCALID_TWIN, LittlerootTown_Movement_TwinApproachPlayerRight
	waitmovement 0
	call LittlerootTown_EventScript_DangerousWithoutPokemon
	applymovement LOCALID_TWIN, LittlerootTown_Movement_TwinReturnRight
	waitmovement 0
	releaseall
	end

LittlerootTown_Movement_TwinApproachPlayerRight: @ 81E810B
	face_right
	delay_8
	disable_jump_landing_ground_effect
	jump_in_place_right
	delay_8
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_fast_right
	face_down
	step_end

LittlerootTown_Movement_TwinReturnRight: @ 81E8118
	walk_left
	walk_down
	walk_left
	walk_left
	walk_left
	walk_in_place_fastest_down
	step_end

LittlerootTown_EventScript_GoSaveBirchTrigger:: @ 81E811F
	lockall
	applymovement LOCALID_TWIN, Common_Movement_WalkInPlaceFastestRight
	waitmovement 0
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestLeft
	waitmovement 0
	special GetPlayerBigGuyGirlString
	msgbox LittlerootTown_Text_CanYouGoSeeWhatsHappening, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_TWIN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	setvar VAR_LITTLEROOT_TOWN_STATE, 2
	releaseall
	end

LittlerootTown_EventScript_TownSign:: @ 81E8151
	msgbox LittlerootTown_Text_TownSign, MSGBOX_SIGN
	end

LittlerootTown_EventScript_BirchsLabSign:: @ 81E815A
	msgbox LittlerootTown_Text_ProfBirchsLab, MSGBOX_SIGN
	end

LittlerootTown_EventScript_BrendansHouseSign:: @ 81E8163
	lockall
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_PlayersHouseSignMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_BirchsHouseSignFemale
	releaseall
	end

LittlerootTown_EventScript_PlayersHouseSignMale:: @ 81E817D
	msgbox LittlerootTown_Text_PlayersHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_BirchsHouseSignFemale:: @ 81E8186
	msgbox LittlerootTown_Text_ProfBirchsHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_MaysHouseSign:: @ 81E818F
	lockall
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_BirchsHouseSignMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_PlayersHouseSignFemale
	releaseall
	end

LittlerootTown_EventScript_BirchsHouseSignMale:: @ 81E81A9
	msgbox LittlerootTown_Text_ProfBirchsHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_PlayersHouseSignFemale:: @ 81E81B2
	msgbox LittlerootTown_Text_PlayersHouse, MSGBOX_DEFAULT
	return

LittlerootTown_EventScript_GiveRunningShoesTrigger0:: @ 81E81BB
	lockall
	setvar VAR_0x8008, 0
	setobjectxy LOCALID_MOM, 10, 9
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger1:: @ 81E81CE
	lockall
	setvar VAR_0x8008, 1
	setobjectxy LOCALID_MOM, 11, 9
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger2:: @ 81E81E1
	lockall
	setvar VAR_0x8008, 2
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger3:: @ 81E81ED
	lockall
	setvar VAR_0x8008, 3
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger4:: @ 81E81F9
	lockall
	setvar VAR_0x8008, 4
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger5:: @ 81E8205
	lockall
	setvar VAR_0x8008, 5
	goto LittlerootTown_EventScript_GiveRunningShoesTrigger
	end

LittlerootTown_EventScript_GiveRunningShoesTrigger:: @ 81E8211
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_MomNoticePlayerMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_MomNoticePlayerFemale
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_SetHomeDoorCoordsMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_SetHomeDoorCoordsFemale
	msgbox LittlerootTown_Text_WaitPlayer, MSGBOX_DEFAULT
	closemessage
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerFemale
	call LittlerootTown_EventScript_GiveRunningShoes
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_MomReturnHomeMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_MomReturnHomeFemale
	goto LittlerootTown_EventScript_SetReceivedRunningShoes
	end

LittlerootTown_EventScript_SetHomeDoorCoordsMale:: @ 81E8281
	setvar VAR_0x8009, 5
	setvar VAR_0x800A, 8
	return

LittlerootTown_EventScript_SetHomeDoorCoordsFemale:: @ 81E828C
	setvar VAR_0x8009, 14
	setvar VAR_0x800A, 8
	return

LittlerootTown_EventScript_MomNoticePlayerMale:: @ 81E8297
	applymovement LOCALID_MOM, Common_Movement_WalkInPlaceFastestRight
	waitmovement 0
	return

LittlerootTown_EventScript_MomNoticePlayerFemale:: @ 81E82A2
	applymovement LOCALID_MOM, Common_Movement_WalkInPlaceFastestLeft
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerMale:: @ 81E82AD
	compare VAR_0x8008, 0
	call_if_eq LittlerootTown_EventScript_MomApproachPlayer0
	compare VAR_0x8008, 1
	call_if_eq LittlerootTown_EventScript_MomApproachPlayer1
	compare VAR_0x8008, 2
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerMale2
	compare VAR_0x8008, 3
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerMale3
	compare VAR_0x8008, 4
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerMale4
	compare VAR_0x8008, 5
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerMale5
	return

LittlerootTown_EventScript_MomApproachPlayerFemale:: @ 81E82F0
	compare VAR_0x8008, 0
	call_if_eq LittlerootTown_EventScript_MomApproachPlayer0
	compare VAR_0x8008, 1
	call_if_eq LittlerootTown_EventScript_MomApproachPlayer1
	compare VAR_0x8008, 2
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerFemale2
	compare VAR_0x8008, 3
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerFemale3
	compare VAR_0x8008, 4
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerFemale4
	compare VAR_0x8008, 5
	call_if_eq LittlerootTown_EventScript_MomApproachPlayerFemale5
	return

LittlerootTown_EventScript_MomApproachPlayer0:: @ 81E8333
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestDown
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayer0
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayer1:: @ 81E8348
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestDown
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayer1
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerMale2:: @ 81E835D
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestLeft
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerMale2
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerMale3:: @ 81E8372
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestLeft
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerMale3
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerMale4:: @ 81E8387
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestLeft
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerMale4
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerMale5:: @ 81E839C
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestLeft
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerMale5
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerFemale2:: @ 81E83B1
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestRight
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerFemale2
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerFemale3:: @ 81E83C6
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestRight
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerFemale3
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerFemale4:: @ 81E83DB
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestRight
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerFemale4
	waitmovement 0
	return

LittlerootTown_EventScript_MomApproachPlayerFemale5:: @ 81E83F0
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_WalkInPlaceFastestRight
	waitmovement 0
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomApproachPlayerFemale5
	waitmovement 0
	return

LittlerootTown_EventScript_MomReturnHomeMale:: @ 81E8405
	compare VAR_0x8008, 0
	call_if_eq LittlerootTown_EventScript_MomReturnHome0
	compare VAR_0x8008, 1
	call_if_eq LittlerootTown_EventScript_MomReturnHome1
	compare VAR_0x8008, 2
	call_if_eq LittlerootTown_EventScript_MomReturnHomeMale2
	compare VAR_0x8008, 3
	call_if_eq LittlerootTown_EventScript_MomReturnHomeMale3
	compare VAR_0x8008, 4
	call_if_eq LittlerootTown_EventScript_MomReturnHomeMale4
	compare VAR_0x8008, 5
	call_if_eq LittlerootTown_EventScript_MomReturnHomeMale5
	return

LittlerootTown_EventScript_MomReturnHomeFemale:: @ 81E8448
	compare VAR_0x8008, 0
	call_if_eq LittlerootTown_EventScript_MomReturnHome0
	compare VAR_0x8008, 1
	call_if_eq LittlerootTown_EventScript_MomReturnHome1
	compare VAR_0x8008, 2
	call_if_eq LittlerootTown_EventScript_MomReturnHomeFemale2
	compare VAR_0x8008, 3
	call_if_eq LittlerootTown_EventScript_MomReturnHomeFemale3
	compare VAR_0x8008, 4
	call_if_eq LittlerootTown_EventScript_MomReturnHomeFemale4
	compare VAR_0x8008, 5
	call_if_eq LittlerootTown_EventScript_MomReturnHomeFemale5
	return

LittlerootTown_EventScript_MomReturnHome0:: @ 81E848B
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHome0
	waitmovement 0
	return

LittlerootTown_EventScript_MomReturnHome1:: @ 81E8496
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHome1
	waitmovement 0
	return

LittlerootTown_EventScript_MomReturnHomeMale2:: @ 81E84A1
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeMale2
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeMale3:: @ 81E84C7
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeMale3
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeMale4:: @ 81E84ED
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeMale4
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeMale5:: @ 81E8513
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeMale5
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale2:: @ 81E8539
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeFemale2
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale3:: @ 81E855F
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeFemale3
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale4:: @ 81E8585
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeFemale4
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_EventScript_MomReturnHomeFemale5:: @ 81E85AB
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomReturnHomeFemale5
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	return

LittlerootTown_Movement_MomApproachPlayer0: @ 81E85D1
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

LittlerootTown_Movement_MomApproachPlayer1: @ 81E85D8
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

LittlerootTown_Movement_MomApproachPlayerMale2: @ 81E85DF
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerMale3: @ 81E85E4
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerMale4: @ 81E85EA
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerMale5: @ 81E85ED
	walk_right
	walk_right
	walk_right
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale2: @ 81E85F1
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale3: @ 81E85F5
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale4: @ 81E85F8
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomApproachPlayerFemale5: @ 81E85FE
	walk_left
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_Movement_MomReturnHome0: @ 81E8603
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

LittlerootTown_Movement_MomReturnHome1: @ 81E8609
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

LittlerootTown_Movement_MomReturnHomeMale2: @ 81E860F
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeMale3: @ 81E8615
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeMale4: @ 81E861C
	walk_left
	walk_left
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeMale5: @ 81E8620
	walk_left
	walk_left
	walk_left
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale2: @ 81E8625
	walk_right
	walk_right
	walk_right
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale3: @ 81E862A
	walk_right
	walk_right
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale4: @ 81E862E
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomReturnHomeFemale5: @ 81E8635
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_fastest_up
	step_end

LittlerootTown_Movement_MomExitThroughDoor: @ 81E863B
	walk_up
	step_end

LittlerootTown_EventScript_Mom:: @ 81E863D
	lock
	faceplayer
	checkplayergender
	compare VAR_RESULT, MALE
	call_if_eq LittlerootTown_EventScript_SetHomeDoorCoordsMale
	compare VAR_RESULT, FEMALE
	call_if_eq LittlerootTown_EventScript_SetHomeDoorCoordsFemale
	call LittlerootTown_EventScript_GiveRunningShoes
	applymovement LOCALID_MOM, Common_Movement_WalkInPlaceFastestUp
	waitmovement 0
	opendoor VAR_0x8009, VAR_0x800A
	waitdooranim
	applymovement LOCALID_MOM, LittlerootTown_Movement_MomExitThroughDoor
	waitmovement 0
	hideobjectat LOCALID_MOM, MAP_LITTLEROOT_TOWN
	closedoor VAR_0x8009, VAR_0x800A
	waitdooranim
	goto LittlerootTown_EventScript_SetReceivedRunningShoes
	end

LittlerootTown_EventScript_SetReceivedRunningShoes:: @ 81E8686
	removeobject LOCALID_MOM
	setvar VAR_LITTLEROOT_TOWN_STATE, 4
	release
	end

LittlerootTown_EventScript_GiveRunningShoes:: @ 81E8693
	msgbox LittlerootTown_Text_WearTheseRunningShoes, MSGBOX_DEFAULT
	giveitem ITEM_OLD_ROD
	giveitem ITEM_POKEBLOCK_CASE
	setflag FLAG_RECEIVED_OLD_ROD
	setflag FLAG_RECEIVED_POKEBLOCK_CASE
	msgbox LittlerootTown_Text_ComeHomeIfAnythingHappens, MSGBOX_DEFAULT
	closemessage
	delay 30
	return

LittlerootTown_Text_DisableShinyRate:
	.string "Yes: 1/1024\n"
	.string "No:  Cancel$"		

LittlerootTown_Text_ShinyRates:
	.string "Yes: 1/250\n"
	.string "No:  1/100$"
		
LittlerootTown_Text_EnbableShinyRate:
	.string "Yes: Boosted shiny rate options\n"
	.string "No:  Disable boosted shiny rate$"	

LittlerootTown_Text_ShinyRate_0:
	.string "Current shiny rate: 1/1024\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ShinyRate_1:
	.string "Current shiny rate: 1/250\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ShinyRate_2:
	.string "Current shiny rate: 1/100\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_ChangePokevial:
	.string "Enable infinite Poké Vial?$"
		
LittlerootTown_Text_ChangePokevialEnabled:
	.string "Infinite Poké Vial is enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangePokevialDisabled:
	.string "Infinite Poké Vial is disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_ChangeItems:
	.string "Enable nurse Joy items?\n"
	.string "Infinite Big Pearls, Sitrus Berries etc...$"
		
LittlerootTown_Text_ChangeItemsEnabled:
	.string "Nurse Joy items are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeItemsDisabled:
	.string "Nurse Joy items are disabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeBerries:
	.string "Enable instant berries?$"
		
LittlerootTown_Text_ChangeBerriesEnabled:
	.string "Instant berries are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeBerriesDisabled:
	.string "Instant berries are disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_ChangeUniqueColor:
	.string "Enable Individually-Unique Pokémon?\n"
	.string "All credit goes to codes for the patch.$"
		
LittlerootTown_Text_ChangeUniqueColorEnabled:
	.string "Individually-Unique Pokémon is enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeUniqueColorDisabled:
	.string "Individually-Unique Pokémon is Disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_ChangeAIcontrolled:
	.string "Enable AI controlled battles?$"
		
LittlerootTown_Text_ChangeAIcontrolledEnabled:
	.string "AI controlled battles are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeAIcontrolledDisabled:
	.string "AI controlled battles are Disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_MakePlaceForSettingsMon:
	.string "Make sure there's no Pokémon in your PC\n"
	.string "box 14 row 5 column 6.$"

LittlerootTown_Text_ChangeIVSettingsDisabled:
	.string "Perfect IVs are disabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeIVSettingsEnabled:
	.string "Perfect IVs are enabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_HandleIVSettings:
	.string "Enable perfect IVs?\n"
	.string "Trainers won't be affected.$"
		
LittlerootTown_Text_EnableIVSettings:
	.string "ARE YOU SURE???????? All your Pokémon's\n"
	.string "IVs will be set to 31.$"
		
LittlerootTown_Text_DisableIVSettings:
	.string "Disable perfect IVs?$"

LittlerootTown_Text_ChangeEVSettingsEnabled:
	.string "EVs are disabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeEVSettingsDisabled:
	.string "EVs are enabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_Text_HandleEVSettings:
	.string "Disable EVs?\n"
	.string "Trainers WILL be affected.$"
		
LittlerootTown_Text_EnableEVSettings:
	.string "ARE YOU SURE???????? All your Pokémon's\n"
	.string "EVs will be set to 0.$"

LittlerootTown_Text_DisableEVSettings:
	.string "Yes: Allow EVs gain\n"
	.string "No:  Cancel$"

LittlerootTown_Text_OurNewHomeLetsGoInside: @ 81E86BC
	.string "Mom: {PLAYER}, we're here, honey!\p"
	.string "It must be tiring riding with our things\n"
	.string "in the moving truck.\p"
	.string "Well, this is Littleroot Town.\p"
	.string "How do you like it?\n"
	.string "This is our new home!\p"
	.string "It has a quaint feel, but it seems to be\n"
	.string "an easy place to live, don't you think?\p"
	.string "And, you get your own room, {PLAYER}!\n"
	.string "Let's go inside.$"

LittlerootTown_Text_WaitPlayer: @ 81E87E1
	.string "Mom: Wait, {PLAYER}!$"

LittlerootTown_Text_WearTheseRunningShoes: @ 81E87F0
	.string "Mom: {PLAYER}! {PLAYER}! Did you\n"
	.string "introduce yourself to Prof. Birch?\p"
	.string "Oh! What an adorable Pokémon!\n"
	.string "You got it from Prof. Birch. How nice!\p"
	.string "You're your father's child, all right.\n"
	.string "You look good together with Pokémon!\p"
	.string "Here, honey! If you're going out on an\n"
	.string "adventure, take Dad's old stuff.\p"
	.string "He never uses it anyway!$"

LittlerootTown_Text_ComeHomeIfAnythingHappens: @ 81E8A03
	.string "… … … … … … … …\n"
	.string "… … … … … … … …\p"
	.string "To think that you have your very own\n"
	.string "Pokémon now…\l"
	.string "Your father will be overjoyed.\p"
	.string "…But please be careful.\n"
	.string "If anything happens, you can come home.\p"
	.string "Go on, go get them, honey!$"

LittlerootTown_Text_CanUsePCToStoreItems: @ 81E8ACF
	.string "If you use a PC, you can store items\n"
	.string "and Pokémon.\p"
	.string "The power of science is staggering!$"

LittlerootTown_Text_BirchSpendsDaysInLab: @ 81E8B25
	.string "Prof. Birch spends days in his Lab\n"
	.string "studying, then he'll suddenly go out in\l"
	.string "the wild to do more research…\p"
	.string "When does Prof. Birch spend time\n"
	.string "at home?$"

LittlerootTown_Text_IfYouGoInGrassPokemonWillJumpOut: @ 81E8BB8
	.string "Um, um, um!\p"
	.string "If you go outside and go in the grass,\n"
	.string "wild Pokémon will jump out!$"

LittlerootTown_Text_DangerousIfYouDontHavePokemon: @ 81E8C07
	.string "It's dangerous if you don't have\n"
	.string "your own Pokémon.$"

LittlerootTown_Text_CanYouGoSeeWhatsHappening: @ 81E8C3A
	.string "Um, hi!\p"
	.string "There are scary Pokémon outside!\n"
	.string "I can hear their cries!\p"
	.string "I want to go see what's going on,\n"
	.string "but I don't have any Pokémon…\p"
	.string "Can you go see what's happening\n"
	.string "for me?$"

LittlerootTown_Text_YouSavedBirch: @ 81E8CE3
	.string "You saved Prof. Birch!\n"
	.string "I'm so glad!$"

LittlerootTown_Text_GoodLuckCatchingPokemon: @ 81E8D07
	.string "Are you going to catch Pokémon?\n"
	.string "Good luck!$"

LittlerootTown_Text_TownSign: @ 81E8D32
	.string "Littleroot Town\n"
	.string "“A town that can't be shaded any hue.”$"

LittlerootTown_Text_ProfBirchsLab: @ 81E8D69
	.string "Prof. Birch's Pokémon Lab$"

LittlerootTown_Text_PlayersHouse: @ 81E8D83
	.string "{PLAYER}'s House$"

LittlerootTown_Text_ProfBirchsHouse: @ 81E8D8E
	.string "Prof. Birch's House$"

LittlerootTown_Text_BirchSomethingToShowYouAtLab: @ 81E8DA2
	.string "Prof. Birch: Well, well, {PLAYER}{KUN}!\n"
	.string "That was good work out there!\p"
	.string "I knew there was something special\n"
	.string "about you when I first saw you,\l"
	.string "but I never expected this.\p"
	.string "I have something to show you.\n"
	.string "Let's go to my Lab.$"
	
	
	

	
		

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
LittlerootTown_EventScript_PatchSettings_PerfectIV::
	faceplayer
	lockall
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	compare VAR_RESULT, 1
	goto_if_ne LittlerootTown_EventScript_NoPlaceForSettingsMon
	setvar VAR_0x8006, 0
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	goto_if_eq LittlerootTown_EventScript_ChangeIVSettingsEnabled
	compare VAR_0x8005, 1
	goto_if_ne LittlerootTown_EventScript_ChangeIVSettingsDisabled
	releaseall
	end
	
LittlerootTown_EventScript_PatchSettings_DisableEV::
	faceplayer
	lockall
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	compare VAR_RESULT, 1
	goto_if_ne LittlerootTown_EventScript_NoPlaceForSettingsMon
	setvar VAR_0x8006, 1
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	goto_if_eq LittlerootTown_EventScript_ChangeEVSettingsEnabled
	compare VAR_0x8005, 1
	goto_if_ne LittlerootTown_EventScript_ChangeEVSettingsDisabled
	releaseall
	end	
	
LittlerootTown_EventScript_PatchSettings_UniqueColor::
	faceplayer
	lockall
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	compare VAR_RESULT, 1
	goto_if_ne LittlerootTown_EventScript_NoPlaceForSettingsMon
	call LittlerootTown_EventScript_UniqueColor
	releaseall
	end

	
LittlerootTown_EventScript_PatchSettings_AIcontrolled::
	faceplayer
	lockall
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	compare VAR_RESULT, 1
	goto_if_ne LittlerootTown_EventScript_NoPlaceForSettingsMon
	call LittlerootTown_EventScript_AIcontrolled
	releaseall
	end
LittlerootTown_EventScript_NoPlaceForSettingsMon::
	msgbox LittlerootTown_Text_MakePlaceForSettingsMon, MSGBOX_DEFAULT
	releaseall
	end
	
LittlerootTown_EventScript_PatchSettings_Others::
	faceplayer
	lockall
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	compare VAR_RESULT, 1
	goto_if_ne LittlerootTown_EventScript_NoPlaceForSettingsMon
	call LittlerootTown_EventScript_PatchSettings_Items
	call LittlerootTown_EventScript_PatchSettings_Berries
	call LittlerootTown_EventScript_PatchSettings_ShinyRate
	call LittlerootTown_EventScript_PatchSettings_Pokevial
	releaseall
	end
	
LittlerootTown_EventScript_HandleIVSettings::
	msgbox LittlerootTown_Text_HandleIVSettings, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq LittlerootTown_EventScript_DisableIVSettings
	compare VAR_RESULT, YES
	goto_if_eq LittlerootTown_EventScript_EnableIVSettings
	
LittlerootTown_EventScript_DisableIVSettings::
	msgbox LittlerootTown_Text_DisableIVSettings, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq LittlerootTown_EventScript_PatchSettings_PerfectIV
	setvar VAR_0x8006, 0 
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	releaseall
	end
		
LittlerootTown_EventScript_EnableIVSettings::
	msgbox LittlerootTown_Text_EnableIVSettings, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq LittlerootTown_EventScript_PatchSettings_PerfectIV
	setvar VAR_0x8006, 0 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	releaseall
	end
		
LittlerootTown_EventScript_HandleEVSettings::
	msgbox LittlerootTown_Text_HandleEVSettings, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq LittlerootTown_EventScript_DisableEVSettings
	compare VAR_RESULT, YES
	goto_if_eq LittlerootTown_EventScript_EnableEVSettings
		
LittlerootTown_EventScript_DisableEVSettings::
	msgbox LittlerootTown_Text_DisableEVSettings, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq LittlerootTown_EventScript_PatchSettings_DisableEV
	setvar VAR_0x8006, 1 
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	releaseall
	end
		
LittlerootTown_EventScript_EnableEVSettings::
	msgbox LittlerootTown_Text_EnableEVSettings, MSGBOX_YESNO
	compare VAR_RESULT, NO
	goto_if_eq LittlerootTown_EventScript_PatchSettings_DisableEV
	setvar VAR_0x8006, 1 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats		
	releaseall
	end

LittlerootTown_EventScript_ChangeEVSettingsEnabled::
	msgbox LittlerootTown_Text_ChangeEVSettingsEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES	
	goto_if_eq LittlerootTown_EventScript_HandleEVSettings
	releaseall
	end
	
LittlerootTown_EventScript_ChangeEVSettingsDisabled::
	msgbox LittlerootTown_Text_ChangeEVSettingsDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES	
	goto_if_eq LittlerootTown_EventScript_HandleEVSettings
	releaseall
	end
	
LittlerootTown_EventScript_ChangeIVSettingsEnabled::
	msgbox LittlerootTown_Text_ChangeIVSettingsEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES	
	goto_if_eq LittlerootTown_EventScript_HandleIVSettings
	releaseall
	end
	
LittlerootTown_EventScript_ChangeIVSettingsDisabled::
	msgbox LittlerootTown_Text_ChangeIVSettingsDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES	
	goto_if_eq LittlerootTown_EventScript_HandleIVSettings
	releaseall
	end
	
LittlerootTown_EventScript_AIcontrolled::
	setvar VAR_0x8006, 7
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeAIcontrolledEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeAIcontrolledDisabled
	return	
	
LittlerootTown_EventScript_ChangeAIcontrolledEnabled::
	msgbox LittlerootTown_Text_ChangeAIcontrolledEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeAIcontrolled
	return

LittlerootTown_EventScript_ChangeAIcontrolledDisabled::
	msgbox LittlerootTown_Text_ChangeAIcontrolledDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeAIcontrolled
	return

LittlerootTown_EventScript_ChangeAIcontrolled::
	msgbox LittlerootTown_Text_ChangeAIcontrolled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableAIcontrolled
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableAIcontrolled
	return	

LittlerootTown_EventScript_EnableAIcontrolled::
	setvar VAR_0x8006, 7
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableAIcontrolled::
	setvar VAR_0x8006, 7
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return

LittlerootTown_EventScript_UniqueColor::
	setvar VAR_0x8006, 6
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeUniqueColorEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeUniqueColorDisabled
	return	
	
LittlerootTown_EventScript_ChangeUniqueColorEnabled::
	msgbox LittlerootTown_Text_ChangeUniqueColorEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeUniqueColor
	return

LittlerootTown_EventScript_ChangeUniqueColorDisabled::
	msgbox LittlerootTown_Text_ChangeUniqueColorDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeUniqueColor
	return

LittlerootTown_EventScript_ChangeUniqueColor::
	msgbox LittlerootTown_Text_ChangeUniqueColor, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableUniqueColor
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableUniqueColor
	return
		
LittlerootTown_EventScript_EnableUniqueColor::
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableUniqueColor::
	setvar VAR_0x8006, 6
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return

LittlerootTown_EventScript_PatchSettings_Berries::
	setvar VAR_0x8006, 5
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeBerriesEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeBerriesDisabled
	return	
	
LittlerootTown_EventScript_ChangeBerriesEnabled::
	msgbox LittlerootTown_Text_ChangeBerriesEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeBerries
	return

LittlerootTown_EventScript_ChangeBerriesDisabled::
	msgbox LittlerootTown_Text_ChangeBerriesDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeBerries
	return

LittlerootTown_EventScript_ChangeBerries::
	msgbox LittlerootTown_Text_ChangeBerries, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableBerries
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableBerries
	return

LittlerootTown_EventScript_EnableBerries::
	setvar VAR_0x8006, 5 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableBerries::
	setvar VAR_0x8006, 5
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return

LittlerootTown_EventScript_PatchSettings_Items::
	setvar VAR_0x8006, 4
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeItemsEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeItemsDisabled
	return	
	
LittlerootTown_EventScript_ChangeItemsEnabled::
	msgbox LittlerootTown_Text_ChangeItemsEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeItems
	return

LittlerootTown_EventScript_ChangeItemsDisabled::
	msgbox LittlerootTown_Text_ChangeItemsDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeItems
	return

LittlerootTown_EventScript_ChangeItems::
	msgbox LittlerootTown_Text_ChangeItems, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableItems
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableItems
	return

LittlerootTown_EventScript_EnableItems::
	setvar VAR_0x8006, 4 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableItems::
	setvar VAR_0x8006, 4
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return			
		
LittlerootTown_EventScript_PatchSettings_Pokevial::
	setvar VAR_0x8006, 3
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangePokevialEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangePokevialDisabled
	return	
	
LittlerootTown_EventScript_ChangePokevialEnabled::
	msgbox LittlerootTown_Text_ChangePokevialEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangePokevial
	return

LittlerootTown_EventScript_ChangePokevialDisabled::
	msgbox LittlerootTown_Text_ChangePokevialDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangePokevial
	return

LittlerootTown_EventScript_ChangePokevial::
	msgbox LittlerootTown_Text_ChangePokevial, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnablePokevial
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisablePokevial
	return

LittlerootTown_EventScript_EnablePokevial::
	setvar VAR_0x8006, 3 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisablePokevial::
	setvar VAR_0x8006, 3
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_ShinyRate_0::
	msgbox LittlerootTown_Text_ShinyRate_0, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_HandleShinyRate
	return

LittlerootTown_EventScript_ShinyRate_1::
	msgbox LittlerootTown_Text_ShinyRate_1, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_HandleShinyRate
	return

LittlerootTown_EventScript_ShinyRate_2::
	msgbox LittlerootTown_Text_ShinyRate_2, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_HandleShinyRate
	return
	
		
LittlerootTown_EventScript_HandleShinyRate::
	setvar VAR_0x8004, 0
	msgbox LittlerootTown_Text_EnbableShinyRate, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableShinyRate
	compare VAR_0x8004, 0
	call_if_eq LittlerootTown_EventScript_DisableShinyRate
	return

LittlerootTown_EventScript_EnableShinyRate::
	setvar VAR_0x8004, 1
	msgbox LittlerootTown_Text_ShinyRates, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableShinyRate_1
	compare VAR_RESULT, NO
	call_if_eq LittlerootTown_EventScript_EnableShinyRate_2
	return

LittlerootTown_EventScript_DisableShinyRate::
	msgbox LittlerootTown_Text_DisableShinyRate, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableShinyRate_0
	return
			
		
LittlerootTown_EventScript_PatchSettings_ShinyRate::
	setvar VAR_0x8006, 2
	special GetSettingsMonStats
	compare VAR_0x8005,1
	call_if_lt LittlerootTown_EventScript_ShinyRate_0
	compare VAR_0x8005,1
	call_if_eq LittlerootTown_EventScript_ShinyRate_1
	compare VAR_0x8005,1
	call_if_gt LittlerootTown_EventScript_ShinyRate_2
	return
	
LittlerootTown_EventScript_EnableShinyRate_0::
	setvar VAR_0x8006, 2 
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_EnableShinyRate_1::
	setvar VAR_0x8006, 2
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return

LittlerootTown_EventScript_EnableShinyRate_2::
	setvar VAR_0x8006, 2
	setvar VAR_0x8007, 2
	special SetSettingsMonStats
	return















LittlerootTown_EventScript_PatchSettings_Random::
	faceplayer
	lockall
	givemon SPECIES_RATTATA, 0, ITEM_NONE
	compare VAR_RESULT, 1
	goto_if_ne LittlerootTown_EventScript_NoPlaceForSettingsMon
	call LittlerootTown_EventScript_PatchSettings_Starters
	call LittlerootTown_EventScript_PatchSettings_Legends
	@call LittlerootTown_EventScript_PatchSettings_Unobtainable
	call LittlerootTown_EventScript_PatchSettings_AltForms
	call LittlerootTown_EventScript_PatchSettings_Randomizer
	releaseall
	end

LittlerootTown_Text_ChangeLegends:
	.string "Enable Legendary/Mythical?\n"
	.string "This affect random starters/eggs$"
		
LittlerootTown_Text_ChangeLegendsEnabled:
	.string "Legendary/Mythical are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeLegendsDisabled:
	.string "Legendary/Mythical are disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_EventScript_PatchSettings_Legends::
	setvar VAR_0x8006, 8
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeLegendsEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeLegendsDisabled
	return	
	
LittlerootTown_EventScript_ChangeLegendsEnabled::
	msgbox LittlerootTown_Text_ChangeLegendsEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeLegends
	return

LittlerootTown_EventScript_ChangeLegendsDisabled::
	msgbox LittlerootTown_Text_ChangeLegendsDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeLegends
	return

LittlerootTown_EventScript_ChangeLegends::
	msgbox LittlerootTown_Text_ChangeLegends, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableLegends
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableLegends
	return

LittlerootTown_EventScript_EnableLegends::
	setvar VAR_0x8006, 8 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableLegends::
	setvar VAR_0x8006, 8
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
	
		
		
		
		
		
		
LittlerootTown_Text_ChangeStarters:
	.string "Enable random starters?$"
		
LittlerootTown_Text_ChangeStartersEnabled:
	.string "Random starters are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeStartersDisabled:
	.string "Random starters are disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_EventScript_PatchSettings_Starters::
	setvar VAR_0x8006, 9
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeStartersEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeStartersDisabled
	return	
	
LittlerootTown_EventScript_ChangeStartersEnabled::
	msgbox LittlerootTown_Text_ChangeStartersEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeStarters
	return

LittlerootTown_EventScript_ChangeStartersDisabled::
	msgbox LittlerootTown_Text_ChangeStartersDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeStarters
	return

LittlerootTown_EventScript_ChangeStarters::
	msgbox LittlerootTown_Text_ChangeStarters, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableStarters
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableStarters
	return

LittlerootTown_EventScript_EnableStarters::
	setvar VAR_0x8006, 9 
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableStarters::
	setvar VAR_0x8006, 9
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return	
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
LittlerootTown_Text_ChangeUnobtainable:
	.string "Enable unobtainable Pokémon?$"
		
LittlerootTown_Text_ChangeUnobtainableEnabled:
	.string "Unobtainable Pokémon are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeUnobtainableDisabled:
	.string "Unobtainable Pokémon are disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_EventScript_PatchSettings_Unobtainable::
	setvar VAR_0x8006, 10
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeUnobtainableEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeUnobtainableDisabled
	return	
	
LittlerootTown_EventScript_ChangeUnobtainableEnabled::
	msgbox LittlerootTown_Text_ChangeUnobtainableEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeUnobtainable
	return

LittlerootTown_EventScript_ChangeUnobtainableDisabled::
	msgbox LittlerootTown_Text_ChangeUnobtainableDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeUnobtainable
	return

LittlerootTown_EventScript_ChangeUnobtainable::
	msgbox LittlerootTown_Text_ChangeUnobtainable, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableUnobtainable
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableUnobtainable
	return

LittlerootTown_EventScript_EnableUnobtainable::
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableUnobtainable::
	setvar VAR_0x8006, 10
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return	
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
		
LittlerootTown_Text_ChangeAltForms:
	.string "Enable alternate forms?\n"
	.string "(Alolan, Galarian, cosmetic etc.)$"
		
LittlerootTown_Text_ChangeAltFormsEnabled:
	.string "Alternate forms are enabled.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeAltFormsDisabled:
	.string "Alternate forms are disabled.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_EventScript_PatchSettings_AltForms::
	setvar VAR_0x8006, 11
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeAltFormsEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeAltFormsDisabled
	return	
	
LittlerootTown_EventScript_ChangeAltFormsEnabled::
	msgbox LittlerootTown_Text_ChangeAltFormsEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeAltForms
	return

LittlerootTown_EventScript_ChangeAltFormsDisabled::
	msgbox LittlerootTown_Text_ChangeAltFormsDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeAltForms
	return

LittlerootTown_EventScript_ChangeAltForms::
	msgbox LittlerootTown_Text_ChangeAltForms, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableAltForms
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableAltForms
	return

LittlerootTown_EventScript_EnableAltForms::
	setvar VAR_0x8006, 11
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableAltForms::
	setvar VAR_0x8006, 11
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return	
		
		
		
		
		
		
		
		
LittlerootTown_Text_ChangeRandomizer:
	.string "Enable randomized wild encounters?\n"
	.string "This isn't affected by previous settings.\p"
	.string "This is not a chaos randomizer and should\n"
	.string "hopefully keep some kind a balance.$"
		
LittlerootTown_Text_ChangeRandomizerEnabled:
	.string "Wild encounters are randomized.\n"
	.string "Would you like to change this settings?$"

LittlerootTown_Text_ChangeRandomizerDisabled:
	.string "Wild encounters are not randomized.\n"
	.string "Would you like to change this settings?$"
		
LittlerootTown_EventScript_PatchSettings_Randomizer::
	setvar VAR_0x8006, 12
	special GetSettingsMonStats
	compare VAR_0x8005, 1
	call_if_eq LittlerootTown_EventScript_ChangeRandomizerEnabled
	compare VAR_0x8005, 1
	call_if_ne LittlerootTown_EventScript_ChangeRandomizerDisabled
	return	
	
LittlerootTown_EventScript_ChangeRandomizerEnabled::
	msgbox LittlerootTown_Text_ChangeRandomizerEnabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeRandomizer
	return

LittlerootTown_EventScript_ChangeRandomizerDisabled::
	msgbox LittlerootTown_Text_ChangeRandomizerDisabled, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_ChangeRandomizer
	return

LittlerootTown_EventScript_ChangeRandomizer::
	msgbox LittlerootTown_Text_ChangeRandomizer, MSGBOX_YESNO
	compare VAR_RESULT, YES
	call_if_eq LittlerootTown_EventScript_EnableRandomizer
	compare VAR_RESULT, YES
	call_if_ne LittlerootTown_EventScript_DisableRandomizer
	return

LittlerootTown_EventScript_EnableRandomizer::
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 1
	special SetSettingsMonStats
	return
		
LittlerootTown_EventScript_DisableRandomizer::
	setvar VAR_0x8006, 12
	setvar VAR_0x8007, 0
	special SetSettingsMonStats
	return	
