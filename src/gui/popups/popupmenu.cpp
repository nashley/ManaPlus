/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2015  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gui/popups/popupmenu.h"

#include "actormanager.h"
#include "configuration.h"
#include "gamemodifiers.h"
#ifdef TMWA_SUPPORT
#include "guildmanager.h"
#endif
#include "item.h"
#include "party.h"
#include "spellmanager.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"
#include "being/playerrelations.h"

#include "input/inputmanager.h"

#include "gui/buttontext.h"
#include "gui/gui.h"
#include "gui/viewport.h"

#include "gui/windows/chatwindow.h"
#include "gui/windows/equipmentwindow.h"
#include "gui/windows/inventorywindow.h"
#include "gui/windows/ministatuswindow.h"
#include "gui/windows/npcdialog.h"
#include "gui/windows/outfitwindow.h"
#include "gui/windows/socialwindow.h"
#include "gui/windows/textcommandeditor.h"
#include "gui/windows/textdialog.h"
#include "gui/windows/tradewindow.h"
#include "gui/windowmenu.h"

#include "gui/widgets/button.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/textfield.h"

#include "gui/widgets/tabs/chat/whispertab.h"

#include "net/adminhandler.h"
#include "net/beinghandler.h"
#include "net/chathandler.h"
#include "net/guildhandler.h"
#ifdef EATHENA_SUPPORT
#include "net/homunculushandler.h"
#include "net/mercenaryhandler.h"
#endif
#include "net/pethandler.h"
#include "net/serverfeatures.h"

#include "resources/chatobject.h"
#include "resources/iteminfo.h"
#include "resources/mapitemtype.h"

#include "resources/db/npcdb.h"

#include "resources/map/map.h"
#include "resources/map/mapitem.h"
#include "resources/map/speciallayer.h"

#include "utils/copynpaste.h"
#include "utils/gettext.h"
#include "utils/process.h"

#include "debug.h"

std::string tradePartnerName;

PopupMenu *popupMenu = nullptr;

PopupMenu::PopupMenu() :
    Popup("PopupMenu", "popupmenu.xml"),
    mBrowserBox(new BrowserBox(this, BrowserBox::AUTO_SIZE, true,
        "popupbrowserbox.xml")),
    mScrollArea(nullptr),
    mBeingId(0),
    mFloorItemId(0),
    mItem(nullptr),
    mItemId(0),
    mItemColor(1),
    mMapItem(nullptr),
    mTab(nullptr),
    mSpell(nullptr),
    mWindow(nullptr),
    mRenameListener(),
    mPlayerListener(),
    mDialog(nullptr),
    mButton(nullptr),
    mNick(),
    mTextField(nullptr),
    mType(ActorType::Unknown),
    mX(0),
    mY(0)
{
    mBrowserBox->setOpaque(false);
    mBrowserBox->setLinkHandler(this);
    mRenameListener.setMapItem(nullptr);
    mRenameListener.setDialog(nullptr);
    mPlayerListener.setNick("");
    mPlayerListener.setDialog(nullptr);
    mPlayerListener.setType(ActorType::Unknown);
    mScrollArea = new ScrollArea(this, mBrowserBox, false);
    mScrollArea->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
}

void PopupMenu::postInit()
{
    add(mScrollArea);
}

