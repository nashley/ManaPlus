/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2014  The ManaPlus Developers
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

#include "actions/chat.h"

#include "guildmanager.h"

#include "actions/actiondef.h"

#include "being/localplayer.h"

#include "gui/chatconsts.h"

#include "gui/windows/chatwindow.h"

#include "gui/widgets/tabs/chat/chattab.h"
#include "gui/widgets/tabs/chat/chattabtype.h"

#include "net/chathandler.h"
#include "net/guildhandler.h"
#include "net/partyhandler.h"
#include "net/serverfeatures.h"

#include "utils/booleanoptions.h"
#include "utils/chatutils.h"
#include "utils/stringutils.h"

#include "debug.h"

namespace Actions
{

static void outString(const ChatTab *const tab,
                      const std::string &str,
                      const std::string &def)
{
    if (!tab)
    {
        chatHandler->me(def, GENERAL_CHANNEL);
        return;
    }

    switch (tab->getType())
    {
        case ChatTabType::PARTY:
        {
            partyHandler->chat(str);
            break;
        }
        case ChatTabType::GUILD:
        {
            if (!localPlayer)
                return;
            const Guild *const guild = localPlayer->getGuild();
            if (guild)
            {
                if (guild->getServerGuild())
                {
                    if (!serverFeatures->haveNativeGuilds())
                        return;
                    guildHandler->chat(guild->getId(), str);
                }
                else if (guildManager)
                {
                    guildManager->chat(str);
                }
            }
            break;
        }
        default:
        case ChatTabType::UNKNOWN:
        case ChatTabType::INPUT:
        case ChatTabType::WHISPER:
        case ChatTabType::DEBUG:
        case ChatTabType::TRADE:
        case ChatTabType::BATTLE:
        case ChatTabType::LANG:
        case ChatTabType::GM:
        case ChatTabType::CHANNEL:
            chatHandler->me(def, GENERAL_CHANNEL);
            break;
    }
}

impHandler0(toggleChat)
{
    return chatWindow ? chatWindow->requestChatFocus() : false;
}

impHandler0(prevChatTab)
{
    if (chatWindow)
    {
        chatWindow->prevTab();
        return true;
    }
    return false;
}

impHandler0(nextChatTab)
{
    if (chatWindow)
    {
        chatWindow->nextTab();
        return true;
    }
    return false;
}

impHandler0(closeChatTab)
{
    if (chatWindow)
    {
        chatWindow->closeTab();
        return true;
    }
    return false;
}

impHandler0(closeAllChatTabs)
{
    if (chatWindow)
    {
        chatWindow->removeAllWhispers();
        chatWindow->saveState();
        return true;
    }
    return false;
}

impHandler0(ignoreAllWhispers)
{
    if (chatWindow)
    {
        chatWindow->ignoreAllWhispers();
        chatWindow->saveState();
        return true;
    }
    return false;
}

impHandler0(scrollChatUp)
{
    if (chatWindow && chatWindow->isWindowVisible())
    {
        chatWindow->scroll(-DEFAULT_CHAT_WINDOW_SCROLL);
        return true;
    }
    return false;
}

impHandler0(scrollChatDown)
{
    if (chatWindow && chatWindow->isWindowVisible())
    {
        chatWindow->scroll(DEFAULT_CHAT_WINDOW_SCROLL);
        return true;
    }
    return false;
}

impHandler(msg)
{
    std::string recvnick;
    std::string message;

    if (event.args.substr(0, 1) == "\"")
    {
        const size_t pos = event.args.find('"', 1);
        if (pos != std::string::npos)
        {
            recvnick = event.args.substr(1, pos - 1);
            if (pos + 2 < event.args.length())
                message = event.args.substr(pos + 2, event.args.length());
        }
    }
    else
    {
        const size_t pos = event.args.find(" ");
        if (pos != std::string::npos)
        {
            recvnick = event.args.substr(0, pos);
            if (pos + 1 < event.args.length())
                message = event.args.substr(pos + 1, event.args.length());
        }
        else
        {
            recvnick = std::string(event.args);
            message.clear();
        }
    }

    trim(message);

    if (message.length() > 0)
    {
        std::string playerName = localPlayer->getName();
        std::string tempNick = recvnick;

        toLower(playerName);
        toLower(tempNick);

        if (tempNick.compare(playerName) == 0 || event.args.empty())
            return true;

        chatWindow->addWhisper(recvnick, message, ChatMsgType::BY_PLAYER);
    }
    else
    {
        // TRANSLATORS: whisper send
        event.tab->chatLog(_("Cannot send empty whispers!"),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

impHandler(query)
{
    const std::string &args = event.args;
    if (chatWindow)
    {
        if (chatWindow->addChatTab(args, true, true))
        {
            chatWindow->saveState();
            return true;
        }
    }

    if (event.tab)
    {
        // TRANSLATORS: new whisper or channel query
        event.tab->chatLog(strprintf(_("Cannot create a whisper tab "
            "\"%s\"! It probably already exists."),
            args.c_str()), ChatMsgType::BY_SERVER);
    }
    return true;
}

impHandler0(clearChatTab)
{
    if (chatWindow)
    {
        chatWindow->clearTab();
        return true;
    }
    return false;
}

impHandler(createParty)
{
    if (!event.tab)
        return false;

    if (event.args.empty())
    {
        // TRANSLATORS: create party message
        event.tab->chatLog(_("Party name is missing."),
            ChatMsgType::BY_SERVER);
    }
    else
    {
        partyHandler->create(event.args);
    }
    return true;
}

impHandler(createGuild)
{
    if (!event.tab || !serverFeatures->haveNativeGuilds())
        return false;

    if (event.args.empty())
    {
        // TRANSLATORS: create guild message
        event.tab->chatLog(_("Guild name is missing."),
            ChatMsgType::BY_SERVER);
    }
    else
    {
        guildHandler->create(event.args);
    }
    return true;
}

impHandler(party)
{
    if (!event.tab)
        return false;

    if (!event.args.empty())
    {
        partyHandler->invite(event.args);
    }
    else
    {
        // TRANSLATORS: party invite message
        event.tab->chatLog(_("Please specify a name."),
            ChatMsgType::BY_SERVER);
    }
    return true;
}

impHandler(me)
{
    outString(event.tab, textToMe(event.args), event.args);
    return true;
}

impHandler(toggle)
{
    if (event.args.empty())
    {
        if (chatWindow && event.tab)
        {
            // TRANSLATORS: message from toggle chat command
            event.tab->chatLog(chatWindow->getReturnTogglesChat() ?
                _("Return toggles chat.") : _("Message closes chat."));
        }
        return true;
    }

    switch (parseBoolean(event.args))
    {
        case 1:
            if (event.tab)
            {
                // TRANSLATORS: message from toggle chat command
                event.tab->chatLog(_("Return now toggles chat."));
            }
            if (chatWindow)
                chatWindow->setReturnTogglesChat(true);
            return true;
        case 0:
            if (event.tab)
            {
                // TRANSLATORS: message from toggle chat command
                event.tab->chatLog(_("Message now closes chat."));
            }
            if (chatWindow)
                chatWindow->setReturnTogglesChat(false);
            return true;
        case -1:
            if (event.tab)
                event.tab->chatLog(strprintf(BOOLEAN_OPTIONS, "toggle"));
            return true;
        default:
            return true;
    }
}

}  // namespace Actions
