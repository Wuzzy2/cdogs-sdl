/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (C) 1995 Ronny Wester
    Copyright (C) 2003 Jeremy Chin
    Copyright (C) 2003-2007 Lucas Martin-King

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    This file incorporates work covered by the following copyright and
    permission notice:

    Copyright (c) 2013, Cong Xu
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/
#include "ai_utils.h"

#include <assert.h>
#include <math.h>

#include "AStar.h"
#include "collision.h"
#include "map.h"
#include "objs.h"


TActor *AIGetClosestPlayer(Vec2i pos)
{
	int i;
	int minDistance = -1;
	TActor *closestPlayer = NULL;
	for (i = 0; i < gOptions.numPlayers; i++)
	{
		if (IsPlayerAlive(i))
		{
			TActor *p = gPlayers[i];
			Vec2i pPos = Vec2iFull2Real(Vec2iNew(p->x, p->y));
			int distance = CHEBYSHEV_DISTANCE(pos.x, pos.y, pPos.x, pPos.y);
			if (!closestPlayer || distance < minDistance)
			{
				closestPlayer = p;
				minDistance = distance;
			}
		}
	}
	return closestPlayer;
}

static TActor *AIGetClosestActor(Vec2i from, int (*compFunc)(TActor *))
{
	// Search all the actors and find the closest one that
	// satisfies the condition
	TActor *a;
	TActor *closest = NULL;
	int minDistance = -1;
	for (a = ActorList(); a; a = a->next)
	{
		int distance;
		// Never target invulnerables or victims
		if (a->flags & (FLAGS_INVULNERABLE | FLAGS_VICTIM))
		{
			continue;
		}
		if (compFunc(a))
		{
			distance = CHEBYSHEV_DISTANCE(from.x, from.y, a->x, a->y);
			if (!closest || distance < minDistance)
			{
				minDistance = distance;
				closest = a;
			}
		}
	}
	return closest;
}

static int IsGood(TActor *a)
{
	return a->pData || (a->flags & FLAGS_GOOD_GUY);
}
static int IsBad(TActor *a)
{
	return !IsGood(a);
}
TActor *AIGetClosestEnemy(Vec2i from, int flags, int isPlayer)
{
	if (!isPlayer && !(flags & FLAGS_GOOD_GUY))
	{
		// we are bad; look for good guys
		return AIGetClosestActor(from, IsGood);
	}
	else
	{
		// we are good; look for bad guys
		return AIGetClosestActor(from, IsBad);
	}
}

static int IsGoodAndVisible(TActor *a)
{
	return IsGood(a) && (a->flags & FLAGS_VISIBLE);
}
static int IsBadAndVisible(TActor *a)
{
	return IsBad(a) && (a->flags & FLAGS_VISIBLE);
}
TActor *AIGetClosestVisibleEnemy(Vec2i from, int flags, int isPlayer)
{
	if (!isPlayer && !(flags & FLAGS_GOOD_GUY))
	{
		// we are bad; look for good guys
		return AIGetClosestActor(from, IsGoodAndVisible);
	}
	else
	{
		// we are good; look for bad guys
		return AIGetClosestActor(from, IsBadAndVisible);
	}
}

Vec2i AIGetClosestPlayerPos(Vec2i pos)
{
	TActor *closestPlayer = AIGetClosestPlayer(pos);
	if (closestPlayer)
	{
		return Vec2iFull2Real(Vec2iNew(closestPlayer->x, closestPlayer->y));
	}
	else
	{
		return pos;
	}
}

int AIReverseDirection(int cmd)
{
	if (cmd & (CMD_LEFT | CMD_RIGHT))
	{
		cmd ^= CMD_LEFT | CMD_RIGHT;
	}
	if (cmd & (CMD_UP | CMD_DOWN))
	{
		cmd ^= CMD_UP | CMD_DOWN;
	}
	return cmd;
}

static void Swap(int *x, int *y)
{
	int temp = *x;
	*x = *y;
	*y = temp;
}

// Floating point part of a number
static double FPart(double x)
{
	return x - floor(x);
}

