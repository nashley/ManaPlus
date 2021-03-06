/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2015  The ManaPlus Developers
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

#include "gui/widgets/colorpage.h"

#include "gui/models/colormodel.h"

#include "gui/skin.h"

#include "gui/fonts/font.h"

#include "debug.h"

ColorPage::ColorPage(const Widget2 *const widget,
                     ListModel *const listModel,
                     const std::string &skin) :
    ListBox(widget, listModel, skin)
{
    mItemPadding = mSkin ? mSkin->getOption("itemPadding") : 1;
    mRowHeight = 13;
    const Font *const font = getFont();
    if (font)
        mRowHeight = font->getHeight() + 2 * mItemPadding;
    if (mListModel)
    {
        setHeight(getRowHeight() * mListModel->getNumberOfElements()
            + 2 * mPadding + 20);
    }
}

ColorPage::~ColorPage()
{
}

void ColorPage::draw(Graphics *graphics)
{
    BLOCK_START("ColorPage::draw")

    const ColorModel *const model = static_cast<ColorModel* const>(
        mListModel);

    mHighlightColor.a = static_cast<int>(mAlpha * 255.0F);
    graphics->setColor(mHighlightColor);
    updateAlpha();
    Font *const font = getFont();

    const int rowHeight = getRowHeight();
    const int width = mDimension.width;

    if (mSelected >= 0)
    {
        graphics->fillRectangle(Rect(mPadding,
            rowHeight * mSelected + mPadding,
            mDimension.width - 2 * mPadding, rowHeight));

        const ColorPair *const colors = model->getColorAt(mSelected);
        graphics->setColorAll(*colors->color1, *colors->color2);
        const std::string str = mListModel->getElementAt(mSelected);
        font->drawString(graphics, str, (width - font->getWidth(str)) / 2,
            mSelected * rowHeight + mPadding);
    }

    graphics->setColorAll(mForegroundColor, mForegroundColor2);
    const int sz = mListModel->getNumberOfElements();
    for (int i = 0, y = mPadding; i < sz; ++i, y += rowHeight)
    {
        if (i != mSelected)
        {
            const ColorPair *const colors = model->getColorAt(i);
            graphics->setColorAll(*colors->color1, *colors->color2);
            const std::string str = mListModel->getElementAt(i);
            font->drawString(graphics, str, (width - font->getWidth(str)) / 2,
                y);
        }
    }

    BLOCK_END("ColorPage::draw")
}

void ColorPage::resetAction()
{
    setSelected(-1);
}

void ColorPage::adjustSize()
{
    BLOCK_START("ColorPage::adjustSize")
    if (mListModel)
    {
        setHeight(getRowHeight() * mListModel->getNumberOfElements()
            + 2 * mPadding + 20);
    }
    BLOCK_END("ColorPage::adjustSize")
}
