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

#include "gui/windows/statuswindow.h"

#include "configuration.h"
#include "gamemodifiers.h"
#include "item.h"
#include "settings.h"
#include "units.h"

#include "gui/windows/chatwindow.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "enums/being/attributes.h"

#include "gui/windows/equipmentwindow.h"
#include "gui/windows/setupwindow.h"

#include "gui/widgets/button.h"
#include "gui/widgets/containerplacer.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/layouttype.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/statuswindowattrs.h"
#include "gui/widgets/vertcontainer.h"
#include "gui/widgets/windowcontainer.h"

#include "net/gamehandler.h"
#include "net/inventoryhandler.h"
#include "net/playerhandler.h"

#include "utils/delete2.h"
#include "utils/gettext.h"

#include <SDL_timer.h>

#include "debug.h"

StatusWindow *statusWindow = nullptr;

StatusWindow::StatusWindow() :
    Window(localPlayer ? localPlayer->getName() :
        "?", Modal_false, nullptr, "status.xml"),
    ActionListener(),
    AttributeListener(),
    StatListener(),
    // TRANSLATORS: status window label
    mLvlLabel(new Label(this, strprintf(_("Level: %d"), 0))),
    // TRANSLATORS: status window label
    mMoneyLabel(new Label(this, strprintf(_("Money: %s"), ""))),
    // TRANSLATORS: status window label
    mHpLabel(new Label(this, _("HP:"))),
    mMpLabel(nullptr),
    // TRANSLATORS: status window label
    mXpLabel(new Label(this, _("Exp:"))),
    mHpBar(nullptr),
    mMpBar(nullptr),
    mXpBar(nullptr),
    mJobLvlLabel(nullptr),
    mJobLabel(nullptr),
    mJobBar(nullptr),
    mAttrCont(new VertContainer(this, 32)),
    mAttrScroll(new ScrollArea(this, mAttrCont, false)),
    mDAttrCont(new VertContainer(this, 32)),
    mDAttrScroll(new ScrollArea(this, mDAttrCont, false)),
    mCharacterPointsLabel(new Label(this, "C")),
    mCorrectionPointsLabel(nullptr),
    // TRANSLATORS: status window button
    mCopyButton(new Button(this, _("Copy to chat"), "copy", this)),
    mAttrs()
{
    setWindowName("Status");
    if (setupWindow)
        setupWindow->registerWindowForReset(this);
    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setStickyButtonLock(true);
    setDefaultSize((windowContainer->getWidth() - 480) / 2,
        (windowContainer->getHeight() - 500) / 2, 480, 500);

    if (localPlayer && !localPlayer->getRaceName().empty())
    {
        setCaption(strprintf("%s (%s)", localPlayer->getName().c_str(),
            localPlayer->getRaceName().c_str()));
    }

    int max = PlayerInfo::getAttribute(Attributes::MAX_HP);
    if (!max)
        max = 1;

    mHpBar = new ProgressBar(this, static_cast<float>(PlayerInfo::getAttribute(
        Attributes::HP)) / static_cast<float>(max), 80, 0, Theme::PROG_HP,
        "hpprogressbar.xml", "hpprogressbar_fill.xml");
    mHpBar->setColor(getThemeColor(Theme::HP_BAR),
        getThemeColor(Theme::HP_BAR_OUTLINE));

    max = PlayerInfo::getAttribute(Attributes::EXP_NEEDED);
    mXpBar = new ProgressBar(this, max ?
            static_cast<float>(PlayerInfo::getAttribute(Attributes::EXP))
            / static_cast<float>(max) : static_cast<float>(0), 80, 0,
            Theme::PROG_EXP, "xpprogressbar.xml", "xpprogressbar_fill.xml");
    mXpBar->setColor(getThemeColor(Theme::XP_BAR),
        getThemeColor(Theme::XP_BAR_OUTLINE));

    const bool magicBar = gameHandler->canUseMagicBar();
    const bool job = serverConfig.getValueBool("showJob", true);

    if (magicBar)
    {
        max = PlayerInfo::getAttribute(Attributes::MAX_MP);
        // TRANSLATORS: status window label
        mMpLabel = new Label(this, _("MP:"));
        const bool useMagic = playerHandler->canUseMagic();
        mMpBar = new ProgressBar(this, max ? static_cast<float>(
            PlayerInfo::getAttribute(Attributes::MAX_MP))
            / static_cast<float>(max) : static_cast<float>(0),
            80, 0, useMagic ? Theme::PROG_MP : Theme::PROG_NO_MP,
            useMagic ? "mpprogressbar.xml" : "nompprogressbar.xml",
            useMagic ? "mpprogressbar_fill.xml" : "nompprogressbar_fill.xml");
        if (useMagic)
        {
            mMpBar->setColor(getThemeColor(Theme::MP_BAR),
                getThemeColor(Theme::MP_BAR_OUTLINE));
        }
        else
        {
            mMpBar->setColor(getThemeColor(Theme::NO_MP_BAR),
                getThemeColor(Theme::NO_MP_BAR_OUTLINE));
        }
    }
    else
    {
        mMpLabel = nullptr;
        mMpBar = nullptr;
    }

    place(0, 0, mLvlLabel, 3);
    place(0, 1, mHpLabel).setPadding(3);
    place(1, 1, mHpBar, 4);
    place(5, 1, mXpLabel).setPadding(3);
    place(6, 1, mXpBar, 5);
    if (magicBar)
    {
        place(0, 2, mMpLabel).setPadding(3);
        // 5, 2 and 6, 2 Job Progress Bar
        if (job)
            place(1, 2, mMpBar, 4);
        else
            place(1, 2, mMpBar, 10);
    }

    if (job)
    {
        // TRANSLATORS: status window label
        mJobLvlLabel = new Label(this, strprintf(_("Job: %d"), 0));
        // TRANSLATORS: status window label
        mJobLabel = new Label(this, _("Job:"));
        mJobBar = new ProgressBar(this, 0.0F, 80, 0, Theme::PROG_JOB,
            "jobprogressbar.xml", "jobprogressbar_fill.xml");
        mJobBar->setColor(getThemeColor(Theme::JOB_BAR),
            getThemeColor(Theme::JOB_BAR_OUTLINE));

        place(3, 0, mJobLvlLabel, 3);
        place(5, 2, mJobLabel).setPadding(3);
        place(6, 2, mJobBar, 5);
        place(6, 0, mMoneyLabel, 3);
    }
    else
    {
        mJobLvlLabel = nullptr;
        mJobLabel = nullptr;
        mJobBar = nullptr;
        place(3, 0, mMoneyLabel, 3);
    }

    // ----------------------
    // Stats Part
    // ----------------------

    mAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
    place(0, 3, mAttrScroll, 5, 3);

    mDAttrScroll->setHorizontalScrollPolicy(ScrollArea::SHOW_NEVER);
    mDAttrScroll->setVerticalScrollPolicy(ScrollArea::SHOW_AUTO);
    place(6, 3, mDAttrScroll, 5, 3);

    getLayout().setRowHeight(3, LayoutType::SET);

    place(0, 6, mCharacterPointsLabel, 5);
    place(0, 5, mCopyButton);

    if (playerHandler->canCorrectAttributes())
    {
        mCorrectionPointsLabel = new Label(this, "C");
        place(0, 7, mCorrectionPointsLabel, 5);
    }

    loadWindowState();
    enableVisibleSound(true);

    // Update bars
    updateHPBar(mHpBar, true);
    if (magicBar)
        updateMPBar(mMpBar, true);
    updateXPBar(mXpBar, false);

    // TRANSLATORS: status window label
    mMoneyLabel->setCaption(strprintf(_("Money: %s"), Units::formatCurrency(
        PlayerInfo::getAttribute(Attributes::MONEY)).c_str()));
    mMoneyLabel->adjustSize();
    // TRANSLATORS: status window label
    mCharacterPointsLabel->setCaption(strprintf(_("Character points: %d"),
        PlayerInfo::getAttribute(Attributes::CHAR_POINTS)));
    mCharacterPointsLabel->adjustSize();

    updateLevelLabel();
}