// Reciprocal of the floating point part of a number
static double RFPart(double x)
{
	return 1 - FPart(x);
}

static int IsTileBlocked(int x, int y, double factor)
{
	if (factor > 0.4)
	{
		return gMap[y][x].flags & MAPTILE_IS_WALL;
	}
	return 0;
}

int AIHasClearLine(Vec2i from, Vec2i to)
{
	// Find all tiles that overlap with the line (from, to)
	// Uses a modified version of Xiaolin Wu's algorithm
	Vec2i delta;
	double gradient;
	Vec2i end;
	double xGap;
	Vec2i tileStart, tileEnd;
	double yIntercept;
	int x;
	int w = TILE_WIDTH;
	int h = TILE_HEIGHT;
	int isSteep = abs(to.y - from.y) > abs(to.x - from.x);
	if (isSteep)
	{
		// Swap x and y
		// Note that this prevents the vertical line special case
		Swap(&from.x, &from.y);
		Swap(&to.x, &to.y);
		Swap(&w, &h);
	}
	if (from.x > to.x)
	{
		// swap to make sure we always go left to right
		Swap(&from.x, &to.x);
		Swap(&from.y, &to.y);
	}

	delta.x = to.x - from.x;
	delta.y = to.y - from.y;
	gradient = (double)delta.y / delta.x;

	// handle first endpoint
	end.x = from.x / w;
	end.y = (int)((from.y + gradient * (end.x * w - from.x)) / h);
	xGap = RFPart(from.x + 0.5);
	tileStart.x = end.x;
	tileStart.y = end.y;
	if (isSteep)
	{
		if (IsTileBlocked(tileStart.y, tileStart.x, RFPart(end.y) * xGap))
		{
			return 0;
		}
		if (IsTileBlocked(tileStart.y + 1, tileStart.x, FPart(end.y) * xGap))
		{
			return 0;
		}
	}
	else
	{
		if (IsTileBlocked(tileStart.x, tileStart.y, RFPart(end.y) * xGap))
		{
			return 0;
		}
		if (IsTileBlocked(tileStart.x, tileStart.y + 1, FPart(end.y) * xGap))
		{
			return 0;
		}
	}
	yIntercept = end.y + gradient;

	// handle second endpoint
	end.x = to.x / w;
	end.y = (int)((to.y + gradient * (end.x * w - to.x)) / h);
	xGap = FPart(to.x + 0.5);
	tileEnd.x = end.x;
	tileEnd.y = end.y;
	if (isSteep)
	{
		if (IsTileBlocked(tileEnd.y, tileEnd.x, RFPart(end.y) * xGap))
		{
			return 0;
		}
		if (IsTileBlocked(tileEnd.y + 1, tileEnd.x, FPart(end.y) * xGap))
		{
			return 0;
		}
	}
	else
	{
		if (IsTileBlocked(tileEnd.x, tileEnd.y, RFPart(end.y) * xGap))
		{
			return 0;
		}
		if (IsTileBlocked(tileEnd.x, tileEnd.y + 1, FPart(end.y) * xGap))
		{
			return 0;
		}
	}

	// main loop
	for (x = tileStart.x + 1; x < tileEnd.x; x++)
	{
		if (isSteep)
		{
			if (IsTileBlocked((int)yIntercept, x, RFPart(yIntercept)))
			{
				return 0;
			}
			if (IsTileBlocked((int)yIntercept + 1, x, FPart(yIntercept)))
			{
				return 0;
			}
		}
		else
		{
			if (IsTileBlocked(x, (int)yIntercept, RFPart(yIntercept)))
			{
				return 0;
			}
			if (IsTileBlocked(x, (int)yIntercept + 1, FPart(yIntercept)))
			{
				return 0;
			}
		}
		yIntercept += gradient;
	}

	return 1;
}

