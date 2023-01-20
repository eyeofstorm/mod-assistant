#include "Chat.h"
#include "Config.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

enum GossipId
{
    ASSISTANT_GOSSIP_TEXT = 48,

    ASSISTANT_GOSSIP_HEIRLOOM = 100,
    ASSISTANT_GOSSIP_GLYPH = 200,
    ASSISTANT_GOSSIP_GEM = 400,
    ASSISTANT_GOSSIP_CONTAINER = 500,
    ASSISTANT_GOSSIP_UTILITIES = 600,
    ASSISTANT_GOSSIP_PROFESSIONS = 700,
};

enum VendorId
{
    ASSISTANT_VENDOR_HEIRLOOM_WEAPON = 9000000,
    ASSISTANT_VENDOR_HEIRLOOM_ARMOR = 9000001,
    ASSISTANT_VENDOR_HEIRLOOM_OTHER = 9000002,
    ASSISTANT_VENDOR_GLYPH = 9000003,
    ASSISTANT_VENDOR_GEM = 9000023,
    ASSISTANT_VENDOR_CONTAINER = 9000030,
};

enum Profession
{
    PROFESSION_LEVEL_APPRENTICE = 75,
    PROFESSION_LEVEL_JOURNEYMAN = 150,
    PROFESSION_LEVEL_EXPERT = 225,
    PROFESSION_LEVEL_ARTISAN = 300,
    PROFESSION_LEVEL_MASTER = 375,
    PROFESSION_LEVEL_GRAND_MASTER = 450,
};

enum CreatureDisplayID
{
    CREATURE_DISP_ID_FELTHAS = 27545, 
};

// Vendors
bool aEnableHeirlooms;
bool aEnableGlyphs;
bool aEnableGems;
bool aEnableContainers;

// Utilities
bool aEnableUtilities;
uint32 aCostNameChange;
uint32 aCostCustomization;
uint32 aCostRaceChange;
uint32 aCostFactionChange;

// Professions
bool aEnableApprenticeProfession;
bool aEnableJourneymanProfession;
bool aEnableExpertProfession;
bool aEnableArtisanProfession;
bool aEnableMasterProfession;
bool aEnableGrandMasterProfession;
uint32 aCostApprenticeProfession;
uint32 aCostJourneymanProfession;
uint32 aCostExpertProfession;
uint32 aCostArtisanProfession;
uint32 aCostMasterProfession;
uint32 aCostGrandMasterProfession;

// NPC duration
uint32 aNpcDuration;

class PlayerHook : public PlayerScript
{
public:
    PlayerHook() : PlayerScript("mod_assistant_player_hook") {}

    void OnLogin(Player* player)
    {
        ChatHandler(player->GetSession()).SendSysMessage("This server is running mod_assistant module...");

        if (strcmp(player->GetName().c_str(), "Felthas") == 0)
        {
            player->SetDisplayId(CREATURE_DISP_ID_FELTHAS);
        }
    }

    // Called when a player switches to a new area (more accurate than UpdateZone)
    void OnUpdateArea(Player* player, uint32 oldArea, uint32 newArea)
    {
        if (!player) return;

        if (strcmp(player->GetName().c_str(), "Felthas") == 0)
        {
            if (player->GetDisplayId() != 27545)
            {
                player->SetDisplayId(CREATURE_DISP_ID_FELTHAS);
            }
        }
    }
};

class Assistant : public CreatureScript
{
public:
    Assistant() : CreatureScript("npc_assistant") {}

    bool OnGossipHello(Player* player, Creature* creature)
    {
        ClearGossipMenuFor(player);

        if (aEnableHeirlooms)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want heirlooms", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_HEIRLOOM);

        if (aEnableGlyphs)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want glyphs", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GLYPH);

        if (aEnableGems)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM);

        if (aEnableContainers)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want containers", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_CONTAINER);

        if (aEnableUtilities)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want utilities", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES);

