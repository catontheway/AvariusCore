#include "AccountMgr.h"
#include "time.h"
#include <stdio.h>
#include "Bag.h"
#include "Common.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStructure.h"
#include "Define.h"
#include "Field.h"
#include "GameEventMgr.h"
#include "Item.h"
#include "Language.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryResult.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Transaction.h"
#include "WorldSession.h"
#include <sstream>
#include <string>
#include <stdlib.h>
#include <Custom/Logic/CustomPlayerLog.h>
#include <Custom/Logic/CustomCharacterSystem.h>
#include <Custom/Logic/CustomTranslationSystem.h>



#define GROUPID 2

enum Translation {

	NOT_ENOUGH_CREDITS = 1,
	PLEASE_LOGOUT  = 2,
	RACE_CHANGE = 3,
	NAME_CHANGE = 4, 
	FACTION_CHANGE = 5,
	FACTION_AND_RACE_CHANGE = 6,
	HELPMENU  = 7 ,
	HELPEXPLANATION = 8,


};

class Race_Change_NPC : public CreatureScript
{
public:
	Race_Change_NPC() : CreatureScript("racechange") {  }

	void DBeintrag(Player* player, std::string reason){
		CustomPlayerLog * PlayerLog = 0;
		PlayerLog->addCompletePlayerLog(player->GetSession()->GetPlayer(), reason);
		return;

	}