void StatusWindow::updateLevelLabel()
{
    if (localPlayer && localPlayer->isGM())
    {
        // TRANSLATORS: status window label
        mLvlLabel->setCaption(strprintf(_("Level: %d (GM %d)"),
            PlayerInfo::getAttribute(Attributes::LEVEL),
            localPlayer->getGMLevel()));
    }
    else
    {
        // TRANSLATORS: status window label
        mLvlLabel->setCaption(strprintf(_("Level: %d"),
            PlayerInfo::getAttribute(Attributes::LEVEL)));
    }
    mLvlLabel->adjustSize();
}

void StatusWindow::statChanged(const int id,
                               const int oldVal1,
                               const int oldVal2 A_UNUSED)
{
    static bool blocked = false;
    if (blocked)
        return;

    if (id == Attributes::JOB)
    {
        if (mJobLvlLabel)
        {
            int lvl = PlayerInfo::getStatBase(id);
            const int oldExp = oldVal1;
            const std::pair<int, int> exp = PlayerInfo::getStatExperience(id);

            if (!lvl)
            {
                // possible server broken and don't send job level,
                // then we fixing it :)
                if (exp.second < 20000)
                {
                    lvl = 0;
                }
                else
                {
                    lvl = (exp.second - 20000) / 150;
                    blocked = true;
                    PlayerInfo::setStatBase(id, lvl);
                    blocked = false;
                }
            }

            if (exp.first < oldExp && exp.second >= 20000)
            {   // possible job level up. but server broken and don't send
                // new job exp limit, we fixing it
                lvl ++;
                blocked = true;
                PlayerInfo::setStatExperience(
                    id, exp.first, 20000 + lvl * 150);
                PlayerInfo::setStatBase(id, lvl);
                blocked = false;
            }

            // TRANSLATORS: status window label
            mJobLvlLabel->setCaption(strprintf(_("Job: %d"), lvl));
            mJobLvlLabel->adjustSize();

            updateProgressBar(mJobBar, id, false);
        }
    }
    else
    {
        updateMPBar(mMpBar, true);
        const Attrs::const_iterator it = mAttrs.find(id);
        if (it != mAttrs.end() && it->second)
            it->second->update();
    }
}

