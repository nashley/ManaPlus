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

#ifndef LISTENERS_GUICONFIGLISTENER_H
#define LISTENERS_GUICONFIGLISTENER_H

#include "configuration.h"

#include "gui/gui.h"

#include "listeners/configlistener.h"

#include "localconsts.h"

class GuiConfigListener final : public ConfigListener
{
    public:
        explicit GuiConfigListener(Gui *const g) :
            mGui(g)
        {}

        A_DELETE_COPY(GuiConfigListener)

        virtual ~GuiConfigListener()
        {
            CHECKLISTENERS
        }

        void optionChanged(const std::string &name)
        {
            if (!mGui)
                return;
            if (name == "customcursor")
                mGui->setUseCustomCursor(config.getBoolValue("customcursor"));
            else if (name == "doubleClick")
                mGui->setDoubleClick(config.getBoolValue("doubleClick"));
        }
    private:
        Gui *mGui;
};

#endif  // LISTENERS_GUICONFIGLISTENER_H
