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

#ifndef RENDER_IMAGEGRAPHICS_H
#define RENDER_IMAGEGRAPHICS_H

#ifdef USE_OPENGL

#include "render/graphics.h"

#include "localconsts.h"

class Image;
class ImageCollection;
class ImageVertexes;

struct SDL_Surface;

/**
 * A central point of control for graphics.
 */
class ImegeGraphics final : public Graphics
{
    public:
        ImegeGraphics();

        A_DELETE_COPY(ImegeGraphics)

        ~ImegeGraphics();

        void setTarget(Image *const target)
        { mTarget = target; }

        Image *getTarget() const
        { return mTarget; }

        void beginDraw() override final
        { }

        void endDraw() override final
        { }

        void pushClipArea(const Rect &rect A_UNUSED) override final
        { }

        void popClipArea() override final
        { }

        void drawRescaledImage(const Image *const image A_UNUSED,
                               int dstX A_UNUSED, int dstY A_UNUSED,
                               const int desiredWidth A_UNUSED,
                               const int desiredHeight A_UNUSED)
                               override final
        { }

        void drawPattern(const Image *const image A_UNUSED,
                         const int x A_UNUSED,
                         const int y A_UNUSED,
                         const int w A_UNUSED,
                         const int h A_UNUSED) override final
        { }

        void drawRescaledPattern(const Image *const image A_UNUSED,
                                 const int x A_UNUSED,
                                 const int y A_UNUSED,
                                 const int w A_UNUSED,
                                 const int h A_UNUSED,
                                 const int scaledWidth A_UNUSED,
                                 const int scaledHeight A_UNUSED)
                                 override final
        { }

        void calcPattern(ImageVertexes *const vert A_UNUSED,
                         const Image *const image A_UNUSED,
                         const int x A_UNUSED,
                         const int y A_UNUSED,
                         const int w A_UNUSED,
                         const int h A_UNUSED) const override final
        { }

        void calcPattern(ImageCollection *const vert A_UNUSED,
                         const Image *const image A_UNUSED,
                         const int x A_UNUSED,
                         const int y A_UNUSED,
                         const int w A_UNUSED,
                         const int h A_UNUSED) const override final
        { }

        void calcTileVertexes(ImageVertexes *const vert A_UNUSED,
                              const Image *const image A_UNUSED,
                              int x A_UNUSED,
                              int y A_UNUSED) const override final
        { }

        void calcTileSDL(ImageVertexes *const vert A_UNUSED,
                         int x A_UNUSED, int y A_UNUSED) const override final
        { }

        void calcTileCollection(ImageCollection *const vertCol A_UNUSED,
                                const Image *const image A_UNUSED,
                                int x A_UNUSED, int y A_UNUSED) override final
        { }

        void drawTileVertexes(const ImageVertexes *const
                              vert A_UNUSED) override final
        { }

        void drawTileCollection(const ImageCollection *const vertCol A_UNUSED)
                                override final
        { }

        void updateScreen() override final
        { }

        SDL_Surface *getScreenshot() override final A_WARN_UNUSED
        { return nullptr; }

        void drawNet(const int x1 A_UNUSED,
                     const int y1 A_UNUSED,
                     const int x2 A_UNUSED,
                     const int y2 A_UNUSED,
                     const int width A_UNUSED,
                     const int height A_UNUSED) override final
        { }

        void calcWindow(ImageCollection *const vertCol A_UNUSED,
                        const int x A_UNUSED, const int y A_UNUSED,
                        const int w A_UNUSED, const int h A_UNUSED,
                        const ImageRect &imgRect A_UNUSED) override final
        { }

        void fillRectangle(const Rect &rect A_UNUSED) override final
        { }

        void drawRectangle(const Rect &rect A_UNUSED) override final
        { }

        void drawPoint(int x A_UNUSED, int y A_UNUSED) override final
        { }

        void drawLine(int x1 A_UNUSED, int y1 A_UNUSED,
                      int x2 A_UNUSED, int y2 A_UNUSED) override final
        { }

        bool setVideoMode(const int w A_UNUSED, const int h A_UNUSED,
                          const int scale A_UNUSED,
                          const int bpp A_UNUSED,
                          const bool fs A_UNUSED, const bool hwaccel A_UNUSED,
                          const bool resize A_UNUSED,
                          const bool noFrame A_UNUSED) override final
        { return false; }

        void drawImage(const Image *const image,
                       int dstX, int dstY) override final;

        void copyImage(const Image *const image,
                       int dstX, int dstY) override final;

        void drawImageCached(const Image *const image,
                             int x, int y) override final;

        void drawPatternCached(const Image *const image A_UNUSED,
                               const int x A_UNUSED,
                               const int y A_UNUSED,
                               const int w A_UNUSED,
                               const int h A_UNUSED) override final
        { }

        void completeCache() override final;

        /**
         * Draws a rectangle using images. 4 corner images, 4 side images and 1
         * image for the inside.
         */
        void drawImageRect(const int x A_UNUSED, const int y A_UNUSED,
                           const int w A_UNUSED, const int h A_UNUSED,
                           const ImageRect &imgRect A_UNUSED) override final
        { }

    protected:
        Image *mTarget;
};

#endif  // USE_OPENGL
#endif  // RENDER_IMAGEGRAPHICS_H
