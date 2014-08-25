/*
 *  The ManaPlus Client
 *  Copyright (C) 2007  Joshua Langley <joshlangley@optusnet.com.au>
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "gui/setupactiondata.h"

#include "input/inputaction.h"
#include "input/inputactiondata.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include "debug.h"

SetupActionData setupActionDataEmotes[] =
{
    {
        // TRANSLATORS: input action name
        N_("Emote modifiers keys"),
        InputAction::NO_VALUE,
        ""
    },
    {
        // TRANSLATORS: input action name
        N_("Emote modifier key"),
        InputAction::EMOTE,
        "",
    },
    {
        // TRANSLATORS: input action name
        N_("Emote shortcuts"),
        InputAction::NO_VALUE,
        ""
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 1),
        InputAction::EMOTE_1,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 2),
        InputAction::EMOTE_2,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 3),
        InputAction::EMOTE_3,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 4),
        InputAction::EMOTE_4,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 5),
        InputAction::EMOTE_5,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 6),
        InputAction::EMOTE_6,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 7),
        InputAction::EMOTE_7,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 8),
        InputAction::EMOTE_8,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 9),
        InputAction::EMOTE_9,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 10),
        InputAction::EMOTE_10,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 11),
        InputAction::EMOTE_11,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 12),
        InputAction::EMOTE_12,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 13),
        InputAction::EMOTE_13,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 14),
        InputAction::EMOTE_14,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 15),
        InputAction::EMOTE_15,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 16),
        InputAction::EMOTE_16,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 17),
        InputAction::EMOTE_17,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 18),
        InputAction::EMOTE_18,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 19),
        InputAction::EMOTE_19,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 20),
        InputAction::EMOTE_20,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 21),
        InputAction::EMOTE_21,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 22),
        InputAction::EMOTE_22,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 23),
        InputAction::EMOTE_23,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 24),
        InputAction::EMOTE_24,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 25),
        InputAction::EMOTE_25,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 26),
        InputAction::EMOTE_26,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 27),
        InputAction::EMOTE_27,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 28),
        InputAction::EMOTE_28,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 29),
        InputAction::EMOTE_29,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 30),
        InputAction::EMOTE_30,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 31),
        InputAction::EMOTE_31,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 32),
        InputAction::EMOTE_32,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 33),
        InputAction::EMOTE_33,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 34),
        InputAction::EMOTE_34,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 35),
        InputAction::EMOTE_35,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 36),
        InputAction::EMOTE_36,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 37),
        InputAction::EMOTE_37,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 38),
        InputAction::EMOTE_38,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 39),
        InputAction::EMOTE_39,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 40),
        InputAction::EMOTE_40,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 41),
        InputAction::EMOTE_41,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 42),
        InputAction::EMOTE_42,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 43),
        InputAction::EMOTE_43,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 44),
        InputAction::EMOTE_44,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 45),
        InputAction::EMOTE_45,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 46),
        InputAction::EMOTE_46,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 47),
        InputAction::EMOTE_47,
        "",
    },
    {
        // TRANSLATORS: input action name
        strprintf(N_("Emote Shortcut %d"), 48),
        InputAction::EMOTE_48,
        "",
    },
    {
        "",
        InputAction::NO_VALUE,
        ""
    }
};