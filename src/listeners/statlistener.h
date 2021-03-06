/*
 *  The ManaPlus Client
 *  Copyright (C) 2014-2015  The ManaPlus Developers
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

#ifndef LISTENERS_STATLISTENER_H
#define LISTENERS_STATLISTENER_H

#include "listeners/baselistener.hpp"

#include "localconsts.h"

class StatListener notfinal
{
    public:
        virtual void statChanged(const int id,
                                 const int oldVal1,
                                 const int oldVal2) = 0;

        static void distributeEvent(const int id,
                                    const int oldVal1,
                                    const int oldVal2);

    defineListenerHeader(StatListener)
};

#endif  // LISTENERS_STATLISTENER_H