void StatusWindow::attributeChanged(const int id,
                                    const int oldVal A_UNUSED,
                                    const int newVal)
{
    switch (id)
    {
        case Attributes::HP:
        case Attributes::MAX_HP:
            updateHPBar(mHpBar, true);
            break;

        case Attributes::MP:
        case Attributes::MAX_MP:
            updateMPBar(mMpBar, true);
            break;

        case Attributes::EXP:
        case Attributes::EXP_NEEDED:
            updateXPBar(mXpBar, false);
            break;

        case Attributes::MONEY:
            // TRANSLATORS: status window label
            mMoneyLabel->setCaption(strprintf(_("Money: %s"),
                Units::formatCurrency(newVal).c_str()));
            mMoneyLabel->adjustSize();
            break;

        case Attributes::CHAR_POINTS:
            mCharacterPointsLabel->setCaption(strprintf(
                // TRANSLATORS: status window label
                _("Character points: %d"), newVal));

            mCharacterPointsLabel->adjustSize();
            // Update all attributes
            for (Attrs::const_iterator it = mAttrs.begin();
                 it != mAttrs.end(); ++it)
            {
                if (it->second)
                    it->second->update();
            }
            break;

        case Attributes::CORR_POINTS:
            mCorrectionPointsLabel->setCaption(strprintf(
                // TRANSLATORS: status window label
                _("Correction points: %d"), newVal));
            mCorrectionPointsLabel->adjustSize();
            // Update all attributes
            for (Attrs::const_iterator it = mAttrs.begin();
                 it != mAttrs.end(); ++it)
            {
                if (it->second)
                    it->second->update();
            }
            break;

        case Attributes::LEVEL:
            // TRANSLATORS: status window label
            mLvlLabel->setCaption(strprintf(_("Level: %d"), newVal));
            mLvlLabel->adjustSize();
            break;

        default:
            break;
    }
}

void StatusWindow::setPointsNeeded(const int id, const int needed)
{
    const Attrs::const_iterator it = mAttrs.find(id);

    if (it != mAttrs.end())
    {
        AttrDisplay *const disp = it->second;
        if (disp && disp->getType() == AttrDisplay::CHANGEABLE)
            static_cast<ChangeDisplay*>(disp)->setPointsNeeded(needed);
    }
}

void StatusWindow::addAttribute(const int id, const std::string &restrict name,
                                const std::string &restrict shortName,
                                const Modifiable modifiable)
{
    AttrDisplay *disp;

    if (modifiable == Modifiable_true)
    {
        disp = new ChangeDisplay(this, id, name, shortName);
        disp->update();
        mAttrCont->add1(disp);
    }
    else
    {
        disp = new DerDisplay(this, id, name, shortName);
        disp->update();
        mDAttrCont->add1(disp);
    }
    mAttrs[id] = disp;
}

