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

#ifndef RENDER_NORMALOPENGLGRAPHICS_H
#define RENDER_NORMALOPENGLGRAPHICS_H

#if defined USE_OPENGL && !defined ANDROID

#include "localconsts.h"
#include "render/graphics.h"

#include "resources/fboinfo.h"

#ifdef ANDROID
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLES2/gl2.h>
#else
#ifndef USE_SDL2
#define GL_GLEXT_PROTOTYPES 1
#endif
#include <SDL_opengl.h>
#include <GL/glext.h>
#endif

class OpenGLGraphicsVertexes;

class NormalOpenGLGraphics final : public Graphics
{
    public:
        NormalOpenGLGraphics();

        A_DELETE_COPY(NormalOpenGLGraphics)

        ~NormalOpenGLGraphics();

        inline void drawQuadArrayfi(const int size);

        inline void drawQuadArrayfiCached(const int size);

        inline void drawQuadArrayfi(const GLint *const intVertArray,
                                    const GLfloat *const floatTexArray,
                                    const int size);

        inline void drawQuadArrayii(const int size);

        inline void drawQuadArrayiiCached(const int size);

        inline void drawQuadArrayii(const GLint *const intVertArray,
                                    const GLint *const intTexArray,
                                    const int size);

        inline void drawLineArrayi(const int size);

        inline void drawLineArrayf(const int size);

        void testDraw() override final;

        #include "render/graphicsdef.hpp"

        #include "render/openglgraphicsdef.hpp"

        #include "render/openglgraphicsdef1.hpp"

        #include "render/openglgraphicsdefadvanced.hpp"

#ifdef DEBUG_BIND_TEXTURE
        unsigned int getBinds() const
        { return mLastBinds; }
#endif

    private:
        GLfloat *mFloatTexArray;
        GLint *mIntTexArray;
        GLint *mIntVertArray;
        GLfloat *mFloatTexArrayCached;
        GLint *mIntTexArrayCached;
        GLint *mIntVertArrayCached;
        float mAlphaCached;
        int mVpCached;
        bool mTexture;

        bool mIsByteColor;
        Color mByteColor;
        GLuint mImageCached;
        float mFloatColor;
        int mMaxVertices;
        bool mColorAlpha;
#ifdef DEBUG_BIND_TEXTURE
        std::string mOldTexture;
        unsigned int mOldTextureId;
        static unsigned int mBinds;
        static unsigned int mLastBinds;
#endif
        FBOInfo mFbo;
};
#endif

#endif  // RENDER_NORMALOPENGLGRAPHICS_H