TObject *AIGetObjectRunningInto(TActor *a, int cmd)
{
	// Check the position just in front of the character;
	// check if there's a (non-dangerous) object in front of it
	Vec2i frontPos = Vec2iFull2Real(Vec2iNew(a->x, a->y));
	TTileItem *item;
	if (cmd & CMD_LEFT)
	{
		frontPos.x--;
	}
	else if (cmd & CMD_RIGHT)
	{
		frontPos.x++;
	}
	if (cmd & CMD_UP)
	{
		frontPos.y--;
	}
	else if (cmd & CMD_DOWN)
	{
		frontPos.y++;
	}
	item = GetItemOnTileInCollision(
		&a->tileItem,
		frontPos,
		TILEITEM_IMPASSABLE,
		CalcCollisionTeam(1, a),
		gCampaign.Entry.mode == CAMPAIGN_MODE_DOGFIGHT);
	if (!item || item->kind != KIND_OBJECT)
	{
		return NULL;
	}
	return item->data;
}


typedef struct
{
	Vec2i Goal;
	ASPath Path;
	int PathIndex;
	int IsFollowing;
} AIGotoContext;
typedef struct
{
	Tile (*Map)[YMAX][XMAX];
} AStarContext;
static int IsWallOrLockedDoor(Tile(*map)[YMAX][XMAX], Vec2i pos)
{
	int tileFlags = (*map)[pos.y][pos.x].flags;
	if (tileFlags & MAPTILE_IS_WALL)
	{
		return 1;
	}
	else if (tileFlags & MAPTILE_NO_WALK)
	{
		return !!(MapGetDoorKeycardFlag(pos) & ~gMission.flags);
	}
	return 0;
}
static void AddTileNeighbors(
	ASNeighborList neighbors, void *node, void *context)
{
	Vec2i *v = node;
	int y;
	AStarContext *c = context;
	for (y = v->y - 1; y <= v->y + 1; y++)
	{
		int x;
		if (y < 0 || y >= YMAX)
		{
			continue;
		}
		for (x = v->x - 1; x <= v->x + 1; x++)
		{
			float cost = 1;
			Vec2i neighbor;
			neighbor.x = x;
			neighbor.y = y;
			if (x < 0 || x >= XMAX)
			{
				continue;
			}
			if (x == v->x && y == v->y)
			{
				continue;
			}
			if (IsWallOrLockedDoor(c->Map, Vec2iNew(x, y)))
			{
				continue;
			}
			// if we're moving diagonally,
			// need to check the axis-aligned neighbours are also clear
			if (IsWallOrLockedDoor(c->Map, Vec2iNew(v->x, y)) ||
				IsWallOrLockedDoor(c->Map, Vec2iNew(x, v->y)))
			{
				continue;
			}
			// slightly prefer axes instead of diagonals
			if (x != v->x && y != v->y)
			{
				cost = 1.1f;
			}
			ASNeighborListAdd(neighbors, &neighbor, cost);
		}
	}
}
static float AStarHeuristic(void *fromNode, void *toNode, void *context)
{
	// Simple Euclidean
	Vec2i *v1 = fromNode;
	Vec2i *v2 = toNode;
	UNUSED(context);
	return (float)sqrt(DistanceSquared(*v1, *v2));
}
static ASPathNodeSource cPathNodeSource =
{
	sizeof(Vec2i), AddTileNeighbors, AStarHeuristic, NULL, NULL
};
static int AIGotoDirect(Vec2i a, Vec2i p)
{
	int cmd = 0;

	if (a.x < p.x)		cmd |= CMD_RIGHT;
	else if (a.x > p.x)	cmd |= CMD_LEFT;

	if (a.y < p.y)		cmd |= CMD_DOWN;
	else if (a.y > p.y)	cmd |= CMD_UP;

	return cmd;
}
// Follow the current A* path
static int AStarFollow(
	AIGotoContext *c, Vec2i currentTile, TTileItem *i, Vec2i a)
{
	Vec2i *pathTile = ASPathGetNode(c->Path, c->PathIndex);
	c->IsFollowing = 1;
	// Check if we need to follow the next step in the path
	// Note: need to make sure the actor is fully within the current tile
	// otherwise it may get stuck at corners
	if (Vec2iEqual(currentTile, *pathTile) &&
		IsTileItemInsideTile(i, currentTile))
	{
		c->PathIndex++;
		pathTile = ASPathGetNode(c->Path, c->PathIndex);
		c->IsFollowing = 0;
	}
	// Go directly to the center of the next tile
	return AIGotoDirect(a, Vec2iCenterOfTile(*pathTile));
}
int AIGoto(TActor *actor, Vec2i p)
{
	Vec2i a = Vec2iFull2Real(Vec2iNew(actor->x, actor->y));
	Vec2i currentTile = Vec2iToTile(a);
	AIGotoContext *c = actor->aiContext;

	// If we are currently following an A* path, always follow it until
	// we have reached a new tile
	if (c && c->IsFollowing)
	{
		return AStarFollow(c, currentTile, &actor->tileItem, a);
	}
	else if (AIHasClearLine(a, p))
	{
		// Simple case: if there's a clear line between AI and target,
		// walk straight towards it
		return AIGotoDirect(a, p);
	}
	else
	{
		Vec2i goalTile = Vec2iToTile(p);

		// Need to calculate A*
		// First, check if we have a valid path already
		int hasValidPath = 0;
		if (c && c->PathIndex < (int)ASPathGetCount(c->Path) - 1)
		{
			// Check that the path destination is
			// reasonably close to where we want to go
			Vec2i *pathEnd = ASPathGetNode(
				c->Path, ASPathGetCount(c->Path) - 1);
			if (CHEBYSHEV_DISTANCE(
				goalTile.x, goalTile.y, pathEnd->x, pathEnd->y) <= 2)
			{
				hasValidPath = 1;
			}
		}

		if (!hasValidPath)
		{
			// We need to recalculate A*
			AStarContext ac;
			ac.Map = &gMap;
			if (!c)
			{
				CCALLOC(actor->aiContext, sizeof(AIGotoContext));
				c = actor->aiContext;
			}
			c->Goal = goalTile;
			c->PathIndex = 1;	// start navigating to the next path node
			c->Path = ASPathCreate(
				&cPathNodeSource, &ac, &currentTile, &c->Goal);

			// In case we can't calculate A* for some reason,
			// try simple navigation again
			if (ASPathGetCount(c->Path) <= 1)
			{
				debug(
					D_MAX,
					"Error: can't calculate path from {%d, %d} to {%d, %d}",
					currentTile.x, currentTile.y,
					goalTile.x, goalTile.y);
				assert(0);
				return AIGotoDirect(a, p);
			}
		}

		return AStarFollow(c, currentTile, &actor->tileItem, a);
	}
}