void StatusWindow::clearAttributes()
{
    mAttrCont->clear();
    mDAttrCont->clear();
    FOR_EACH (Attrs::iterator, it, mAttrs)
        delete (*it).second;
    mAttrs.clear();
}

void StatusWindow::updateHPBar(ProgressBar *const bar, const bool showMax)
{
    if (!bar)
        return;

    const int hp = PlayerInfo::getAttribute(Attributes::HP);
    const int maxHp = PlayerInfo::getAttribute(Attributes::MAX_HP);
    if (showMax)
        bar->setText(toString(hp).append("/").append(toString(maxHp)));
    else
        bar->setText(toString(hp));

    float prog = 1.0;
    if (maxHp > 0)
        prog = static_cast<float>(hp) / static_cast<float>(maxHp);
    bar->setProgress(prog);
}

void StatusWindow::updateMPBar(ProgressBar *const bar,
                               const bool showMax) const
{
    if (!bar)
        return;

    const int mp = PlayerInfo::getAttribute(Attributes::MP);
    const int maxMp = PlayerInfo::getAttribute(Attributes::MAX_MP);
    if (showMax)
        bar->setText(toString(mp).append("/").append(toString(maxMp)));
    else
        bar->setText(toString(mp));

    float prog = 1.0F;
    if (maxMp > 0)
        prog = static_cast<float>(mp) / static_cast<float>(maxMp);

    if (playerHandler->canUseMagic())
    {
        bar->setColor(getThemeColor(Theme::MP_BAR),
            getThemeColor(Theme::MP_BAR_OUTLINE));
        bar->setProgressPalette(Theme::PROG_MP);
    }
    else
    {
        bar->setColor(getThemeColor(Theme::NO_MP_BAR),
            getThemeColor(Theme::NO_MP_BAR_OUTLINE));
        bar->setProgressPalette(Theme::PROG_NO_MP);
    }

    bar->setProgress(prog);
}

void StatusWindow::updateProgressBar(ProgressBar *const bar, const int value,
                                     const int max, const bool percent)
{
    if (!bar)
        return;

    if (max == 0)
    {
        // TRANSLATORS: status bar label
        bar->setText(_("Max"));
        bar->setProgress(1);
        bar->setText(toString(value));
    }
    else
    {
        const float progress = static_cast<float>(value)
            / static_cast<float>(max);
        if (percent)
        {
            bar->setText(strprintf("%2.5f%%",
                static_cast<double>(100 * progress)));
        }
        else
        {
            bar->setText(toString(value).append("/").append(toString(max)));
        }
        bar->setProgress(progress);
    }
}

void StatusWindow::updateXPBar(ProgressBar *const bar, const bool percent)
{
    if (!bar)
        return;

    updateProgressBar(bar, PlayerInfo::getAttribute(Attributes::EXP),
        PlayerInfo::getAttribute(Attributes::EXP_NEEDED), percent);
}

void StatusWindow::updateJobBar(ProgressBar *const bar, const bool percent)
{
    if (!bar)
        return;

    const std::pair<int, int> exp =  PlayerInfo::getStatExperience(
        Attributes::JOB);
    updateProgressBar(bar, exp.first, exp.second, percent);
}

void StatusWindow::updateProgressBar(ProgressBar *const bar, const int id,
                                     const bool percent)
{
    const std::pair<int, int> exp =  PlayerInfo::getStatExperience(id);
    updateProgressBar(bar, exp.first, exp.second, percent);
}

void StatusWindow::updateWeightBar(ProgressBar *const bar)
{
    if (!bar)
        return;

    if (PlayerInfo::getAttribute(Attributes::MAX_WEIGHT) == 0)
    {
        // TRANSLATORS: status bar label
        bar->setText(_("Max"));
        bar->setProgress(1.0);
    }
    else
    {
        const int totalWeight = PlayerInfo::getAttribute(
            Attributes::TOTAL_WEIGHT);
        const int maxWeight = PlayerInfo::getAttribute(Attributes::MAX_WEIGHT);
        float progress = 1.0F;
        if (maxWeight)
        {
            progress = static_cast<float>(totalWeight)
                / static_cast<float>(maxWeight);
        }
        bar->setText(strprintf("%s/%s", Units::formatWeight(
            totalWeight).c_str(), Units::formatWeight(maxWeight).c_str()));
        bar->setProgress(progress);
    }
}