void PopupMenu::showPopup(const int x, const int y, const Being *const being)
{
    if (!being || !localPlayer || !actorManager)
        return;

    mBeingId = being->getId();
    mNick = being->getName();
    mType = being->getType();
    mBrowserBox->clearRows();
    mX = x;
    mY = y;

    const std::string &name = mNick;
    mBrowserBox->addRow(name + being->getGenderSignWithSpace());

    switch (being->getType())
    {
        case ActorType::Player:
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: trade with player
            mBrowserBox->addRow("/trade 'NAME'", _("Trade"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: trade attack player
            mBrowserBox->addRow("/attack 'NAME'", _("Attack"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: send whisper to player
            mBrowserBox->addRow("/whispertext 'NAME'", _("Whisper"));
            addGmCommands();
            mBrowserBox->addRow("##3---");

            // TRANSLATORS: popup menu item
            // TRANSLATORS: heal player
            mBrowserBox->addRow("/heal :'BEINGID'", _("Heal"));
            mBrowserBox->addRow("##3---");

            addPlayerRelation(name);
            mBrowserBox->addRow("##3---");

            addFollow();
            addPartyName(being->getPartyName());

            const Guild *const guild1 = being->getGuild();
            const Guild *const guild2 = localPlayer->getGuild();
            if (guild2)
            {
                if (guild1)
                {
                    if (guild1->getId() == guild2->getId())
                    {
                        mBrowserBox->addRow("/kickguild 'NAME'",
                            // TRANSLATORS: popup menu item
                            // TRANSLATORS: kick player from guild
                            _("Kick from guild"));
                        if (guild2->getServerGuild())
                        {
                            mBrowserBox->addRow(strprintf(
                                "@@guild-pos|%s >@@",
                                // TRANSLATORS: popup menu item
                                // TRANSLATORS: change player position in guild
                                _("Change pos in guild")));
                        }
                    }
                }
                else if (guild2->getMember(mNick))
                {
                    mBrowserBox->addRow("/kickguild 'NAME'",
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: kick player from guild
                        _("Kick from guild"));
                    if (guild2->getServerGuild())
                    {
                        mBrowserBox->addRow(strprintf(
                            "@@guild-pos|%s >@@",
                            // TRANSLATORS: popup menu item
                            // TRANSLATORS: change player position in guild
                            _("Change pos in guild")));
                    }
                }
                else
                {
#ifdef TMWA_SUPPORT
                    if (guild2->getServerGuild()
                        || (guildManager && guildManager->havePower()))
#endif
                    {
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: invite player to guild
                        mBrowserBox->addRow("/guild 'NAME'",
                            _("Invite to guild"));
                    }
                }
            }

            // TRANSLATORS: popup menu item
            // TRANSLATORS: set player invisible for self by id
            mBrowserBox->addRow("/nuke 'NAME'", _("Nuke"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: move to player location
            mBrowserBox->addRow("/navigateto 'NAME'", _("Move"));
            addPlayerMisc();
            addBuySell(being);
#ifdef EATHENA_SUPPORT
            addChat(being);
#endif
            break;
        }

        case ActorType::Npc:
            if (!addBeingMenu())
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: talk with npc
                mBrowserBox->addRow("/talk 'NAME'", _("Talk"));
                if (serverFeatures->haveNpcWhispers())
                {
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: whisper to npc
                    mBrowserBox->addRow("/whispertext NPC:'NAME'",
                        _("Whisper"));
                }
                // TRANSLATORS: popup menu item
                // TRANSLATORS: buy from npc
                mBrowserBox->addRow("/buy 'NAME'", _("Buy"));
                // TRANSLATORS: popup menu item
                // TRANSLATORS: sell to npc
                mBrowserBox->addRow("/sell 'NAME'", _("Sell"));
            }

            mBrowserBox->addRow("##3---");
            // TRANSLATORS: popup menu item
            // TRANSLATORS: move to npc location
            mBrowserBox->addRow("/navigateto 'NAME'", _("Move"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add comment to npc
            mBrowserBox->addRow("addcomment", _("Add comment"));
#ifdef EATHENA_SUPPORT
            addChat(being);
#endif
            break;

        case ActorType::Monster:
        {
            // Monsters can be attacked
            // TRANSLATORS: popup menu item
            // TRANSLATORS: attack monster
            mBrowserBox->addRow("/attack :'BEINGID'", _("Attack"));

            if (config.getBoolValue("enableAttackFilter"))
            {
                mBrowserBox->addRow("##3---");
                if (actorManager->isInAttackList(name)
                    || actorManager->isInIgnoreAttackList(name)
                    || actorManager->isInPriorityAttackList(name))
                {
                    mBrowserBox->addRow("/removeattack 'NAME'",
                        // TRANSLATORS: remove monster from attack list
                        // TRANSLATORS: popup menu item
                        _("Remove from attack list"));
                }
                else
                {
                    mBrowserBox->addRow("/addpriorityattack 'NAME'",
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: add monster to priotiry attack list
                        _("Add to priority attack list"));
                    mBrowserBox->addRow("/addattack 'NAME'",
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: add monster to attack list
                        _("Add to attack list"));
                    mBrowserBox->addRow("/addignoreattack 'NAME'",
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: add monster to ignore list
                        _("Add to ignore list"));
                }
            }
            break;
        }

#ifdef EATHENA_SUPPORT
        case ActorType::Mercenary:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: Mercenary move to master
            mBrowserBox->addRow("mercenary to master", _("Move to master"));
            mBrowserBox->addRow("##3---");
            // TRANSLATORS: popup menu item
            // TRANSLATORS: fire mercenary
            mBrowserBox->addRow("fire mercenary", _("Fire"));
            mBrowserBox->addRow("##3---");
            break;

        case ActorType::Homunculus:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: Mercenary move to master
            mBrowserBox->addRow("homunculus to master", _("Move to master"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: feed homunculus
            mBrowserBox->addRow("homunculus feed", _("Feed"));
            mBrowserBox->addRow("##3---");
            // TRANSLATORS: popup menu item
            // TRANSLATORS: delete homunculus
            mBrowserBox->addRow("homunculus delete", _("Kill"));
            mBrowserBox->addRow("##3---");
            break;

        case ActorType::Pet:
            if (being->getOwner() == localPlayer)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: feed pet
                mBrowserBox->addRow("pet feed", _("Feed"));
                // TRANSLATORS: popup menu item
                // TRANSLATORS: pet drop loot
                mBrowserBox->addRow("pet drop loot", _("Drop loot"));
                // TRANSLATORS: popup menu item
                // TRANSLATORS: pet unequip item
                mBrowserBox->addRow("pet unequip", _("Unequip"));
                mBrowserBox->addRow("##3---");
                // TRANSLATORS: popup menu item
                // TRANSLATORS: pet rename item
                mBrowserBox->addRow("/setpetname", _("Rename"));
                mBrowserBox->addRow("##3---");
                // TRANSLATORS: popup menu item
                // TRANSLATORS: pet return to egg
                mBrowserBox->addRow("pet to egg", _("Return to egg"));
                mBrowserBox->addRow("##3---");
            }
            break;
#endif
        case ActorType::Avatar:
        case ActorType::Unknown:
        case ActorType::FloorItem:
        case ActorType::Portal:
        case ActorType::LocalPet:
        default:
            break;
    }
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add being name to chat
    mBrowserBox->addRow("/addtext 'NAME'", _("Add name to chat"));
    mBrowserBox->addRow("##3---");

    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

bool PopupMenu::addBeingMenu()
{
    Being *being = actorManager->findBeing(mBeingId);
    if (!being)
        return false;

    BeingInfo *const info = NPCDB::get(being->getSubType());
    if (!info)
        return false;

    const std::vector<BeingMenuItem> &menu = info->getMenu();
    FOR_EACH (std::vector<BeingMenuItem>::const_iterator, it, menu)
    {
        const BeingMenuItem &item = *it;
        mBrowserBox->addRow("/" + item.command, item.name.c_str());
    }
    return true;
}

void PopupMenu::setMousePos()
{
    if (viewport)
    {
        mX = viewport->mMouseX;
        mY = viewport->mMouseY;
    }
    else
    {
        Gui::getMouseState(&mX, &mY);
    }
}

void PopupMenu::showPopup(const int x, const int y,
                          const std::vector<ActorSprite*> &beings)
{
    mX = x;
    mY = y;
    mBrowserBox->clearRows();
    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Players"));
    FOR_EACH (std::vector<ActorSprite*>::const_iterator, it, beings)
    {
        const Being *const being = dynamic_cast<const Being*>(*it);
        const ActorSprite *const actor = *it;
        if (being && !being->getName().empty())
        {
            mBrowserBox->addRow(strprintf("@@player_%u|%s >@@",
                static_cast<unsigned>(being->getId()), (being->getName()
                + being->getGenderSignWithSpace()).c_str()));
        }
        else if (actor->getType() == ActorType::FloorItem)
        {
            const FloorItem *const floorItem
                = static_cast<const FloorItem*>(actor);
            mBrowserBox->addRow(strprintf("@@flooritem_%u|%s >@@",
                static_cast<unsigned>(actor->getId()),
                floorItem->getName().c_str()));
        }
    }
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));
    showPopup(x, y);
}

void PopupMenu::showPlayerPopup(const std::string &nick)
{
    if (nick.empty() || !localPlayer)
        return;

    setMousePos();
    mNick = nick;
    mBeingId = 0;
    mType = ActorType::Player;
    mBrowserBox->clearRows();

    const std::string &name = mNick;

    mBrowserBox->addRow(name);

    // TRANSLATORS: popup menu item
    // TRANSLATORS: send whisper to player
    mBrowserBox->addRow("/whispertext 'NAME'", _("Whisper"));
    addGmCommands();
    mBrowserBox->addRow("##3---");

    addPlayerRelation(name);
    mBrowserBox->addRow("##3---");

    addFollow();
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add comment to player
    mBrowserBox->addRow("addcomment", _("Add comment"));

    if (localPlayer->isInParty())
    {
        const Party *const party = localPlayer->getParty();
        if (party)
        {
            const PartyMember *const member = party->getMember(mNick);
            if (member)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: kick player from party
                mBrowserBox->addRow("/kickparty 'NAME'", _("Kick from party"));
                mBrowserBox->addRow("##3---");
                const PartyMember *const o = party->getMember(
                    localPlayer->getName());
                if (o && member->getMap() == o->getMap())
                {
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: move to player position
                    mBrowserBox->addRow("/navigate 'X' 'Y'", _("Move"));
                }
            }
        }
    }

    const Guild *const guild2 = localPlayer->getGuild();
    if (guild2)
    {
        if (guild2->getMember(mNick))
        {
#ifdef TMWA_SUPPORT
            if (guild2->getServerGuild() || (guildManager
                && guildManager->havePower()))
#endif
            {
                mBrowserBox->addRow("/kickguild 'NAME'",
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: kick player from guild
                    _("Kick from guild"));
            }
            if (guild2->getServerGuild())
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: change player position in guild
                mBrowserBox->addRow(strprintf(
                    "@@guild-pos|%s >@@", _("Change pos in guild")));
            }
        }
        else
        {
#ifdef TMWA_SUPPORT
            if (guild2->getServerGuild() || (guildManager
                && guildManager->havePower()))
#endif
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: invite player to guild
                mBrowserBox->addRow("/guild 'NAME'", _("Invite to guild"));
            }
        }
    }

    addBuySellDefault();
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add player name to chat
    mBrowserBox->addRow("/addtext 'NAME'", _("Add name to chat"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(mX, mY);
}

void PopupMenu::showPopup(const int x, const int y,
                          const FloorItem *const floorItem)
{
    if (!floorItem)
        return;

    mX = x;
    mY = y;
    mFloorItemId = floorItem->getId();
    mType = ActorType::FloorItem;
    mBrowserBox->clearRows();
    const std::string name = floorItem->getName();
    mNick = name;

    mBrowserBox->addRow(name);

    if (config.getBoolValue("enablePickupFilter"))
    {
        if (actorManager->isInPickupList(name)
            || (actorManager->isInPickupList("")
            && !actorManager->isInIgnorePickupList(name)))
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: pickup item from ground
            mBrowserBox->addRow("/pickup 'FLOORID'", _("Pick up"));
            mBrowserBox->addRow("##3---");
        }
        addPickupFilter(name);
    }
    else
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: pickup item from ground
        mBrowserBox->addRow("/pickup 'FLOORID'", _("Pick up"));
    }
    addProtection();
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add item name to chat
    mBrowserBox->addRow("/addchat 'FLOORID'", _("Add to chat"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(mX, mY);
}

void PopupMenu::showPopup(const int x, const int y, MapItem *const mapItem)
{
    if (!mapItem)
        return;

    mMapItem = mapItem;
    mX = x;
    mY = y;

    mBrowserBox->clearRows();

    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Map Item"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: rename map item
    mBrowserBox->addRow("rename map", _("Rename"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: remove map item
    mBrowserBox->addRow("remove map", _("Remove"));

    if (localPlayer && localPlayer->isGM())
    {
        mBrowserBox->addRow("##3---");
        // TRANSLATORS: popup menu item
        // TRANSLATORS: warp to map item
        mBrowserBox->addRow("/warp 'MAPX' 'MAPY'", _("Warp"));
    }
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showMapPopup(const int x, const int y,
                             const int x2, const int y2)
{
    mX = x2;
    mY = y2;

    mBrowserBox->clearRows();

    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Map Item"));

    if (localPlayer && localPlayer->isGM())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: warp to map item
        mBrowserBox->addRow("/warp 'MAPX' 'MAPY'", _("Warp"));
    }
    // TRANSLATORS: popup menu item
    // TRANSLATORS: move to map item
    mBrowserBox->addRow("/navigate 'X' 'Y'", _("Move"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: move camera to map item
    mBrowserBox->addRow("/movecamera 'X' 'Y'", _("Move camera"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showOutfitsWindowPopup(const int x, const int y)
{
    mX = x;
    mY = y;
    mWindow = outfitWindow;

    mBrowserBox->clearRows();

    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Outfits"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: clear selected outfit
    mBrowserBox->addRow("clear outfit", _("Clear outfit"));
    mBrowserBox->addRow("##3---");

    addWindowMenu(outfitWindow);
    mBrowserBox->addRow("##3---");

    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showSpellPopup(const int x, const int y,
                               TextCommand *const cmd)
{
    if (!cmd)
        return;

    mBrowserBox->clearRows();

    mSpell = cmd;
    mX = x;
    mY = y;

    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Spells"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: edit selected spell
    mBrowserBox->addRow("edit spell", _("Edit spell"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showChatPopup(const int x, const int y, ChatTab *const tab)
{
    if (!tab || !actorManager || !localPlayer)
        return;

    mTab = tab;
    mX = x;
    mY = y;
    mWindow = chatWindow;

    mBrowserBox->clearRows();

    const ChatTabType::Type &type = tab->getType();
    if (type == ChatTabType::WHISPER || type == ChatTabType::CHANNEL)
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: close chat tab
        mBrowserBox->addRow("chat close", _("Close"));
    }

    // TRANSLATORS: popup menu item
    // TRANSLATORS: remove all text from chat tab
    mBrowserBox->addRow("/chatclear", _("Clear"));
    mBrowserBox->addRow("##3---");

    if (tab->getAllowHighlight())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: disable chat tab highlight
        mBrowserBox->addRow("disable highlight", _("Disable highlight"));
    }
    else
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: enable chat tab highlight
        mBrowserBox->addRow("enable highlight", _("Enable highlight"));
    }
    if (tab->getRemoveNames())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: do not remove player names from chat tab
        mBrowserBox->addRow("dont remove name", _("Don't remove name"));
    }
    else
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: remove player names from chat tab
        mBrowserBox->addRow("remove name", _("Remove name"));
    }
    if (tab->getNoAway())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: enable away messages in chat tab
        mBrowserBox->addRow("enable away", _("Enable away"));
    }
    else
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: disable away messages in chat tab
        mBrowserBox->addRow("disable away", _("Disable away"));
    }
    mBrowserBox->addRow("##3---");
    if (tab->getType() == static_cast<int>(ChatTabType::PARTY))
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: enable away messages in chat tab
        mBrowserBox->addRow("/leaveparty", _("Leave"));
        mBrowserBox->addRow("##3---");
    }
    // TRANSLATORS: popup menu item
    // TRANSLATORS: copy selected text to clipboard
    mBrowserBox->addRow("chat clipboard", _("Copy to clipboard"));
    mBrowserBox->addRow("##3---");

    if (type == ChatTabType::WHISPER)
    {
        const WhisperTab *const wTab = static_cast<WhisperTab*>(tab);
        std::string name = wTab->getNick();

        const Being* const being  = actorManager->findBeingByName(
            name, ActorType::Player);

        if (being)
        {
            mBeingId = being->getId();
            mNick = being->getName();
            mType = being->getType();

            // TRANSLATORS: popup menu item
            // TRANSLATORS: trade with player
            mBrowserBox->addRow("/trade 'NAME'", _("Trade"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: attack player
            mBrowserBox->addRow("/attack 'NAME'", _("Attack"));
            mBrowserBox->addRow("##3---");
            // TRANSLATORS: popup menu item
            // TRANSLATORS: heal player
            mBrowserBox->addRow("/heal :'BEINGID'", _("Heal"));
            mBrowserBox->addRow("##3---");
            addPlayerRelation(name);
            mBrowserBox->addRow("##3---");
            addFollow();
            // TRANSLATORS: popup menu item
            // TRANSLATORS: move to player position
            mBrowserBox->addRow("/navigateto 'NAME'", _("Move"));
            addPlayerMisc();
            addBuySell(being);
            mBrowserBox->addRow("##3---");
            addParty(wTab->getNick());
            const Guild *const guild1 = being->getGuild();
            const Guild *const guild2 = localPlayer->getGuild();
            if (guild2)
            {
                if (guild1)
                {
                    if (guild1->getId() == guild2->getId())
                    {
#ifdef TMWA_SUPPORT
                        if (guild2->getServerGuild() || (guildManager
                            && guildManager->havePower()))
#endif
                        {
                            mBrowserBox->addRow("/kickguild 'NAME'",
                                // TRANSLATORS: popup menu item
                                // TRANSLATORS: kick player from guild
                                _("Kick from guild"));
                        }
                        if (guild2->getServerGuild())
                        {
                            // TRANSLATORS: popup menu item
                            // TRANSLATORS: change player position in guild
                            mBrowserBox->addRow(strprintf("@@guild-pos|%s >@@",
                                 _("Change pos in guild")));
                        }
                    }
                }
                else
                {
#ifdef TMWA_SUPPORT
                    if (guild2->getServerGuild() || (guildManager
                        && guildManager->havePower()))
#endif
                    {
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: invite player to guild
                        mBrowserBox->addRow("/guild 'NAME'",
                            _("Invite to guild"));
                    }
                }
            }
        }
        else
        {
            mNick = name;
            mType = ActorType::Player;
            addPlayerRelation(name);
            mBrowserBox->addRow("##3---");
            addFollow();

            if (localPlayer->isInParty())
            {
                const Party *const party = localPlayer->getParty();
                if (party)
                {
                    const PartyMember *const m = party->getMember(mNick);
                    if (m)
                    {
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: move to player location
                        mBrowserBox->addRow("/navigateto 'NAME'", _("Move"));
                    }
                }
            }
            addPlayerMisc();
            addBuySellDefault();
            if (serverFeatures->havePartyNickInvite())
                addParty(wTab->getNick());
            mBrowserBox->addRow("##3---");
        }
    }

    addWindowMenu(chatWindow);
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showChangePos(const int x, const int y)
{
    mBrowserBox->clearRows();
    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Change guild position"));

    if (!localPlayer)
        return;

    mX = x;
    mY = y;
    const Guild *const guild = localPlayer->getGuild();
    if (guild)
    {
        const PositionsMap &map = guild->getPositions();
        FOR_EACH (PositionsMap::const_iterator, itr, map)
        {
            mBrowserBox->addRow(strprintf("@@guild-pos-%u|%s@@",
                itr->first, itr->second.c_str()));
        }
        // TRANSLATORS: popup menu item
        // TRANSLATORS: close menu
        mBrowserBox->addRow("cancel", _("Cancel"));

        showPopup(x, y);
    }
    else
    {
        mBeingId = 0;
        mFloorItemId = 0;
        mItem = nullptr;
        mMapItem = nullptr;
        mNick.clear();
        mType = ActorType::Unknown;
        mX = 0;
        mY = 0;
        setVisible(false);
    }
}

void PopupMenu::showWindowPopup(Window *const window)
{
    if (!window)
        return;

    setMousePos();
    mWindow = window;
    mBrowserBox->clearRows();
    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("window"));

    addWindowMenu(window);

    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(mX, mY);
}

void PopupMenu::addWindowMenu(Window *const window)
{
    if (window->getCloseButton())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: close window
        mBrowserBox->addRow("window close", _("Close"));
    }

    if (window->isStickyButtonLock())
    {
        if (window->isSticky())
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: unlock window
            mBrowserBox->addRow("window unlock", _("Unlock"));
        }
        else
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: lock window
            mBrowserBox->addRow("window lock", _("Lock"));
        }
    }
}

void PopupMenu::handleLink(const std::string &link,
                           MouseEvent *event A_UNUSED)
{
    Being *being = nullptr;
    if (actorManager)
        being = actorManager->findBeing(mBeingId);

    if (link == "chat close" && mTab)
    {
        inputManager.executeChatCommand(InputAction::CLOSE_CHAT_TAB,
            std::string(), mTab);
    }
    else if (link == "remove map" && mMapItem)
    {
        if (viewport)
        {
            const Map *const map = viewport->getMap();
            if (map)
            {
                SpecialLayer *const specialLayer = map->getSpecialLayer();
                if (specialLayer)
                {
                    const bool isHome = (mMapItem->getType()
                        == static_cast<int>(MapItemType::HOME));
                    const int x = static_cast<const int>(mMapItem->getX());
                    const int y = static_cast<const int>(mMapItem->getY());
                    specialLayer->setTile(x, y,
                        static_cast<int>(MapItemType::EMPTY));
                    if (socialWindow)
                        socialWindow->removePortal(x, y);
                    if (isHome && localPlayer)
                    {
                        localPlayer->removeHome();
                        localPlayer->saveHomes();
                    }
                }
            }
        }
    }
    else if (link == "rename map" && mMapItem)
    {
        mRenameListener.setMapItem(mMapItem);
        // TRANSLATORS: number of chars in string should be near original
        mDialog = new TextDialog(_("Rename map sign          "),
        // TRANSLATORS: number of chars in string should be near original
            _("Name:                    "));
        mDialog->postInit();
        mRenameListener.setDialog(mDialog);
        mDialog->setText(mMapItem->getComment());
        mDialog->setActionEventId("ok");
        mDialog->addActionListener(&mRenameListener);
    }
    else if (link == "edit spell" && mSpell)
    {
        (new TextCommandEditor(mSpell))->postInit();
    }
    else if (link == "undress" && being)
    {
        beingHandler->undress(being);
    }
    else if (link == "addcomment" && !mNick.empty())
    {
        // TRANSLATORS: number of chars in string should be near original
        TextDialog *const dialog = new TextDialog(
            _("Player comment            "),
            // TRANSLATORS: number of chars in string should be near original
            _("Comment:                      "));
        dialog->postInit();
        mPlayerListener.setDialog(dialog);
        mPlayerListener.setNick(mNick);
        mPlayerListener.setType(mType);

        if (being)
        {
            being->updateComment();
            dialog->setText(being->getComment());
        }
        else
        {
            dialog->setText(Being::loadComment(mNick,
                static_cast<ActorType::Type>(mType)));
        }
        dialog->setActionEventId("ok");
        dialog->addActionListener(&mPlayerListener);
    }
    else if (link == "enable highlight" && mTab)
    {
        inputManager.executeChatCommand(InputAction::ENABLE_HIGHLIGHT,
            std::string(), mTab);
    }
    else if (link == "disable highlight" && mTab)
    {
        inputManager.executeChatCommand(InputAction::DISABLE_HIGHLIGHT,
            std::string(), mTab);
    }
    else if (link == "dont remove name" && mTab)
    {
        inputManager.executeChatCommand(InputAction::DONT_REMOVE_NAME,
            std::string(), mTab);
    }
    else if (link == "remove name" && mTab)
    {
        inputManager.executeChatCommand(InputAction::REMOVE_NAME,
            std::string(), mTab);
    }
    else if (link == "disable away" && mTab)
    {
        inputManager.executeChatCommand(InputAction::DISABLE_AWAY,
            std::string(), mTab);
    }
    else if (link == "enable away" && mTab)
    {
        inputManager.executeChatCommand(InputAction::ENABLE_AWAY,
            std::string(), mTab);
    }
    else if (link == "chat clipboard" && mTab)
    {
        if (chatWindow)
            chatWindow->copyToClipboard(mX, mY);
    }
    else if (link == "npc clipboard" && mBeingId)
    {
        NpcDialog::copyToClipboard(mBeingId, mX, mY);
    }
    else if (link == "remove pickup" && !mNick.empty())
    {
        if (actorManager)
        {
            actorManager->removePickupItem(mNick);
            if (socialWindow)
                socialWindow->updatePickupFilter();
        }
    }
    else if (link == "add pickup" && !mNick.empty())
    {
        if (actorManager)
        {
            actorManager->addPickupItem(mNick);
            if (socialWindow)
                socialWindow->updatePickupFilter();
        }
    }
    else if (link == "add pickup ignore" && !mNick.empty())
    {
        if (actorManager)
        {
            actorManager->addIgnorePickupItem(mNick);
            if (socialWindow)
                socialWindow->updatePickupFilter();
        }
    }
    else if (link == "attack moveup")
    {
        if (actorManager)
        {
            const int idx = actorManager->getAttackMobIndex(mNick);
            if (idx > 0)
            {
                std::list<std::string> mobs
                    = actorManager->getAttackMobs();
                std::list<std::string>::iterator it = mobs.begin();
                std::list<std::string>::iterator it2 = it;
                while (it != mobs.end())
                {
                    if (*it == mNick)
                    {
                        -- it2;
                        mobs.splice(it2, mobs, it);
                        actorManager->setAttackMobs(mobs);
                        actorManager->rebuildAttackMobs();
                        break;
                    }
                    ++ it;
                    ++ it2;
                }

                if (socialWindow)
                    socialWindow->updateAttackFilter();
            }
        }
    }
    else if (link == "priority moveup")
    {
        if (actorManager)
        {
            const int idx = actorManager->
                getPriorityAttackMobIndex(mNick);
            if (idx > 0)
            {
                std::list<std::string> mobs
                    = actorManager->getPriorityAttackMobs();
                std::list<std::string>::iterator it = mobs.begin();
                std::list<std::string>::iterator it2 = it;
                while (it != mobs.end())
                {
                    if (*it == mNick)
                    {
                        -- it2;
                        mobs.splice(it2, mobs, it);
                        actorManager->setPriorityAttackMobs(mobs);
                        actorManager->rebuildPriorityAttackMobs();
                        break;
                    }
                    ++ it;
                    ++ it2;
                }

                if (socialWindow)
                    socialWindow->updateAttackFilter();
            }
        }
    }
    else if (link == "attack movedown")
    {
        if (actorManager)
        {
            const int idx = actorManager->getAttackMobIndex(mNick);
            const int size = actorManager->getAttackMobsSize();
            if (idx + 1 < size)
            {
                std::list<std::string> mobs
                    = actorManager->getAttackMobs();
                std::list<std::string>::iterator it = mobs.begin();
                std::list<std::string>::iterator it2 = it;
                while (it != mobs.end())
                {
                    if (*it == mNick)
                    {
                        ++ it2;
                        if (it2 == mobs.end())
                            break;

                        mobs.splice(it, mobs, it2);
                        actorManager->setAttackMobs(mobs);
                        actorManager->rebuildAttackMobs();
                        break;
                    }
                    ++ it;
                    ++ it2;
                }

                if (socialWindow)
                    socialWindow->updateAttackFilter();
            }
        }
    }
    else if (link == "priority movedown")
    {
        if (localPlayer)
        {
            const int idx = actorManager
                ->getPriorityAttackMobIndex(mNick);
            const int size = actorManager->getPriorityAttackMobsSize();
            if (idx + 1 < size)
            {
                std::list<std::string> mobs
                    = actorManager->getPriorityAttackMobs();
                std::list<std::string>::iterator it = mobs.begin();
                std::list<std::string>::iterator it2 = it;
                while (it != mobs.end())
                {
                    if (*it == mNick)
                    {
                        ++ it2;
                        if (it2 == mobs.end())
                            break;

                        mobs.splice(it, mobs, it2);
                        actorManager->setPriorityAttackMobs(mobs);
                        actorManager->rebuildPriorityAttackMobs();
                        break;
                    }
                    ++ it;
                    ++ it2;
                }

                if (socialWindow)
                    socialWindow->updateAttackFilter();
            }
        }
    }
    else if (link == "attack remove")
    {
        if (actorManager)
        {
            if (mNick.empty())
            {
                if (actorManager->isInAttackList(mNick))
                {
                    actorManager->removeAttackMob(mNick);
                    actorManager->addIgnoreAttackMob(mNick);
                }
                else
                {
                    actorManager->removeAttackMob(mNick);
                    actorManager->addAttackMob(mNick);
                }
            }
            else
            {
                actorManager->removeAttackMob(mNick);
            }
            if (socialWindow)
                socialWindow->updateAttackFilter();
        }
    }
    else if (link == "pickup remove")
    {
        if (actorManager)
        {
            if (mNick.empty())
            {
                if (actorManager->isInPickupList(mNick))
                {
                    actorManager->removePickupItem(mNick);
                    actorManager->addIgnorePickupItem(mNick);
                }
                else
                {
                    actorManager->removePickupItem(mNick);
                    actorManager->addPickupItem(mNick);
                }
            }
            else
            {
                actorManager->removePickupItem(mNick);
            }
            if (socialWindow)
                socialWindow->updatePickupFilter();
        }
    }
    else if (link == "reset yellow")
    {
        GameModifiers::resetModifiers();
    }
    else if (link == "bar to chat" && !mNick.empty())
    {
        if (chatWindow)
            chatWindow->addInputText(mNick);
    }
    else if (link == "items" && being)
    {
        if (being == localPlayer)
        {
            if (equipmentWindow && !equipmentWindow->isWindowVisible())
                equipmentWindow->setVisible(true);
        }
        else
        {
            if (beingEquipmentWindow)
            {
                beingEquipmentWindow->setBeing(being);
                beingEquipmentWindow->setVisible(true);
            }
        }
    }
    else if (link == "undress item" && being && mItemId)
    {
        being->undressItemById(mItemId);
    }
    else if (link == "guild-pos" && !mNick.empty())
    {
        showChangePos(getX(), getY());
        return;
    }
    else if (link == "clear outfit")
    {
        if (outfitWindow)
            outfitWindow->clearCurrentOutfit();
    }
    else if (link == "clipboard copy")
    {
        if (mTextField)
            mTextField->handleCopy();
    }
    else if (link == "clipboard paste")
    {
        if (mTextField)
            mTextField->handlePaste();
    }
    else if (link == "open link" && !mNick.empty())
    {
        openBrowser(mNick);
    }
    else if (link == "clipboard link" && !mNick.empty())
    {
        sendBuffer(mNick);
    }
    else if (link == "goto" && !mNick.empty())
    {
        adminHandler->gotoName(mNick);
    }
    else if (link == "recall" && !mNick.empty())
    {
        adminHandler->recallName(mNick);
    }
    else if (link == "revive" && !mNick.empty())
    {
        adminHandler->reviveName(mNick);
    }
    else if (link == "ipcheck" && !mNick.empty())
    {
        adminHandler->ipcheckName(mNick);
    }
    else if (link == "gm" && !mNick.empty())
    {
        showGMPopup();
        return;
    }
    else if (link == "window close" && mWindow)
    {
        if (Widget::widgetExists(mWindow))
            mWindow->close();
    }
    else if (link == "window unlock" && mWindow)
    {
        if (Widget::widgetExists(mWindow))
            mWindow->setSticky(false);
    }
    else if (link == "window lock" && mWindow)
    {
        if (Widget::widgetExists(mWindow))
            mWindow->setSticky(true);
    }
#ifdef EATHENA_SUPPORT
    else if (link == "join chat" && being)
    {
        const ChatObject *const chat = being->getChat();
        if (chat)
            chatHandler->joinChat(chat, "");
    }
    else if (link == "fire mercenary")
    {
        mercenaryHandler->fire();
    }
    else if (link == "mercenary to master")
    {
        mercenaryHandler->moveToMaster();
    }
    else if (link == "homunculus to master")
    {
        homunculusHandler->moveToMaster();
    }
    else if (link == "homunculus feed")
    {
        homunculusHandler->feed();
    }
    else if (link == "homunculus delete")
    {
        homunculusHandler->fire();
    }
#endif
    else if (link == "pet feed")
    {
        petHandler->feed();
    }
    else if (link == "pet drop loot")
    {
        petHandler->dropLoot();
    }
    else if (link == "pet to egg")
    {
        petHandler->returnToEgg();
    }
    else if (link == "pet unequip")
    {
        petHandler->unequip();
    }
    else if (!link.compare(0, 10, "guild-pos-"))
    {
        if (localPlayer)
        {
            const int num = atoi(link.substr(10).c_str());
            const Guild *const guild = localPlayer->getGuild();
            if (guild)
            {
                guildHandler->changeMemberPostion(
                    guild->getMember(mNick), num);
            }
        }
    }
    else if (!link.compare(0, 7, "player_"))
    {
        if (actorManager)
        {
            mBeingId = atoi(link.substr(7).c_str());
            being = actorManager->findBeing(mBeingId);
            if (being)
            {
                showPopup(getX(), getY(), being);
                return;
            }
        }
    }
    else if (!link.compare(0, 10, "flooritem_"))
    {
        if (actorManager)
        {
            const int id = atoi(link.substr(10).c_str());
            if (id)
            {
                const FloorItem *const item = actorManager->findItem(id);
                if (item)
                {
                    mFloorItemId = item->getId();
                    showPopup(getX(), getY(), item);
                    return;
                }
            }
        }
    }
    else if (!link.compare(0, 12, "hide button_"))
    {
        if (windowMenu)
            windowMenu->showButton(link.substr(12), false);
    }
    else if (!link.compare(0, 12, "show button_"))
    {
        if (windowMenu)
            windowMenu->showButton(link.substr(12), true);
    }
    else if (!link.compare(0, 9, "hide bar_"))
    {
        if (miniStatusWindow)
            miniStatusWindow->showBar(link.substr(9), false);
    }
    else if (!link.compare(0, 9, "show bar_"))
    {
        if (miniStatusWindow)
            miniStatusWindow->showBar(link.substr(9), true);
    }
    else if (!link.compare(0, 12, "show window_"))
    {
        const int id = atoi(link.substr(12).c_str());
        if (id >= 0)
            inputManager.executeAction(id);
    }
    else if (!link.compare(0, 6, "mute_+"))
    {
        if (being)
        {
            const int time = atoi(link.substr(6).c_str());
            adminHandler->mute(being, 1, time);
        }
    }
    else if (!link.compare(0, 6, "mute_-"))
    {
        if (being)
        {
            const int time = atoi(link.substr(6).c_str());
            adminHandler->mute(being, 0, time);
        }
    }
    else if (!link.empty() && link[0] == '/')
    {
        std::string cmd = link.substr(1);
        replaceAll(cmd, "'NAME'", mNick);
        replaceAll(cmd, "'X'", toString(mX));
        replaceAll(cmd, "'Y'", toString(mY));
        replaceAll(cmd, "'BEINGID'", toString(mBeingId));
        replaceAll(cmd, "'FLOORID'", toString(mFloorItemId));
        replaceAll(cmd, "'ITEMID'", toString(mItemId));
        replaceAll(cmd, "'ITEMCOLOR'", toString(mItemColor));
        replaceAll(cmd, "'BEINGTYPEID'", toString(static_cast<int>(mType)));
        replaceAll(cmd, "'PLAYER'", localPlayer->getName());
        if (mItem)
            replaceAll(cmd, "'INVINDEX'", toString(mItem->getInvIndex()));
        else
            replaceAll(cmd, "'INVINDEX'", "0");
        if (mMapItem)
        {
            replaceAll(cmd, "'MAPX'", toString(mMapItem->getX()));
            replaceAll(cmd, "'MAPY'", toString(mMapItem->getY()));
        }
        else
        {
            replaceAll(cmd, "'MAPX'", toString(mX));
            replaceAll(cmd, "'MAPY'", toString(mY));
        }

        const size_t pos = cmd.find(' ');
        const std::string type(cmd, 0, pos);
        std::string args(cmd, pos == std::string::npos ? cmd.size() : pos + 1);
        args = trim(args);
        inputManager.executeChatCommand(type, args, mTab);
    }
    // Unknown actions
    else if (link != "cancel")
    {
        logger->log("PopupMenu: Warning, unknown action '%s'", link.c_str());
    }

    setVisible(false);

    mBeingId = 0;
    mFloorItemId = 0;
    mItem = nullptr;
    mItemId = 0;
    mItemColor = 1;
    mMapItem = nullptr;
    mTab = nullptr;
    mSpell = nullptr;
    mWindow = nullptr;
    mDialog = nullptr;
    mButton = nullptr;
    mNick.clear();
    mTextField = nullptr;
    mType = ActorType::Unknown;
    mX = 0;
    mY = 0;
}

void PopupMenu::showPopup(Window *const parent,
                          const int x, const int y,
                          Item *const item,
                          const InventoryType::Type type)
{
    if (!item)
        return;

    mItem = item;
    mItemId = item->getId();
    mItemColor = item->getColor();
    mWindow = parent;
    mX = x;
    mY = y;
    mNick.clear();
    mBrowserBox->clearRows();

    const int cnt = item->getQuantity();
    const bool isProtected = PlayerInfo::isItemProtected(mItemId);

    switch (type)
    {
        case InventoryType::INVENTORY:
            if (tradeWindow && tradeWindow->isWindowVisible() && !isProtected)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: add item to trade
                mBrowserBox->addRow("/addtrade 'INVINDEX'", _("Add to trade"));
                if (cnt > 1)
                {
                    if (cnt > 10)
                    {
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: add 10 item amount to trade
                        mBrowserBox->addRow("/addtrade 'INVINDEX' 10",
                            _("Add to trade 10"));
                    }
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: add half item amount to trade
                    mBrowserBox->addRow("/addtrade 'INVINDEX' /",
                        _("Add to trade half"));
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: add all amount except one item to trade
                    mBrowserBox->addRow("/addtrade 'INVINDEX' -1",
                        _("Add to trade all-1"));
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: add all amount item to trade
                    mBrowserBox->addRow("/addtrade 'INVINDEX' all",
                        _("Add to trade all"));
                }
                mBrowserBox->addRow("##3---");
            }
            if (InventoryWindow::isStorageActive())
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: add item to storage
                mBrowserBox->addRow("/invtostorage 'INVINDEX'", _("Store"));
                if (cnt > 1)
                {
                    if (cnt > 10)
                    {
                        // TRANSLATORS: popup menu item
                        // TRANSLATORS: add 10 item amount to storage
                        mBrowserBox->addRow("/invtostorage 'INVINDEX' 10",
                            _("Store 10"));
                    }
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: add half item amount to storage
                    mBrowserBox->addRow("/invtostorage 'INVINDEX' /",
                        _("Store half"));
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: add all except one item amount to storage
                    mBrowserBox->addRow("/invtostorage 'INVINDEX' -1",
                        _("Store all-1"));
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: add all item amount to storage
                    mBrowserBox->addRow("/invtostorage 'INVINDEX' all",
                        _("Store all"));
                }
                mBrowserBox->addRow("##3---");
            }
            addUseDrop(item, isProtected);
            break;

        case InventoryType::STORAGE:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: get item from storage
            mBrowserBox->addRow("/storagetoinv 'INVINDEX'", _("Retrieve"));
            if (cnt > 1)
            {
                if (cnt > 10)
                {
                    // TRANSLATORS: popup menu item
                    // TRANSLATORS: get 10 item amount from storage
                    mBrowserBox->addRow("/storagetoinv 'INVINDEX' 10",
                        _("Retrieve 10"));
                }
                // TRANSLATORS: popup menu item
                // TRANSLATORS: get half item amount from storage
                mBrowserBox->addRow("/storagetoinv 'INVINDEX' /",
                    _("Retrieve half"));
                // TRANSLATORS: popup menu item
                // TRANSLATORS: get all except one item amount from storage
                mBrowserBox->addRow("/storagetoinv 'INVINDEX' -1",
                    _("Retrieve all-1"));
                // TRANSLATORS: popup menu item
                // TRANSLATORS: get all item amount from storage
                mBrowserBox->addRow("/storagetoinv 'INVINDEX' all",
                    _("Retrieve all"));
            }
            break;

        case InventoryType::TRADE:
        case InventoryType::NPC:
#ifdef EATHENA_SUPPORT
        case InventoryType::CART:
        case InventoryType::VENDING:
        case InventoryType::MAIL:
#endif
        case InventoryType::TYPE_END:
        default:
            break;
    }


    addProtection();
    if (config.getBoolValue("enablePickupFilter"))
    {
        mNick = item->getName();
        mBrowserBox->addRow("##3---");
        addPickupFilter(mNick);
    }
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add item name to chat
    mBrowserBox->addRow("/addchat 'ITEMID'", _("Add to chat"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showItemPopup(const int x, const int y, const int itemId,
                              const unsigned char color)
{
    const Inventory *const inv = PlayerInfo::getInventory();
    if (!inv)
        return;

    Item *const item = inv->findItem(itemId, color);
    if (item)
    {
        showItemPopup(x, y, item);
    }
    else
    {
        mItem = nullptr;
        mItemId = itemId;
        mItemColor = color;
        mX = x;
        mY = y;
        mBrowserBox->clearRows();

        if (!PlayerInfo::isItemProtected(mItemId))
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: use item
            mBrowserBox->addRow("/use 'ITEMID'", _("Use"));
        }
        addProtection();
        mBrowserBox->addRow("##3---");
        // TRANSLATORS: popup menu item
        // TRANSLATORS: close menu
        mBrowserBox->addRow("cancel", _("Cancel"));

        showPopup(x, y);
    }
}

void PopupMenu::showItemPopup(const int x, const int y, Item *const item)
{
    mItem = item;
    mX = x;
    mY = y;
    if (item)
    {
        mItemId = item->getId();
        mItemColor = item->getColor();
    }
    else
    {
        mItemId = 0;
        mItemColor = 1;
    }
    mNick.clear();
    mBrowserBox->clearRows();

    if (item)
    {
        const bool isProtected = PlayerInfo::isItemProtected(mItemId);
        addUseDrop(item, isProtected);
        if (InventoryWindow::isStorageActive())
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add item to storage
            mBrowserBox->addRow("/invtostorage 'INVINDEX'", _("Store"));
        }
        // TRANSLATORS: popup menu item
        // TRANSLATORS: add item name to chat
        mBrowserBox->addRow("/addchat 'ITEMID'", _("Add to chat"));

        if (config.getBoolValue("enablePickupFilter"))
        {
            mNick = item->getName();

            mBrowserBox->addRow("##3---");
            addPickupFilter(mNick);
        }
    }
    addProtection();
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showDropPopup(const int x, const int y, Item *const item)
{
    mItem = item;
    mX = x;
    mY = y;
    mNick.clear();
    mBrowserBox->clearRows();

    if (item)
    {
        mItemId = item->getId();
        mItemColor = item->getColor();
        const bool isProtected = PlayerInfo::isItemProtected(mItemId);
        addUseDrop(item, isProtected);
        if (InventoryWindow::isStorageActive())
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add item to storage
            mBrowserBox->addRow("/invtostorage 'INVINDEX'", _("Store"));
        }
        addProtection();
        // TRANSLATORS: popup menu item
        // TRANSLATORS: add item name to chat
        mBrowserBox->addRow("/addchat 'ITEMID'", _("Add to chat"));
        if (config.getBoolValue("enablePickupFilter"))
        {
            mNick = item->getName();
            mBrowserBox->addRow("##3---");
            addPickupFilter(mNick);
        }
    }
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    mBrowserBox->addRow("/cleardrops", _("Clear drop window"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showPopup(const int x, const int y, Button *const button)
{
    if (!button || !windowMenu)
        return;

    mButton = button;
    mX = x;
    mY = y;

    mBrowserBox->clearRows();
    std::vector<Button *> names = windowMenu->getButtons();
    for (std::vector<Button *>::const_iterator it = names.begin(),
         it_end = names.end(); it != it_end; ++ it)
    {
        const Button *const btn = *it;
        if (!btn || btn->getActionEventId() == "SET")
            continue;

        if (btn->isVisibleLocal())
        {
            mBrowserBox->addRow(strprintf("@@hide button_%s|%s %s (%s)@@",
                // TRANSLATORS: popup menu item
                btn->getActionEventId().c_str(), _("Hide"),
                btn->getDescription().c_str(), btn->getCaption().c_str()));
        }
        else
        {
            mBrowserBox->addRow(strprintf("@@show button_%s|%s %s (%s)@@",
                // TRANSLATORS: popup menu item
                btn->getActionEventId().c_str(), _("Show"),
                btn->getDescription().c_str(), btn->getCaption().c_str()));
        }
    }
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showPopup(const int x, const int y, const ProgressBar *const b)
{
    if (!b || !miniStatusWindow)
        return;

    mNick = b->text();
    mX = x;
    mY = y;

    mBrowserBox->clearRows();
    std::vector <ProgressBar*> bars = miniStatusWindow->getBars();
    ProgressBar *onlyBar = nullptr;
    int cnt = 0;

    // search for alone visible bar
    for (std::vector <ProgressBar*>::const_iterator it = bars.begin(),
         it_end = bars.end(); it != it_end; ++it)
    {
        ProgressBar *const bar = *it;
        if (!bar)
            continue;

        if (bar->isVisibleLocal())
        {
            cnt ++;
            onlyBar = bar;
        }
    }
    if (cnt > 1)
        onlyBar = nullptr;

    for (std::vector <ProgressBar*>::const_iterator it = bars.begin(),
         it_end = bars.end(); it != it_end; ++it)
    {
        ProgressBar *const bar = *it;
        if (!bar || bar == onlyBar)
            continue;

        if (bar->isVisibleLocal())
        {
            mBrowserBox->addRow(strprintf("@@hide bar_%s|%s %s@@",
                // TRANSLATORS: popup menu item
                bar->getActionEventId().c_str(), _("Hide"),
                bar->getId().c_str()));
        }
        else
        {
            mBrowserBox->addRow(strprintf("@@show bar_%s|%s %s@@",
                // TRANSLATORS: popup menu item
                bar->getActionEventId().c_str(), _("Show"),
                bar->getId().c_str()));
        }
    }

    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    mBrowserBox->addRow("/yellowbar", _("Open yellow bar settings"));
    // TRANSLATORS: popup menu item
    mBrowserBox->addRow("reset yellow", _("Reset yellow bar"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: copy status to chat
    mBrowserBox->addRow("bar to chat", _("Copy to chat"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showAttackMonsterPopup(const int x, const int y,
                                       const std::string &name, const int type)
{
    if (!localPlayer || !actorManager)
        return;

    mNick = name;
    mType = ActorType::Monster;
    mX = x;
    mY = y;

    mBrowserBox->clearRows();

    if (name.empty())
    {
        // TRANSLATORS: popup menu header
        mBrowserBox->addRow(_("(default)"));
    }
    else
    {
        mBrowserBox->addRow(name);
    }
    switch (type)
    {
        case MapItemType::ATTACK:
        {
            const int idx = actorManager->getAttackMobIndex(name);
            const int size = actorManager->getAttackMobsSize();
            if (idx > 0)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: move attack target up
                mBrowserBox->addRow("attack moveup", _("Move up"));
            }
            if (idx + 1 < size)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: move attack target down
                mBrowserBox->addRow("attack movedown", _("Move down"));
            }
            break;
        }
        case MapItemType::PRIORITY:
        {
            const int idx = actorManager->
                getPriorityAttackMobIndex(name);
            const int size = actorManager->getPriorityAttackMobsSize();
            if (idx > 0)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: move attack target up
                mBrowserBox->addRow("priority moveup", _("Move up"));
            }
            if (idx + 1 < size)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: move attack target down
                mBrowserBox->addRow("priority movedown", _("Move down"));
            }
            break;
        }
        case MapItemType::IGNORE_:
            break;
        default:
            break;
    }

    // TRANSLATORS: popup menu item
    // TRANSLATORS: remove attack target
    mBrowserBox->addRow("attack remove", _("Remove"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showPickupItemPopup(const int x, const int y,
                                    const std::string &name)
{
    if (!localPlayer || !actorManager)
        return;

    mNick = name;
    mType = ActorType::FloorItem;
    mX = x;
    mY = y;

    mBrowserBox->clearRows();

    if (name.empty())
    {
        // TRANSLATORS: popup menu header
        mBrowserBox->addRow(_("(default)"));
    }
    else
    {
        mBrowserBox->addRow(name);
    }

    // TRANSLATORS: popup menu item
    // TRANSLATORS: remove item from pickup filter
    mBrowserBox->addRow("pickup remove", _("Remove"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showUndressPopup(const int x, const int y,
                                 const Being *const being, Item *const item)
{
    if (!being || !item)
        return;

    mBeingId = being->getId();
    mItem = item;
    mItemId = item->getId();
    mItemColor = item->getColor();
    mX = x;
    mY = y;

    mBrowserBox->clearRows();

    // TRANSLATORS: popup menu item
    // TRANSLATORS: undress item from player
    mBrowserBox->addRow("undress item", _("Undress"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showTextFieldPopup(TextField *const input)
{
    setMousePos();
    mTextField = input;

    mBrowserBox->clearRows();

    // TRANSLATORS: popup menu item
    // TRANSLATORS: copy text to clipboard
    mBrowserBox->addRow("clipboard copy", _("Copy"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: paste text from clipboard
    mBrowserBox->addRow("clipboard paste", _("Paste"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(mX, mY);
}

void PopupMenu::showLinkPopup(const std::string &link)
{
    setMousePos();
    mNick = link;

    mBrowserBox->clearRows();

    // TRANSLATORS: popup menu item
    // TRANSLATORS: open link in browser
    mBrowserBox->addRow("open link", _("Open link"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: copy link to clipboard
    mBrowserBox->addRow("clipboard link", _("Copy to clipboard"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(mX, mY);
}

void PopupMenu::showWindowsPopup()
{
    setMousePos();
    mBrowserBox->clearRows();
    const std::vector<ButtonText*> &names = windowMenu->getButtonTexts();
    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("Show window"));

    FOR_EACH (std::vector<ButtonText*>::const_iterator, it, names)
    {
        const ButtonText *const btn = *it;
        if (!btn)
            continue;

        mBrowserBox->addRow(strprintf("show window_%d", btn->key),
            btn->text.c_str());
    }
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(mX, mY);
}

void PopupMenu::showNpcDialogPopup(const int npcId, const int x, const int y)
{
    mBeingId = npcId;
    mX = x;
    mY = y;
    mBrowserBox->clearRows();
    // TRANSLATORS: popup menu item
    // TRANSLATORS: copy npc text to clipboard
    mBrowserBox->addRow("npc clipboard", _("Copy to clipboard"));
    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(x, y);
}

void PopupMenu::showPopup(int x, int y)
{
    const int pad2 = 2 * mPadding;
    const int bPad2 = 2 * mBrowserBox->getPadding();
    mBrowserBox->setPosition(mPadding, mPadding);
    mScrollArea->setPosition(mPadding, mPadding);
    // add padding to initial size before draw browserbox
    int height = mBrowserBox->getHeight();
    if (height + pad2 >= mainGraphics->getHeight())
    {
        height = mainGraphics->getHeight() - bPad2 - pad2;
        mBrowserBox->setWidth(mBrowserBox->getWidth() + bPad2 + 5);
        mScrollArea->setWidth(mBrowserBox->getWidth() + pad2 + 10);
        setContentSize(mBrowserBox->getWidth() + pad2 + 20,
            height + pad2);
    }
    else
    {
        mBrowserBox->setWidth(mBrowserBox->getWidth() + bPad2);
        mScrollArea->setWidth(mBrowserBox->getWidth() + pad2);
        setContentSize(mBrowserBox->getWidth() + pad2,
            height + pad2);
    }
    if (mainGraphics->mWidth < (x + getWidth() + 5))
        x = mainGraphics->mWidth - getWidth();
    if (mainGraphics->mHeight < (y + getHeight() + 5))
        y = mainGraphics->mHeight - getHeight();
    mScrollArea->setHeight(height);
    setPosition(x, y);
    setVisible(true);
    requestMoveToTop();
}

void PopupMenu::addNormalRelations()
{
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add player to disregarded list
    mBrowserBox->addRow("/disregard 'NAME'", _("Disregard"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add player to ignore list
    mBrowserBox->addRow("/ignore 'NAME'", _("Ignore"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add player to black list
    mBrowserBox->addRow("/blacklist 'NAME'", _("Black list"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add player to enemy list
    mBrowserBox->addRow("/enemy 'NAME'", _("Set as enemy"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add player to erased list
    mBrowserBox->addRow("/erase 'NAME'", _("Erase"));
}

void PopupMenu::addPlayerRelation(const std::string &name)
{
    switch (player_relations.getRelation(name))
    {
        case PlayerRelation::NEUTRAL:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to friends list
            mBrowserBox->addRow("/friend 'NAME'", _("Be friend"));
            addNormalRelations();
            break;

        case PlayerRelation::FRIEND:
            addNormalRelations();
            break;

        case PlayerRelation::BLACKLISTED:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: remove player from ignore list
            mBrowserBox->addRow("/unignore 'NAME'", _("Unignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to disregarded list
            mBrowserBox->addRow("/disregard 'NAME'", _("Disregard"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to ignore list
            mBrowserBox->addRow("/ignore 'NAME'", _("Ignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to enemy list
            mBrowserBox->addRow("/enemy 'NAME'", _("Set as enemy"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to erased list
            mBrowserBox->addRow("/erase 'NAME'", _("Erase"));
            break;

        case PlayerRelation::DISREGARDED:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: remove player from ignore list
            mBrowserBox->addRow("/unignore 'NAME'", _("Unignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to completle ignore list
            mBrowserBox->addRow("/ignore 'NAME'", _("Completely ignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to erased list
            mBrowserBox->addRow("/erase 'NAME'", _("Erase"));
            break;

        case PlayerRelation::IGNORED:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: remove player from ignore list
            mBrowserBox->addRow("/unignore 'NAME'", _("Unignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to erased list
            mBrowserBox->addRow("/erase 'NAME'", _("Erase"));
            break;

        case PlayerRelation::ENEMY2:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: remove player from ignore list
            mBrowserBox->addRow("/unignore 'NAME'", _("Unignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to disregarded list
            mBrowserBox->addRow("/disregard 'NAME'", _("Disregard"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to ignore list
            mBrowserBox->addRow("/ignore 'NAME'", _("Ignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to black list
            mBrowserBox->addRow("/blacklist 'NAME'", _("Black list"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to erased list
            mBrowserBox->addRow("/erase 'NAME'", _("Erase"));
            break;

        case PlayerRelation::ERASED:
            // TRANSLATORS: popup menu item
            // TRANSLATORS: remove player from ignore list
            mBrowserBox->addRow("/unignore 'NAME'", _("Unignore"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to disregarded list
            mBrowserBox->addRow("/disregard 'NAME'", _("Disregard"));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add player to ignore list
            mBrowserBox->addRow("/ignore 'NAME'", _("Completely ignore"));
            break;

        default:
            break;
    }
}

void PopupMenu::addFollow()
{
    if (features.getBoolValue("allowFollow"))
    {
        // TRANSLATORS: popup menu item
        mBrowserBox->addRow("/follow 'NAME'", _("Follow"));
    }
    // TRANSLATORS: popup menu item
    // TRANSLATORS: imitate player
    mBrowserBox->addRow("/imitation 'NAME'", _("Imitate"));
}

void PopupMenu::addBuySell(const Being *const being)
{
    if (player_relations.getDefault() & PlayerRelation::TRADE)
    {
        mBrowserBox->addRow("##3---");
        const bool haveVending = serverFeatures->haveVending();
        if (being->isSellShopEnabled())
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: buy item
            mBrowserBox->addRow("/buy 'NAME'", _("Buy"));
        }
        else if (!haveVending)
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: buy item
            mBrowserBox->addRow("/buy 'NAME'", _("Buy (?)"));
        }
        if (being->isBuyShopEnabled())
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: sell item
            mBrowserBox->addRow("/sell 'NAME'", _("Sell"));
        }
        else if (!haveVending)
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: sell item
            mBrowserBox->addRow("/sell 'NAME'", _("Sell (?)"));
        }
    }
}

void PopupMenu::addBuySellDefault()
{
    if (player_relations.getDefault() & PlayerRelation::TRADE)
    {
        mBrowserBox->addRow("##3---");
        // TRANSLATORS: popup menu item
        // TRANSLATORS: buy item
        mBrowserBox->addRow("/buy 'NAME'", _("Buy (?)"));
        // TRANSLATORS: popup menu item
        // TRANSLATORS: sell item
        mBrowserBox->addRow("/sell 'NAME'", _("Sell (?)"));
    }
}

void PopupMenu::addPartyName(const std::string &partyName)
{
    if (localPlayer->isInParty())
    {
        if (localPlayer->getParty())
        {
            if (localPlayer->getParty()->getName() != partyName)
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: invite player to party
                mBrowserBox->addRow("/party 'NAME'", _("Invite to party"));
            }
            else
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: kick player from party
                mBrowserBox->addRow("/kickparty 'NAME'", _("Kick from party"));
            }
            mBrowserBox->addRow("##3---");
        }
    }
}

void PopupMenu::addParty(const std::string &nick)
{
    if (localPlayer->isInParty())
    {
        const Party *const party = localPlayer->getParty();
        if (party)
        {
            if (!party->isMember(nick))
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: invite player to party
                mBrowserBox->addRow("/party 'NAME'", _("Invite to party"));
            }
            else
            {
                // TRANSLATORS: popup menu item
                // TRANSLATORS: kick player from party
                mBrowserBox->addRow("/kickparty 'NAME'", _("Kick from party"));
            }
            mBrowserBox->addRow("##3---");
        }
    }
}

#ifdef EATHENA_SUPPORT
void PopupMenu::addChat(const Being *const being)
{
    if (!being)
        return;
    const ChatObject *const chat = being->getChat();
    if (chat)
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: invite player to party
        mBrowserBox->addRow("join chat",
            strprintf(_("Join chat %s"), chat->title.c_str()).c_str());
        mBrowserBox->addRow("##3---");
    }
}
#endif

void PopupMenu::addPlayerMisc()
{
    // TRANSLATORS: popup menu item
    mBrowserBox->addRow("items", _("Show Items"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: undress player
    mBrowserBox->addRow("undress", _("Undress"));
    // TRANSLATORS: popup menu item
    // TRANSLATORS: add comment to player
    mBrowserBox->addRow("addcomment", _("Add comment"));
}

void PopupMenu::addPickupFilter(const std::string &name)
{
    if (actorManager->isInPickupList(name)
        || actorManager->isInIgnorePickupList(name))
    {
        mBrowserBox->addRow("remove pickup",
            // TRANSLATORS: popup menu item
            // TRANSLATORS: remove item from pickup list
            _("Remove from pickup list"));
    }
    else
    {
        // TRANSLATORS: popup menu item
        mBrowserBox->addRow("add pickup", _("Add to pickup list"));
        mBrowserBox->addRow("add pickup ignore",
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add item to pickup list
            _("Add to ignore list"));
    }
}

void PopupMenu::showPopup(const int x, const int y,
                          ListModel *const model)
{
    if (!model)
        return;

    mBrowserBox->clearRows();
    for (int f = 0, sz = model->getNumberOfElements(); f < sz; f ++)
    {
        mBrowserBox->addRow(strprintf("dropdown_%d", f),
            model->getElementAt(f).c_str());
    }
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));
    showPopup(x, y);
}

void PopupMenu::clear()
{
    if (mDialog)
    {
        mDialog->close();
        mDialog = nullptr;
    }
    mItem = nullptr;
    mMapItem = nullptr;
    mTab = nullptr;
    mSpell = nullptr;
    mWindow = nullptr;
    mButton = nullptr;
    mTextField = nullptr;
}

void PopupMenu::addProtection()
{
    if (PlayerInfo::isItemProtected(mItemId))
    {
        mBrowserBox->addRow("##3---");
        // TRANSLATORS: popup menu item
        // TRANSLATORS: remove protection from item
        mBrowserBox->addRow("/unprotectitem 'ITEMID'", _("Unprotect item"));
    }
    else
    {
        if (mItemId < SPELL_MIN_ID)
        {
            mBrowserBox->addRow("##3---");
            // TRANSLATORS: popup menu item
            // TRANSLATORS: add protection to item
            mBrowserBox->addRow("/protectitem 'ITEMID'", _("Protect item"));
        }
    }
}

void PopupMenu::addUseDrop(const Item *const item, const bool isProtected)
{
    const ItemInfo &info = item->getInfo();
    const std::string &str = (item->isEquipment() == Equipm_true
        && item->isEquipped() == Equipped_true)
        ? info.getUseButton2() : info.getUseButton();

    if (str.empty())
    {
        // TRANSLATORS: popup menu item
        mBrowserBox->addRow("/useinv 'INVINDEX'", _("Use"));
    }
    else
    {
        // TRANSLATORS: popup menu item
        mBrowserBox->addRow("/useinv 'INVINDEX'", str.c_str());
    }

    if (!isProtected)
    {
        mBrowserBox->addRow("##3---");
        if (item->getQuantity() > 1)
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: drop item
            mBrowserBox->addRow("/dropinv 'INVINDEX'", _("Drop..."));
            // TRANSLATORS: popup menu item
            // TRANSLATORS: drop all item amount
            mBrowserBox->addRow("/dropinvall 'INVINDEX'", _("Drop all"));
        }
        else
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: drop item
            mBrowserBox->addRow("/dropinv 'INVINDEX'", _("Drop"));
        }
    }
}

void PopupMenu::addGmCommands()
{
    if (localPlayer->isGM())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: gm commands
        mBrowserBox->addRow("gm", _("GM..."));
    }
}

void PopupMenu::showGMPopup()
{
    mBrowserBox->clearRows();
    // TRANSLATORS: popup menu header
    mBrowserBox->addRow(_("GM commands"));
    if (localPlayer->isGM())
    {
        // TRANSLATORS: popup menu item
        // TRANSLATORS: check player ip
        mBrowserBox->addRow("ipcheck", _("Check ip"));
        // TRANSLATORS: popup menu item
        // TRANSLATORS: go to player position
        mBrowserBox->addRow("goto", _("Goto"));
        // TRANSLATORS: popup menu item
        // TRANSLATORS: recall player to current position
        mBrowserBox->addRow("recall", _("Recall"));
        // TRANSLATORS: popup menu item
        // TRANSLATORS: revive player
        mBrowserBox->addRow("revive", _("Revive"));
        if (mBeingId)
        {
            // TRANSLATORS: popup menu item
            // TRANSLATORS: kick player
            mBrowserBox->addRow("/kick :'BEINGID'", _("Kick"));
        }
        if (serverFeatures->haveMute())
        {
            mBrowserBox->addRow("##3---");
            mBrowserBox->addRow("mute_+1",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Mute %d"), 1).c_str());
            mBrowserBox->addRow("mute_+5",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Mute %d"), 5).c_str());
            mBrowserBox->addRow("mute_+10",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Mute %d"), 10).c_str());
            mBrowserBox->addRow("mute_+15",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Mute %d"), 15).c_str());
            mBrowserBox->addRow("mute_+30",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Mute %d"), 30).c_str());

            mBrowserBox->addRow("mute_-1",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Unmute %d"), 1).c_str());
            mBrowserBox->addRow("mute_-5",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Unmute %d"), 5).c_str());
            mBrowserBox->addRow("mute_-10",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Unmute %d"), 10).c_str());
            mBrowserBox->addRow("mute_-15",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Unmute %d"), 15).c_str());
            mBrowserBox->addRow("mute_-30",
                // TRANSLATORS: popup menu item
                // TRANSLATORS: mute player
                strprintf(_("Unmute %d"), 30).c_str());
        }
    }

    mBrowserBox->addRow("##3---");
    // TRANSLATORS: popup menu item
    // TRANSLATORS: close menu
    mBrowserBox->addRow("cancel", _("Cancel"));

    showPopup(getX(), getY());
}

void PopupMenu::moveUp()
{
    mBrowserBox->moveSelectionUp();
}

void PopupMenu::moveDown()
{
    mBrowserBox->moveSelectionDown();
}

void PopupMenu::select()
{
    mBrowserBox->selectSelection();
}