int AIHunt(TActor *actor)
{
	int cmd = 0;
	int dx, dy;
	Vec2i targetPos = Vec2iNew(actor->x, actor->y);
	if (!(actor->pData || (actor->flags & FLAGS_GOOD_GUY)))
	{
		targetPos = AIGetClosestPlayerPos(Vec2iNew(actor->x, actor->y));
	}

	if (actor->flags & FLAGS_VISIBLE)
	{
		TActor *a = AIGetClosestEnemy(
			Vec2iNew(actor->x, actor->y), actor->flags, !!actor->pData);
		if (a)
		{
			targetPos.x = a->x;
			targetPos.y = a->y;
		}
	}

	dx = abs(targetPos.x - actor->x);
	dy = abs(targetPos.y - actor->y);

	if (2 * dx > dy)
	{
		if (actor->x < targetPos.x)			cmd |= CMD_RIGHT;
		else if (actor->x > targetPos.x)	cmd |= CMD_LEFT;
	}
	if (2 * dy > dx)
	{
		if (actor->y < targetPos.y)			cmd |= CMD_DOWN;
		else if (actor->y > targetPos.y)	cmd |= CMD_UP;
	}
	// If it's a coward, reverse directions...
	if (actor->flags & FLAGS_RUNS_AWAY)
	{
		cmd = AIReverseDirection(cmd);
	}

	return cmd;
}