void StatusWindow::updateMoneyBar(ProgressBar *const bar)
{
    if (!bar)
        return;

    const int money = PlayerInfo::getAttribute(Attributes::MONEY);
    bar->setText(Units::formatCurrency(money));
    if (money > 0)
    {
        const float progress = static_cast<float>(money)
            / static_cast<float>(1000000000);
        bar->setProgress(progress);
    }
    else
    {
        bar->setProgress(1.0);
    }
}

void StatusWindow::updateArrowsBar(ProgressBar *const bar)
{
    if (!bar || !equipmentWindow)
        return;

    const Item *const item = equipmentWindow->getEquipment(
        inventoryHandler->getProjectileSlot());

    if (item && item->getQuantity() > 0)
        bar->setText(toString(item->getQuantity()));
    else
        bar->setText("0");
}

void StatusWindow::updateInvSlotsBar(ProgressBar *const bar)
{
    if (!bar)
        return;

    const Inventory *const inv = PlayerInfo::getInventory();
    if (!inv)
        return;

    const int usedSlots = inv->getNumberOfSlotsUsed();
    const int maxSlots = inv->getSize();

    if (maxSlots)
    {
        bar->setProgress(static_cast<float>(usedSlots)
            / static_cast<float>(maxSlots));
    }

    bar->setText(strprintf("%d", usedSlots));
}

std::string StatusWindow::translateLetter(const char *const letters)
{
    char buf[2];
    char *const str = gettext(letters);
    if (strlen(str) != 3)
        return letters;

    buf[0] = str[1];
    buf[1] = 0;
    return std::string(buf);
}

std::string StatusWindow::translateLetter2(const std::string &letters)
{
    if (letters.size() < 5)
        return "";

    return std::string(gettext(letters.substr(1, 1).c_str()));
}

void StatusWindow::updateStatusBar(ProgressBar *const bar,
                                   const bool percent A_UNUSED) const
{
    bar->setText(translateLetter2(GameModifiers::getMoveTypeString())
        .append(translateLetter2(GameModifiers::getCrazyMoveTypeString()))
        .append(translateLetter2(GameModifiers::getMoveToTargetTypeString()))
        .append(translateLetter2(GameModifiers::getFollowModeString()))
        .append(" ").append(translateLetter2(
        GameModifiers::getAttackWeaponTypeString()))
        .append(translateLetter2(GameModifiers::getAttackTypeString()))
        .append(translateLetter2(GameModifiers::getMagicAttackTypeString()))
        .append(translateLetter2(GameModifiers::getPvpAttackTypeString()))
        .append(" ").append(translateLetter2(
        GameModifiers::getQuickDropCounterString()))
        .append(translateLetter2(GameModifiers::getPickUpTypeString()))
        .append(" ").append(translateLetter2(
        GameModifiers::getMapDrawTypeString()))
        .append(" ").append(translateLetter2(
        GameModifiers::getImitationModeString()))
        .append(translateLetter2(GameModifiers::getCameraModeString()))
        .append(translateLetter2(GameModifiers::getAwayModeString())));

    bar->setProgress(50);
    if (settings.disableGameModifiers)
        bar->setBackgroundColor(getThemeColor(Theme::STATUSBAR_ON));
    else
        bar->setBackgroundColor(getThemeColor(Theme::STATUSBAR_OFF));
}

void StatusWindow::action(const ActionEvent &event)
{
    if (!chatWindow)
        return;

    if (event.getId() == "copy")
    {
        Attrs::const_iterator it = mAttrs.begin();
        const Attrs::const_iterator it_end = mAttrs.end();
        std::string str;
        while (it != it_end)
        {
            const ChangeDisplay *const attr = dynamic_cast<ChangeDisplay*>(
                (*it).second);
            if (attr)
            {
                str.append(strprintf("%s:%s ", attr->getShortName().c_str(),
                    attr->getValue().c_str()));
            }
            ++ it;
        }
        chatWindow->addInputText(str);
    }
}

