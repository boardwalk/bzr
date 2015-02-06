/*
 * Bael'Zharon's Respite
 * Copyright (C) 2014 Daniel Skorupski
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef BZR_EUI_H
#define BZR_EUI_H

/*
 * eui is a constraint-based, immediate mode gui
 * it is constraint-based in that is allows "auto layout" style "visual format language"
 * while being entirely immediate mode (objects are created on the fly each frame)
 * various mechanisms are used to cache constraints and other state
 * it aims to marry ImGui with proper layout and good looks.
 */
namespace eui {

struct Context
{
    unordered_map<uintptr_t, uintptr_t> addrCount;
};

struct Element
{
    Element();

    pair<uintptr_t, uintptr_t> name;
};

struct Text : public Element
{
    Text(const char* fmt, ...);
};

//
// Example login interface ui
//

/*
void showCharacterSelect()
{
    eui::Window window("Select character");

    eui::ListBox characterList(window);

    for(string& character : characters_)
    {
        eui::ListItem(characterList).setText(character);
    }

    eui::Button enterWorld("Enter world");
    eui::Button exitGame("Exit game");

    eui::addConstraint("|-[v]-[v]-|", characterList, enterWorld);
    eui::addConstraint("[v]-[v]-|", characterList, exitGame);
    eui::addConstraint("V:|-[v]-|", characterList);
    eui::addConstraint("V:|-[v]-[v]-|", enterWorldButton, exitGame);

    if(enterWorld.pushed())
    {
        // characterList.selectedIndex()
        // ...
    }
    else if(exitGame.push())
    {
        // ...
    }
}
*/

//
// Equivalents to the examples here
// https://developer.apple.com/library/ios/documentation/UserExperience/Conceptual/AutolayoutPG/VisualFormatLanguage/VisualFormatLanguage.html
//
// addConstraint("v-v", button, textField);
// addConstraint("[v(>=50)]", button);
// addConstraint("|-50-[v]-50-|", purpleBox);
// addConstraint("V:[v]-10-[v]", topField, bottomField);
// addConstraint("[v][v]", maroonView, blueView);
// addConstraint("[v(==v)]", button1, button2);
// addConstraint("[v(>=70,<=100)]", flexibleButton);
// addConstraint("|-[v]-[v]-[v(>=20)]-|", find, findNext, findField);
//
// Essentially, view names are replaced with "v" and
// (not shown here) metric names are replaced with "m"
// Views and metrics are not passed as dictionaries but directly as arguments to add constraint
//
void addConstraint(const char* fmt, ...);

void setContext(Context* ctx);

intptr_t pc();
void pushId(intptr_t id);
void pushId(const char* name);
void popId();

} // namespace eui

#endif