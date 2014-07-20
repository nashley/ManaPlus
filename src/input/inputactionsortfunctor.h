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

#ifndef INPUT_INPUTACTIONSORTFUNCTOR_H
#define INPUT_INPUTACTIONSORTFUNCTOR_H

#include "input/inputactiondata.h"

#include "localconsts.h"

class InputActionSortFunctor final
{
    public:
        bool operator() (const int key1, const int key2) const
        {
            return keys[key1].priority >= keys[key2].priority;
        }

        const InputActionData *keys;
};

#endif  // INPUT_INPUTACTIONSORTFUNCTOR_H