AttrDisplay::AttrDisplay(const Widget2 *const widget,
                         const int id, const std::string &restrict name,
                         const std::string &restrict shortName) :
    Container(widget),
    mId(id),
    mName(name),
    mShortName(shortName),
    mLayout(new LayoutHelper(this)),
    mLabel(new Label(this, name)),
    mValue(new Label(this, "1 "))
{
    setSize(100, 32);

    mLabel->setAlignment(Graphics::CENTER);
    mValue->setAlignment(Graphics::CENTER);
}

AttrDisplay::~AttrDisplay()
{
    delete2(mLayout);
}

std::string AttrDisplay::update()
{
    const int base = PlayerInfo::getStatBase(mId);
    const int bonus = PlayerInfo::getStatMod(mId);
    std::string value = toString(base + bonus);
    if (bonus)
        value.append(strprintf("=%d%+d", base, bonus));
    mValue->setCaption(value);
    return mName;
}

DerDisplay::DerDisplay(const Widget2 *const widget,
                       const int id, const std::string &restrict name,
                       const std::string &restrict shortName) :
    AttrDisplay(widget, id, name, shortName)
{
    ContainerPlacer place = mLayout->getPlacer(0, 0);

    place(0, 0, mLabel, 3);
    place(3, 0, mValue, 2);
}

ChangeDisplay::ChangeDisplay(const Widget2 *const widget,
                             const int id, const std::string &restrict name,
                             const std::string &restrict shortName) :
    AttrDisplay(widget, id, name, shortName),
    ActionListener(),
    mNeeded(1),
    // TRANSLATORS: status window label
    mPoints(new Label(this, _("Max"))),
    mDec(nullptr),
    // TRANSLATORS: status window label (plus sign)
    mInc(new Button(this, _("+"), "inc", this))
{
    // Do the layout
    ContainerPlacer place = mLayout->getPlacer(0, 0);

    place(0, 0, mLabel, 3);
    place(4, 0, mValue, 2);
    place(6, 0, mInc);
    place(7, 0, mPoints);

    if (playerHandler->canCorrectAttributes())
    {
        // TRANSLATORS: status window label (minus sign)
        mDec = new Button(this, _("-"), "dec", this);
        mDec->setWidth(mInc->getWidth());

        place(3, 0, mDec);
    }
}

std::string ChangeDisplay::update()
{
    if (mNeeded > 0)
    {
        mPoints->setCaption(toString(mNeeded));
    }
    else
    {
        // TRANSLATORS: status bar label
        mPoints->setCaption(_("Max"));
    }

    if (mDec)
        mDec->setEnabled(PlayerInfo::getAttribute(Attributes::CORR_POINTS));

    mInc->setEnabled(PlayerInfo::getAttribute(Attributes::CHAR_POINTS)
        >= mNeeded && mNeeded > 0);

    return AttrDisplay::update();
}

void ChangeDisplay::setPointsNeeded(const int needed)
{
    mNeeded = needed;
    update();
}

void ChangeDisplay::action(const ActionEvent &event)
{
    if (playerHandler->canCorrectAttributes() &&
        event.getSource() == mDec)
    {
        const int newcorrpoints = PlayerInfo::getAttribute(
            Attributes::CORR_POINTS);
        PlayerInfo::setAttribute(Attributes::CORR_POINTS, newcorrpoints - 1);

        const int newpoints = PlayerInfo::getAttribute(
            Attributes::CHAR_POINTS) + 1;
        PlayerInfo::setAttribute(Attributes::CHAR_POINTS, newpoints);

        const int newbase = PlayerInfo::getStatBase(mId) - 1;
        PlayerInfo::setStatBase(mId, newbase);

        playerHandler->decreaseAttribute(mId);
    }
    else if (event.getSource() == mInc)
    {
        int cnt = 1;
        if (config.getBoolValue("quickStats"))
        {
            cnt = mInc->getClickCount();
            if (cnt > 10)
                cnt = 10;
        }

        const int newpoints = PlayerInfo::getAttribute(
            Attributes::CHAR_POINTS) - cnt;
        PlayerInfo::setAttribute(Attributes::CHAR_POINTS, newpoints);

        const int newbase = PlayerInfo::getStatBase(mId) + cnt;
        PlayerInfo::setStatBase(mId, newbase);

        for (unsigned f = 0; f < mInc->getClickCount(); f ++)
        {
            playerHandler->increaseAttribute(mId);
            if (cnt != 1)
                SDL_Delay(100);
        }
    }
}
