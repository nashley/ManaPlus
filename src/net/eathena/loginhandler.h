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

#ifndef NET_EATHENA_LOGINHANDLER_H
#define NET_EATHENA_LOGINHANDLER_H

#include "net/ea/loginhandler.h"

#include "net/eathena/messagehandler.h"

namespace EAthena
{

class LoginHandler final : public MessageHandler, public Ea::LoginHandler
{
    public:
        LoginHandler();

        A_DELETE_COPY(LoginHandler)

        ~LoginHandler();

        void handleMessage(Net::MessageIn &msg) override final;

        void connect() override final;

        bool isConnected() const override final A_WARN_UNUSED;

        void disconnect() override final;

        unsigned int getMaxPasswordLength() const override final A_WARN_UNUSED
        { return 24; }

        void changePassword(const std::string &restrict oldPassword,
                            const std::string &restrict newPassword)
                            const override final;

        void sendVersion() const override final;

        ServerInfo *getCharServer() const override final A_WARN_UNUSED;

        int supportedOptionalActions() const override final A_WARN_UNUSED;

        void ping() const override final;

    protected:
        static void processLoginError2(Net::MessageIn &msg);

        static void processUpdateHost2(Net::MessageIn &msg);

        static void processServerVersion(Net::MessageIn &msg);

        static void processCondingKey(Net::MessageIn &msg);

        static void processCharPasswordResponse(Net::MessageIn &msg);

    private:
        void sendLoginRegister(const std::string &restrict username,
                               const std::string &restrict password,
                               const std::string &restrict email)
                               const override final;
};

}  // namespace EAthena

#endif  // NET_EATHENA_LOGINHANDLER_H
