/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.
    Copyright (c) 2014, Cong Xu
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
#pragma once

#include "actors.h"
#include "pic.h"
#include "tile.h"

// Effects for "pick up" objects
typedef enum
{
	PICKUP_NONE,
	PICKUP_JEWEL,
	PICKUP_HEALTH,
	PICKUP_KEYCARD
} PickupType;

// Pickups are game objects that players can collect, and which have special
// effects
typedef struct
{
	const Pic *Pic;
	PickupType Type;
	union
	{
		int Score;
		int Health;
		int Keys;	// Refer to flags in mission.h
	} u;
	TTileItem tileItem;
	bool isInUse;
} Pickup;

extern CArray gPickups;	// of Pickup

// Score for picking up an objective
#define PICKUP_SCORE 10


void PickupsInit(void);
void PickupsTerminate(void);
int PickupAdd(
	const Vec2i pos, const char *picName, const int oldIdx,
	const PickupType type);
void PickupDestroy(const int id);

void PickupPickup(const TActor *a, const Pickup *p);
