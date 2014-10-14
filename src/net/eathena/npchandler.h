/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#ifndef NET_EATHENA_NPCHANDLER_H
#define NET_EATHENA_NPCHANDLER_H

#include "net/ea/npchandler.h"

#include "net/eathena/messagehandler.h"

namespace EAthena
{

class NpcHandler final : public MessageHandler, public Ea::NpcHandler
{
    public:
        NpcHandler();

        A_DELETE_COPY(NpcHandler)

        void handleMessage(Net::MessageIn &msg) override final;

        void talk(const int npcId) const override final;

        void nextDialog(const int npcId) const override final;

        void closeDialog(const int npcId) override final;

        void listInput(const int npcId,
                       const unsigned char value) const override final;

        void integerInput(const int npcId,
                          const int value) const override final;

        void stringInput(const int npcId,
                         const std::string &value) const override final;

        void buy(const int beingId) const override final;

        void sell(const int beingId) const override final;

        void buyItem(const int beingId, const int itemId,
                     const unsigned char color,
                     const int amount) const override final;

        void sellItem(const int beingId, const int itemId,
                      const int amount) const override final;

        void completeProgressBar() const override final;

        void produceMix(const int nameId,
                        const int materialId1,
                        const int materialId2,
                        const int materialId3) const override final;

        void cooking(const CookingType::Type type,
                     const int nameId) const override final;

        void repair(const int index) const override final;

        void refine(const int index) const override final;

        int getNpc(Net::MessageIn &msg) override final;

        void identify(const int index) const override final;

        void selectArrow(const int nameId) const override final;

        void selectAutoSpell(const int skillId) const override final;

    protected:
        static void processNpcCutin(Net::MessageIn &msg);

        static void processNpcViewPoint(Net::MessageIn &msg);

        void processNpcShowProgressBar(Net::MessageIn &msg) const;

        void processNpcCloseTimeout(Net::MessageIn &msg) const;
};

}  // namespace EAthena

#endif  // NET_EATHENA_NPCHANDLER_H
