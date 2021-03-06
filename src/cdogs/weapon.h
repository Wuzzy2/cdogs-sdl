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

    Copyright (c) 2013-2014, Cong Xu
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
#ifndef __WEAPON
#define __WEAPON

#include "bullet_class.h"
#include "defs.h"
#include "pics.h"
#include "sounds.h"
#include "utils.h"

// WARNING: used for old-style maps, do not touch
typedef enum
{
	GUN_KNIFE,
	GUN_MG,
	GUN_GRENADE,
	GUN_FLAMER,
	GUN_SHOTGUN,
	GUN_POWERGUN,
	GUN_FRAGGRENADE,
	GUN_MOLOTOV,
	GUN_SNIPER,
	GUN_MINE,
	GUN_DYNAMITE,
	GUN_GASBOMB,
	GUN_PETRIFY,
	GUN_BROWN,
	GUN_CONFUSEBOMB,
	GUN_GASGUN,
	GUN_PULSERIFLE,
	GUN_HEATSEEKER,
	GUN_COUNT
} gun_e;

// picture of player holding a gun
typedef enum
{
	GUNPIC_BLASTER,
	GUNPIC_KNIFE,
	GUNPIC_COUNT
} gunpic_e;

// Gun states
typedef enum
{
	GUNSTATE_READY,
	GUNSTATE_FIRING,
	GUNSTATE_RECOIL,
	GUNSTATE_COUNT
} gunstate_e;

typedef struct
{
	gunpic_e pic;
	char *name;
	char *Description;
	const BulletClass *Bullet;
	int Cost;			// Cost in score to fire weapon
	int Lock;
	int ReloadLead;
	Mix_Chunk *Sound;
	Mix_Chunk *ReloadSound;
	int SoundLockLength;
	double Recoil;		// Random recoil for inaccurate weapons, in radians
	struct
	{
		int Count;		// Number of bullets in spread
		double Width;	// Width of individual spread, in radians
	} Spread;
	double AngleOffset;
	int MuzzleHeight;
	int ElevationLow;
	int ElevationHigh;
	const ParticleClass *MuzzleFlash;
	const ParticleClass *Brass;
	bool CanShoot;
	int ShakeAmount;	// Amount of screen shake to produce
	bool IsRealGun;	// whether this gun can be used as is by players
} GunDescription;
typedef struct
{
	CArray Guns;	// of GunDescription
	GunDescription Default;
	CArray CustomGuns;	// of GunDescription
} GunClasses;

typedef struct
{
	const GunDescription *Gun;
	gunstate_e state;
	int lock;
	int soundLock;
	int stateCounter;
} Weapon;

extern GunClasses gGunDescriptions;
extern const TOffsetPic cGunPics[GUNPIC_COUNT][DIRECTION_COUNT][GUNSTATE_COUNT];
extern const OffsetTable cMuzzleOffset[GUNPIC_COUNT];

void WeaponInitialize(GunClasses *g);
void WeaponLoadJSON(GunClasses *g, CArray *classes, json_t *root);
void WeaponClassesClear(CArray *classes);
void WeaponTerminate(GunClasses *g);
Weapon WeaponCreate(const GunDescription *gun);
const GunDescription *StrGunDescription(const char *s);
Vec2i GunGetMuzzleOffset(const GunDescription *desc, const direction_e dir);
void WeaponUpdate(
	Weapon *w, const int ticks, const Vec2i fullPos, const direction_e d);
int WeaponCanFire(Weapon *w);
void WeaponFire(
	Weapon *w, const direction_e d, const Vec2i pos,
	const int flags, const int player, const int uid);
void GunAddBullets(
	const GunDescription *g, const Vec2i fullPos, const int z,
	const double radians,
	const int flags, const int player, const int uid,
	const bool playSound);
void WeaponHoldFire(Weapon *w);

int GunGetRange(const GunDescription *g);
bool IsHighDPS(const GunDescription *g);
bool IsLongRange(const GunDescription *g);
bool IsShortRange(const GunDescription *g);

// Initialise bullets and weapons in one go
void BulletAndWeaponInitialize(
	BulletClasses *b, GunClasses *g, const char *bpath, const char *gpath);

#endif
