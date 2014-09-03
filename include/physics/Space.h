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
#ifndef BZR_PHYSICS_SPACE_H
#define BZR_PHYSICS_SPACE_H

#include "Noncopyable.h"

class Body;

/*
 * Asheron's Call uses a very simple physics model
 * The static environment consists of:
 *   The landscape
 *     A heightmap, effectively just a z value for any x and y
 *     A player on the ground cannot traverse an area with a slope greater than a certain amount
 *     A player who makes contact with an impassable slope will slide down the slope, uncontrollable
 *     A player cannot traverse onto "deep water" areas -- these are effectively infinitely high walls
 *   The structures and doodads on the landscape and in those structures
 *     A set of polygons the player cannot pass through
 *   Dynamic objects (players, doors, etc)
 *     Player killer vs player killer => zero restitution collision, each player cannot move closer but can move "against" each other perpendicularly.
 *       Effectively each is treated as an immovable object.
 *     Non-player killer vs any player => no collision whatsoever
 *     Door (for ex) vs player => Cannot stop door (will move through player), cannot move player.
 *       Again, effectively immovable objects.
 *   "Bumpers"
 *     You cannot traverse when the ground drops too quickly, both on the landscape and on structures, unless you are in the air.
 *     (this could be the same rule as the landscape steepness rule)
 *   Ground physics
 *     You may never leave the ground *unless* you jump.
 *     You are effectively stuck to the ground, no matter how fast you travel or steep the terrain becomes.
 *     You may move and turn in any direction in agreement with the bumper rule
 *   Air physics
 *     Initiated when jumping or falling (f.e. when leaving a portal)
 *     You may only turn, there is no other "air control".
 *     Continues while you are either in the air or contacting an impassable slope.
 *     Impassable slopes "deflect" you along them, but you fall at roughly the same speed.
 *
 *   Boiling down the rules here:
 *     state:
 *       position (landblock, x, y, z)
 *       velocity (dx, dy, dz)
 *       orientation (rw, rx, ry, rz) // players can only rotation around z axis
 *     parameters:
 *       gravity (-z m/s**2, perhaps? while in the air)
 *       terminal velocity (-z m/s while in the air)
 *       run speed (maximum velocity in m/s), walk speed
 *       running acceleration/deceleration curve (m/s**2, perhaps?) // players do not move 0% to 100% and 100% to 0 instantly
 *       maximum slope
 *
 * +-----+
 * |     |  / A
 * |     | /
 * +-----+ ---> V
 *         \
 *          \ B 
 *
 * Raycast towards A & B
 * While on ground:
 *   If there is a object within an appreciable distance towards A, STOP!
 *   If there is *NO* object within an appreciable distance towards B, STOP!
 *
 * Another idea:
 *  The absolute value of your speed normalized change in Z in a single frame is greater than (param), STOP!
 *
 * Collision detection given two shapes.
 *  Shapes are either:
 *    A polygonal mesh
 *    Or a heightmap (effectively a very large polygonal mesh!)
 *  Take velocity of each shape
 *  Find closest possible point (contact point) and stop each shape perpendicular to the contact normal at that point.
 */

class Space : Noncopyable
{
public:
    void step(fp_t dt);

private:
    void stepBody(Body& object, fp_t dt);

    vector<weak_ptr<Body>> _bodies;
};

#endif