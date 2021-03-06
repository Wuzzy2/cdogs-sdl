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
#include "pickup.h"

#include "game_events.h"
#include "map.h"


CArray gPickups;


void PickupsInit(void)
{
	CArrayInit(&gPickups, sizeof(Pickup));
	CArrayReserve(&gPickups, 128);
}
void PickupsTerminate(void)
{
	for (int i = 0; i < (int)gPickups.size; i++)
	{
		const Pickup *p = CArrayGet(&gPickups, i);
		if (p->isInUse)
		{
			PickupDestroy(i);
		}
	}
	CArrayTerminate(&gPickups);
}
static const Pic *GetPickupPic(const int id, Vec2i *offset);
int PickupAdd(
	const Vec2i pos, const char *picName, const int oldIdx,
	const PickupType type)
{
	// Find an empty slot in pickup list
	Pickup *p = NULL;
	int i;
	for (i = 0; i < (int)gPickups.size; i++)
	{
		Pickup *pu = CArrayGet(&gPickups, i);
		if (!pu->isInUse)
		{
			p = pu;
			break;
		}
	}
	if (p == NULL)
	{
		Pickup pu;
		memset(&pu, 0, sizeof pu);
		CArrayPushBack(&gPickups, &pu);
		i = (int)gPickups.size - 1;
		p = CArrayGet(&gPickups, i);
	}
	memset(p, 0, sizeof *p);
	p->Pic = PicManagerGet(&gPicManager, picName, oldIdx);
	p->Type = type;
	p->tileItem.x = p->tileItem.y = -1;
	p->tileItem.flags = 0;
	p->tileItem.kind = KIND_PICKUP;
	p->tileItem.getPicFunc = GetPickupPic;
	p->tileItem.getActorPicsFunc = NULL;
	p->tileItem.w = p->Pic->size.x;
	p->tileItem.h = p->Pic->size.y;
	p->tileItem.id = i;
	MapTryMoveTileItem(&gMap, &p->tileItem, Vec2iFull2Real(pos));
	p->isInUse = true;
	return i;
}
void PickupDestroy(const int id)
{
	Pickup *p = CArrayGet(&gPickups, id);
	CASSERT(p->isInUse, "Destroying not-in-use pickup");
	MapRemoveTileItem(&gMap, &p->tileItem);
	p->isInUse = false;
}

void PickupPickup(const TActor *a, const Pickup *p)
{
	bool canPickup = true;
	Mix_Chunk *sound = NULL;
	const Vec2i actorPos = Vec2iNew(a->tileItem.x, a->tileItem.y);
	switch (p->Type)
	{
	case PICKUP_JEWEL:
		{
			GameEvent e = GameEventNew(GAME_EVENT_SCORE);
			e.u.Score.PlayerIndex = a->playerIndex;
			e.u.Score.Score = p->u.Score;
			GameEventsEnqueue(&gGameEvents, e);
			sound = gSoundDevice.pickupSound;
		}
		break;

	case PICKUP_HEALTH:
		// Don't pick up unless taken damage
		canPickup = false;
		if (a->health < ActorGetCharacter(a)->maxHealth)
		{
			canPickup = true;
			GameEvent e = GameEventNew(GAME_EVENT_TAKE_HEALTH_PICKUP);
			e.u.Heal.PlayerIndex = a->playerIndex;
			e.u.Heal.Health = p->u.Health;
			GameEventsEnqueue(&gGameEvents, e);
			sound = gSoundDevice.healthSound;
		}
		break;

	case PICKUP_KEYCARD:
		gMission.flags |= p->u.Keys;
		sound = gSoundDevice.keySound;
		// Clear cache since we may now have new paths
		PathCacheClear(&gPathCache);
		break;

	default:
		CASSERT(false, "unexpected objective");
		break;
	}
	if (canPickup)
	{
		SoundPlayAt(&gSoundDevice, sound, actorPos);
		UpdateMissionObjective(
			&gMission, p->tileItem.flags, OBJECTIVE_COLLECT,
			a->playerIndex, actorPos);
		PickupDestroy(p->tileItem.id);
	}
}

static const Pic *GetPickupPic(const int id, Vec2i *offset)
{
	const Pickup *p = CArrayGet(&gPickups, id);
	*offset = Vec2iScaleDiv(p->Pic->size, -2);
	return p->Pic;
}