        if (hasValidProfession(player))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "I want help with my professions", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS);

        SendGossipMenuFor(player, ASSISTANT_GOSSIP_TEXT, creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (sender != GOSSIP_SENDER_MAIN)
            return false;

        if (action == 1)
        {
            OnGossipHello(player, creature);
        }
        else if (action == ASSISTANT_GOSSIP_HEIRLOOM)
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want weapons", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_HEIRLOOM + 1);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want armor", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_HEIRLOOM + 2);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want something else", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_HEIRLOOM + 3);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Previous Page", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(player, ASSISTANT_GOSSIP_TEXT, creature->GetGUID());
        }
        else if (action == ASSISTANT_GOSSIP_HEIRLOOM + 1)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_HEIRLOOM_WEAPON);
        }
        else if (action == ASSISTANT_GOSSIP_HEIRLOOM + 2)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_HEIRLOOM_ARMOR);
        }
        else if (action == ASSISTANT_GOSSIP_HEIRLOOM + 3)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_HEIRLOOM_OTHER);
        }
        else if (action == ASSISTANT_GOSSIP_GLYPH)
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some major glyphs", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GLYPH + 1);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some minor glyphs", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GLYPH + 2);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Previous Page", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(player, ASSISTANT_GOSSIP_TEXT, creature->GetGUID());
        }
        else if (action == ASSISTANT_GOSSIP_GLYPH + 1)
        {
            switch (player->getClass())
            {
            case CLASS_WARRIOR:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH);
                break;
            case CLASS_PALADIN:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 1);
                break;
            case CLASS_HUNTER:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 2);
                break;
            case CLASS_ROGUE:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 3);
                break;
            case CLASS_PRIEST:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 4);
                break;
            case CLASS_DEATH_KNIGHT:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 5);
                break;
            case CLASS_SHAMAN:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 6);
                break;
            case CLASS_MAGE:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 7);
                break;
            case CLASS_WARLOCK:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 8);
                break;
            case CLASS_DRUID:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 9);
                break;
            }
        }
        else if (action == ASSISTANT_GOSSIP_GLYPH + 2)
        {
            switch (player->getClass())
            {
            case CLASS_WARRIOR:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 10);
                break;
            case CLASS_PALADIN:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 11);
                break;
            case CLASS_HUNTER:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 12);
                break;
            case CLASS_ROGUE:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 13);
                break;
            case CLASS_PRIEST:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 14);
                break;
            case CLASS_DEATH_KNIGHT:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 15);
                break;
            case CLASS_SHAMAN:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 16);
                break;
            case CLASS_MAGE:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 17);
                break;
            case CLASS_WARLOCK:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 18);
                break;
            case CLASS_DRUID:
                player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GLYPH + 19);
                break;
            }
        }
        else if (action == ASSISTANT_GOSSIP_GEM)
        {
            ClearGossipMenuFor(player);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some meta gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 1);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some red gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 2);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some blue gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 3);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some yellow gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 4);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some purple gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 5);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some green gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 6);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want some orange gems", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_GEM + 7);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Previous Page", GOSSIP_SENDER_MAIN, 1);

            SendGossipMenuFor(player, ASSISTANT_GOSSIP_TEXT, creature->GetGUID());
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 1)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM);
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 2)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM + 1);
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 3)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM + 2);
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 4)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM + 3);
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 5)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM + 4);
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 6)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM + 5);
        }
        else if (action == ASSISTANT_GOSSIP_GEM + 7)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_GEM + 6);
        }
        else if (action == ASSISTANT_GOSSIP_CONTAINER)
        {
            player->GetSession()->SendListInventory(creature->GetGUID(), ASSISTANT_VENDOR_CONTAINER);
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES)
        {
            ClearGossipMenuFor(player);

            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to change my name",       GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  1, "Do you wish to continue the transaction?", aCostNameChange, false);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to change my appearance", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  2, "Do you wish to continue the transaction?", aCostCustomization, false);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to change my race",       GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  3, "Do you wish to continue the transaction?", aCostRaceChange, false);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to change my faction",    GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  4, "Do you wish to continue the transaction?", aCostFactionChange, false);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to reset my instances",   GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  5);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to reset my cooldowns",   GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  6);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to remove my sickness",   GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  7);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to repair my items",      GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  8);
            AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to open bank",            GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES +  9);
            // AddGossipItemFor(player, GOSSIP_ICON_VENDOR, "I want to open mailbox",         GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES + 10);

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Previous Page", GOSSIP_SENDER_MAIN, 1);

            SendGossipMenuFor(player, ASSISTANT_GOSSIP_TEXT, creature->GetGUID());
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 1)
        {
            if (hasLoginFlag(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You have to complete the previously activated feature before trying to perform another.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES);
            }
            else
            {
                player->ModifyMoney(-aCostNameChange);
                player->SetAtLoginFlag(AT_LOGIN_RENAME);
                ChatHandler(player->GetSession()).PSendSysMessage("You can now log out to apply the name change.");
                CloseGossipMenuFor(player);
            }
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 2)
        {
            if (hasLoginFlag(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You have to complete the previously activated feature before trying to perform another.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES);
            }
            else
            {
                player->ModifyMoney(-aCostCustomization);
                player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);
                ChatHandler(player->GetSession()).PSendSysMessage("You can now log out to apply the customization.");
                CloseGossipMenuFor(player);
            }
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 3)
        {
            if (hasLoginFlag(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You have to complete the previously activated feature before trying to perform another.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES);
            }
            else
            {
                player->ModifyMoney(-aCostRaceChange);
                player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);
                ChatHandler(player->GetSession()).PSendSysMessage("You can now log out to apply the race change.");
                CloseGossipMenuFor(player);
            }
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 4)
        {
            if (hasLoginFlag(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You have to complete the previously activated feature before trying to perform another.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_UTILITIES);
            }
            else
            {
                player->ModifyMoney(-aCostFactionChange);
                player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);
                ChatHandler(player->GetSession()).PSendSysMessage("You can now log out to apply the faction change.");
                CloseGossipMenuFor(player);
            }
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 5)
        {
            ClearGossipMenuFor(player);

            // reset instance
            for (uint8 i = 0; i < MAX_DIFFICULTY; ++i)
            {
                BoundInstancesMap const& m_boundInstances = sInstanceSaveMgr->PlayerGetBoundInstances(player->GetGUID(), Difficulty(i));

                for (BoundInstancesMap::const_iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
                {
                    if (itr->first != player->GetMapId())
                    {
                        sInstanceSaveMgr->PlayerUnbindInstance(player->GetGUID(), itr->first, Difficulty(i), true, player);
                        itr = m_boundInstances.begin();
                    }
                    else
                    {
                        ++itr;
                    }
                }
            }

            player->GetSession()->SendNotification("|cffFFFF00NPC SERVICES \n |cffFFFFFFInstances succesfully reseted!");
            player->CastSpell(player, 59908);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 6)
        {
            ClearGossipMenuFor(player);

            // reset cooldowns
            if (player->IsInCombat())
            {
                player->GetSession()->SendNotification("You are in combat!");
                return false;
            }

            player->RemoveAllSpellCooldown();
            player->GetSession()->SendNotification("|cffFFFF00NPC SERVICES \n |cffFFFFFFCooldowns succesfully reseted!");
            player->CastSpell(player, 31726);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 7)
        {
            ClearGossipMenuFor(player);

            // remove sickness
            if(player->HasAura(15007))
            {
				player->RemoveAura(15007);
            }

            player->GetSession()->SendNotification("|cffFFFF00NPC SERVICES \n |cffFFFFFFSickness succesfully removed!");
            player->CastSpell(player, 31726);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 8)
        {
            ClearGossipMenuFor(player);

            // repair items
            player->DurabilityRepairAll(false, 0, false);
            player->GetSession()->SendNotification("|cffFFFF00NPC SERVICES \n |cffFFFFFFItems repaired succesfully!");
            player->CastSpell(player, 31726);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_UTILITIES + 9)
        {
            ClearGossipMenuFor(player);

            // open bank
            player->GetSession()->SendShowBank(player->GetGUID());
        }
        // else if (action == ASSISTANT_GOSSIP_UTILITIES + 10)
        // {
        //     ClearGossipMenuFor(player);

        //     // open mailbox
        //     player->GetSession()->SendShowMailBox(player->GetGUID());
        // }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS)
        {
            ClearGossipMenuFor(player);

            if (isValidProfession(player, SKILL_FIRST_AID))
            {
                uint32 cost = getProfessionCost(player, SKILL_FIRST_AID);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in First Aid", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 1, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_BLACKSMITHING))
            {
                uint32 cost = getProfessionCost(player, SKILL_BLACKSMITHING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Blacksmithing", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 2, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_LEATHERWORKING))
            {
                uint32 cost = getProfessionCost(player, SKILL_LEATHERWORKING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Leatherworking", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 3, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_ALCHEMY))
            {
                uint32 cost = getProfessionCost(player, SKILL_ALCHEMY);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Alchemy", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 4, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_HERBALISM))
            {
                uint32 cost = getProfessionCost(player, SKILL_HERBALISM);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Herbalism", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 5, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_COOKING))
            {
                uint32 cost = getProfessionCost(player, SKILL_COOKING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Cooking", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 6, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_MINING))
            {
                uint32 cost = getProfessionCost(player, SKILL_MINING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Mining", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 7, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_TAILORING))
            {
                uint32 cost = getProfessionCost(player, SKILL_TAILORING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Tailoring", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 8, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_ENGINEERING))
            {
                uint32 cost = getProfessionCost(player, SKILL_ENGINEERING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Engineering", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 9, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_ENCHANTING))
            {
                uint32 cost = getProfessionCost(player, SKILL_ENCHANTING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Enchanting", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 10, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_FISHING))
            {
                uint32 cost = getProfessionCost(player, SKILL_FISHING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Fishing", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 11, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_SKINNING))
            {
                uint32 cost = getProfessionCost(player, SKILL_SKINNING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Skinning", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 12, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_INSCRIPTION))
            {
                uint32 cost = getProfessionCost(player, SKILL_INSCRIPTION);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Inscription", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 13, "Do you wish to continue the transaction?", cost, false);
            }

            if (isValidProfession(player, SKILL_JEWELCRAFTING))
            {
                uint32 cost = getProfessionCost(player, SKILL_JEWELCRAFTING);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "I want help with my skill in Jewelcrafting", GOSSIP_SENDER_MAIN, ASSISTANT_GOSSIP_PROFESSIONS + 14, "Do you wish to continue the transaction?", cost, false);
            }

            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Previous Page", GOSSIP_SENDER_MAIN, 1);
            SendGossipMenuFor(player, ASSISTANT_GOSSIP_TEXT, creature->GetGUID());
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 1)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_FIRST_AID);
            uint32 professionCost = getProfessionCost(player, SKILL_FIRST_AID);

            player->SetSkill(SKILL_FIRST_AID, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 2)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_BLACKSMITHING);
            uint32 professionCost = getProfessionCost(player, SKILL_BLACKSMITHING);

            player->SetSkill(SKILL_BLACKSMITHING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 3)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_LEATHERWORKING);
            uint32 professionCost = getProfessionCost(player, SKILL_LEATHERWORKING);

            player->SetSkill(SKILL_LEATHERWORKING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 4)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_ALCHEMY);
            uint32 professionCost = getProfessionCost(player, SKILL_ALCHEMY);

            player->SetSkill(SKILL_ALCHEMY, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 5)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_HERBALISM);
            uint32 professionCost = getProfessionCost(player, SKILL_HERBALISM);

            player->SetSkill(SKILL_HERBALISM, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 6)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_COOKING);
            uint32 professionCost = getProfessionCost(player, SKILL_COOKING);

            player->SetSkill(SKILL_COOKING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 7)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_MINING);
            uint32 professionCost = getProfessionCost(player, SKILL_MINING);

            player->SetSkill(SKILL_MINING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 8)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_TAILORING);
            uint32 professionCost = getProfessionCost(player, SKILL_TAILORING);

            player->SetSkill(SKILL_TAILORING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 9)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_ENGINEERING);
            uint32 professionCost = getProfessionCost(player, SKILL_ENGINEERING);

            player->SetSkill(SKILL_ENGINEERING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 10)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_ENCHANTING);
            uint32 professionCost = getProfessionCost(player, SKILL_ENCHANTING);

            player->SetSkill(SKILL_ENCHANTING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 11)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_FISHING);
            uint32 professionCost = getProfessionCost(player, SKILL_FISHING);

            player->SetSkill(SKILL_FISHING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 12)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_SKINNING);
            uint32 professionCost = getProfessionCost(player, SKILL_SKINNING);

            player->SetSkill(SKILL_SKINNING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 13)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_INSCRIPTION);
            uint32 professionCost = getProfessionCost(player, SKILL_INSCRIPTION);

            player->SetSkill(SKILL_INSCRIPTION, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }
        else if (action == ASSISTANT_GOSSIP_PROFESSIONS + 14)
        {
            ClearGossipMenuFor(player);

            uint16 maxSkillValue = player->GetPureMaxSkillValue(SKILL_JEWELCRAFTING);
            uint32 professionCost = getProfessionCost(player, SKILL_JEWELCRAFTING);

            player->SetSkill(SKILL_JEWELCRAFTING, 0, maxSkillValue, maxSkillValue);
            player->ModifyMoney(-professionCost);

            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 1);
        }

        return true;
    }

private:
    bool hasLoginFlag(Player* player)
    {
        if (player->HasAtLoginFlag(AT_LOGIN_RENAME) || player->HasAtLoginFlag(AT_LOGIN_CUSTOMIZE) || player->HasAtLoginFlag(AT_LOGIN_CHANGE_RACE) || player->HasAtLoginFlag(AT_LOGIN_CHANGE_FACTION))
            return true;

        return false;
    }

    bool hasValidProfession(Player* player)
    {
        if (aEnableApprenticeProfession || aEnableJourneymanProfession || aEnableExpertProfession || aEnableArtisanProfession || aEnableMasterProfession || aEnableGrandMasterProfession)
            if (isValidProfession(player, SKILL_FIRST_AID) || isValidProfession(player, SKILL_BLACKSMITHING) || isValidProfession(player, SKILL_LEATHERWORKING) || isValidProfession(player, SKILL_ALCHEMY) || isValidProfession(player, SKILL_HERBALISM) || isValidProfession(player, SKILL_COOKING) || isValidProfession(player, SKILL_MINING) || isValidProfession(player, SKILL_TAILORING) || isValidProfession(player, SKILL_ENGINEERING) || isValidProfession(player, SKILL_ENCHANTING) || isValidProfession(player, SKILL_FISHING) || isValidProfession(player, SKILL_SKINNING) || isValidProfession(player, SKILL_INSCRIPTION) || isValidProfession(player, SKILL_JEWELCRAFTING))
                return true;

        return false;
    }

    bool isValidProfession(Player* player, uint32 skillId)
    {
        if (player->HasSkill(skillId) && ((player->GetPureSkillValue(skillId) < PROFESSION_LEVEL_APPRENTICE && player->GetPureMaxSkillValue(skillId) == PROFESSION_LEVEL_APPRENTICE && aEnableApprenticeProfession) || (player->GetPureSkillValue(skillId) < PROFESSION_LEVEL_JOURNEYMAN && player->GetPureMaxSkillValue(skillId) == PROFESSION_LEVEL_JOURNEYMAN && aEnableJourneymanProfession) || (player->GetPureSkillValue(skillId) < PROFESSION_LEVEL_EXPERT && player->GetPureMaxSkillValue(skillId) == PROFESSION_LEVEL_EXPERT && aEnableExpertProfession) || (player->GetPureSkillValue(skillId) < PROFESSION_LEVEL_ARTISAN && player->GetPureMaxSkillValue(skillId) == PROFESSION_LEVEL_ARTISAN && aEnableArtisanProfession) || (player->GetPureSkillValue(skillId) < PROFESSION_LEVEL_MASTER && player->GetPureMaxSkillValue(skillId) == PROFESSION_LEVEL_MASTER && aEnableMasterProfession) || (player->GetPureSkillValue(skillId) < PROFESSION_LEVEL_GRAND_MASTER && player->GetPureMaxSkillValue(skillId) == PROFESSION_LEVEL_GRAND_MASTER && aEnableGrandMasterProfession)))
            return true;

        return false;
    }

    int getProfessionCost(Player* player, uint32 skill)
    {
        int cost = 0;

        if (player->GetPureMaxSkillValue(skill) == PROFESSION_LEVEL_APPRENTICE)
        {
            cost = aCostApprenticeProfession;
        }
        else if (player->GetPureMaxSkillValue(skill) == PROFESSION_LEVEL_JOURNEYMAN)
        {
            cost = aCostJourneymanProfession;
        }
        else if (player->GetPureMaxSkillValue(skill) == PROFESSION_LEVEL_EXPERT)
        {
            cost = aCostExpertProfession;
        }
        else if (player->GetPureMaxSkillValue(skill) == PROFESSION_LEVEL_ARTISAN)
        {
            cost = aCostArtisanProfession;
        }
        else if (player->GetPureMaxSkillValue(skill) == PROFESSION_LEVEL_MASTER)
        {
            cost = aCostMasterProfession;
        }
        else if (player->GetPureMaxSkillValue(skill) == PROFESSION_LEVEL_GRAND_MASTER)
        {
            cost = aCostGrandMasterProfession;
        }

        return cost * 10000;
    }
};

class Configuration : public WorldScript
{
public:
    Configuration() : WorldScript("mod_assistant_configuration") {}

    void OnAfterConfigLoad(bool /*reload*/) override
    {
        // NPC duration
        aNpcDuration = sConfigMgr->GetOption<uint32>("Assistant.NpcDuration", 60) * IN_MILLISECONDS;

        // Vendors
        aEnableHeirlooms = sConfigMgr->GetOption<bool>("Assistant.Heirlooms", 1);
        aEnableGlyphs = sConfigMgr->GetOption<bool>("Assistant.Glyphs", 1);
        aEnableGems = sConfigMgr->GetOption<bool>("Assistant.Gems", 1);
        aEnableContainers = sConfigMgr->GetOption<bool>("Assistant.Containers", 1);

        // Utilities
        aEnableUtilities = sConfigMgr->GetOption<bool>("Assistant.Utilities", 1);
        aCostNameChange = sConfigMgr->GetOption<uint32>("Assistant.Utilities.NameChange", 10) * 10000;
        aCostCustomization = sConfigMgr->GetOption<uint32>("Assistant.Utilities.Customization", 50) * 10000;
        aCostRaceChange = sConfigMgr->GetOption<uint32>("Assistant.Utilities.RaceChange", 500) * 10000;
        aCostFactionChange = sConfigMgr->GetOption<uint32>("Assistant.Utilities.FactionChange", 1000) * 10000;

        // Professions
        aEnableApprenticeProfession = sConfigMgr->GetOption<bool>("Assistant.Professions.Apprentice.Enabled", 1);
        aEnableJourneymanProfession = sConfigMgr->GetOption<bool>("Assistant.Professions.Journeyman.Enabled", 1);
        aEnableExpertProfession = sConfigMgr->GetOption<bool>("Assistant.Professions.Expert.Enabled", 1);
        aEnableArtisanProfession = sConfigMgr->GetOption<bool>("Assistant.Professions.Artisan.Enabled", 1);
        aEnableMasterProfession = sConfigMgr->GetOption<bool>("Assistant.Professions.Master.Enabled", 0);
        aEnableGrandMasterProfession = sConfigMgr->GetOption<bool>("Assistant.Professions.GrandMaster.Enabled", 0);
        aCostApprenticeProfession = sConfigMgr->GetOption<uint32>("Assistant.Professions.Apprentice.Cost", 100);
        aCostJourneymanProfession = sConfigMgr->GetOption<uint32>("Assistant.Professions.Journeyman.Cost", 250);
        aCostExpertProfession = sConfigMgr->GetOption<uint32>("Assistant.Professions.Expert.Cost", 500);
        aCostArtisanProfession = sConfigMgr->GetOption<uint32>("Assistant.Professions.Artisan.Cost", 750);
        aCostMasterProfession = sConfigMgr->GetOption<uint32>("Assistant.Professions.Master.Cost", 1250);
        aCostGrandMasterProfession = sConfigMgr->GetOption<uint32>("Assistant.Professions.GrandMaster.Cost", 2500);
    }
};

void AddSC_mod_assistant()
{
    new PlayerHook();
    new Assistant();
    new Configuration();
}