	bool OnGossipHello(Player* player, Creature* creature)
	{
		
		if (sConfigMgr->GetBoolDefault("Race.Change", true)) {
			CustomTranslationSystem * TranslationSystem = 0;
			std::string howdoesthiswork = TranslationSystem->getCompleteTranslationsString(GROUPID, HELPMENU, player->GetSession()->GetPlayer());
			std::string racechange = TranslationSystem->getCompleteTranslationsString(GROUPID, RACE_CHANGE, player->GetSession()->GetPlayer());
			std::string factionchange = TranslationSystem->getCompleteTranslationsString(GROUPID, FACTION_CHANGE, player->GetSession()->GetPlayer());
			std::string factionandracechange = TranslationSystem->getCompleteTranslationsString(GROUPID, FACTION_AND_RACE_CHANGE, player->GetSession()->GetPlayer());
			std::string rename = TranslationSystem->getCompleteTranslationsString(GROUPID, NAME_CHANGE, player->GetSession()->GetPlayer());

			player->PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, GOSSIP_ICON_CHAT,howdoesthiswork, GOSSIP_SENDER_MAIN, 0, "", 0, false);
			player->PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, GOSSIP_ICON_CHAT, racechange, GOSSIP_SENDER_MAIN, 1, "", 0, false);
			player->PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, GOSSIP_ICON_CHAT, factionchange, GOSSIP_SENDER_MAIN, 2, "", 0, false);
			player->PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, GOSSIP_ICON_CHAT, factionandracechange, GOSSIP_SENDER_MAIN, 3, "", 0, false);
			player->PlayerTalkClass->GetGossipMenu().AddMenuItem(-1, GOSSIP_ICON_CHAT, rename, GOSSIP_SENDER_MAIN, 4, "", 0, false);
			player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
			return true;
		}
		else {
			creature->SetPhaseMask(2, true);
			player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
			return true;
		}
		
	}
    
	bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action){
		player->PlayerTalkClass->ClearMenus();
		CustomTranslationSystem * TranslationSystem = 0;
		std::string helpexplanation = TranslationSystem->getCompleteTranslationsString(GROUPID, HELPEXPLANATION, player->GetSession()->GetPlayer());
		std::string logoutnow = TranslationSystem->getCompleteTranslationsString(GROUPID, PLEASE_LOGOUT, player->GetSession()->GetPlayer());
		int vipCoin = sConfigMgr->GetIntDefault("Vip.Vendor.CurrencyID", 38186);
		if (sender != GOSSIP_SENDER_MAIN) {
			return false;
		}
			

		switch (action){
		case  0:
			
			ChatHandler(player->GetSession()).PSendSysMessage("%s",helpexplanation,
					player->GetName());
				player->PlayerTalkClass->SendCloseGossip();
				return true;
			break;

			//RaceChange
		case  1:
			
			if (player->HasItemCount(vipCoin, 2) && player->HasEnoughMoney(500*GOLD)){
				player->DestroyItemCount(49426, 2, true, false);
				player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
				player->GetGUID();
				std::ostringstream ss;
				ss << "|cff54b5ffEin Rassenwechsel wurde durchgefuehrt von: |r " << ChatHandler(player->GetSession()).GetNameLink();
				sWorld->SendGMText(LANG_GM_BROADCAST, ss.str().c_str());
				DBeintrag(player->GetSession()->GetPlayer(), "Racechange");
				ChatHandler(player->GetSession()).PSendSysMessage("%s", logoutnow,
					player->GetName());
				player->PlayerTalkClass->SendCloseGossip();
				return true;
			}

			else{
				std::string notenoughcredit = TranslationSystem->getCompleteTranslationsString(GROUPID, NOT_ENOUGH_CREDITS, player->GetSession()->GetPlayer());
				creature->Say(notenoughcredit, LANG_UNIVERSAL,nullptr);
				return true;
			}

			break;


			//Factionchange
		case  2:

			if (player->HasItemCount(vipCoin, 2)){
				player->DestroyItemCount(49426, 2, true);
				player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
				player->GetGUID();
				std::ostringstream ss;
				ss << "|cff54b5ffEin Fraktionswechsel wurde durchgefuehrt von: |r " << ChatHandler(player->GetSession()).GetNameLink();
				sWorld->SendGMText(LANG_GM_BROADCAST, ss.str().c_str());
				DBeintrag(player->GetSession()->GetPlayer(), "Factionchange");
				ChatHandler(player->GetSession()).PSendSysMessage("%s", logoutnow,
					player->GetName());
				player->PlayerTalkClass->SendCloseGossip();
				return true;
			}


		else{
			std::string notenoughcredit = TranslationSystem->getCompleteTranslationsString(GROUPID, NOT_ENOUGH_CREDITS, player->GetSession()->GetPlayer());
			creature->Say(notenoughcredit, LANG_UNIVERSAL, nullptr);
			return true;
		}

			break;

			//Faction and Race
		case 3:
			if (player->HasItemCount(vipCoin, 4)){
				player->DestroyItemCount(49426, 4, true, false);
				player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
				player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
				
				player->GetGUID();
				std::ostringstream ss;
				DBeintrag(player->GetSession()->GetPlayer(), "Faction and Race change");
				ss << "|cff54b5ffEin Rassen und Fraktionswechsel wurde durchgefuehrt von: |r " << ChatHandler(player->GetSession()).GetNameLink();
				sWorld->SendGMText(LANG_GM_BROADCAST, ss.str().c_str());
				ChatHandler(player->GetSession()).PSendSysMessage("%s", logoutnow,
					player->GetName());
				player->PlayerTalkClass->SendCloseGossip();
				return true;
			}


			else{
				std::string notenoughcredit = TranslationSystem->getCompleteTranslationsString(GROUPID, NOT_ENOUGH_CREDITS, player->GetSession()->GetPlayer());
				creature->Say(notenoughcredit, LANG_UNIVERSAL, nullptr);
				return true;
			}break;

			//RaceChange
		case 4:
			if (player->HasItemCount(vipCoin, 1)) {
				player->SetAtLoginFlag(AT_LOGIN_RENAME);
				player->DestroyItemCount(49426, 4, true, false);
				std::ostringstream ss;
				DBeintrag(player->GetSession()->GetPlayer(), "Rename of Character");
				ss << "|cff54b5ffEine Namensaenderung wurde durchgefuehrt von : |r " << ChatHandler(player->GetSession()).GetNameLink();
				sWorld->SendGMText(LANG_GM_BROADCAST, ss.str().c_str());


				ChatHandler(player->GetSession()).PSendSysMessage("%s", logoutnow,
					player->GetName());
				return true;

			}

			else{

				std::string notenoughcredit = TranslationSystem->getCompleteTranslationsString(GROUPID, NOT_ENOUGH_CREDITS, player->GetSession()->GetPlayer());
				creature->Say(notenoughcredit, LANG_UNIVERSAL, nullptr);
				return true;
			}break;

		}
		return true;
	}
};

void AddSC_Race_Change_NPC()
{
	new Race_Change_NPC();
}