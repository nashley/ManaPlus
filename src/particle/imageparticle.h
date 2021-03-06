/*
 *  The ManaPlus Client
 *  Copyright (C) 2006-2009  The Mana World Development Team
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

#ifndef PARTICLE_IMAGEPARTICLE_H
#define PARTICLE_IMAGEPARTICLE_H

#include "particle/particle.h"

#include "utils/stringmap.h"

class Image;

/**
 * A particle that uses an image for its visualization.
 */
class ImageParticle notfinal : public Particle
{
    public:
        /**
         * Constructor. The image is reference counted by this particle.
         *
         * @param map   the map this particle appears on
         * @param image an Image instance, may not be NULL
         */
        explicit ImageParticle(Image *const image);

        A_DELETE_COPY(ImageParticle)

        /**
         * Destructor.
         */
        virtual ~ImageParticle();

        /**
         * Draws the particle image
         */
        virtual void draw(Graphics *const graphics,
                          const int offsetX,
                          const int offsetY) const override final;

        void setAlpha(const float alpha) override final
        { mAlpha = alpha; }

        static StringIntMap imageParticleCountByName;
    protected:
        Image *mImage;   /**< The image used for this particle. */
};

#endif  // PARTICLE_IMAGEPARTICLE_H
