/*
    C-Dogs SDL
    A port of the legendary (and fun) action/arcade cdogs.

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
#ifndef __CONFIG
#define __CONFIG

#include "grafx.h"
#include "input.h"
#include "sounds.h"

#define CONFIG_FILE "options.cnf"

typedef enum
{
	ALLYCOLLISION_NORMAL,
	ALLYCOLLISION_REPEL,
	ALLYCOLLISION_NONE
} AllyCollision;

const char *AllyCollisionStr(AllyCollision a);
AllyCollision StrAllyCollision(const char *str);

typedef enum
{
	DIFFICULTY_VERYEASY = 1,
	DIFFICULTY_EASY,
	DIFFICULTY_NORMAL,
	DIFFICULTY_HARD,
	DIFFICULTY_VERYHARD
} difficulty_e;

const char *DifficultyStr(difficulty_e d);
difficulty_e StrDifficulty(const char *str);

typedef enum
{
	FIREMOVE_STOP = 0,
	FIREMOVE_NORMAL,
	FIREMOVE_STRAFE
} FireMoveStyle;
const char *FireMoveStyleStr(FireMoveStyle s);
FireMoveStyle StrFireMoveStyle(const char *s);

typedef enum
{
	SWITCHMOVE_SLIDE = 0,
	SWITCHMOVE_STRAFE,
	SWITCHMOVE_NONE
} SwitchMoveStyle;
const char *SwitchMoveStyleStr(SwitchMoveStyle s);
SwitchMoveStyle StrSwitchMoveStyle(const char *s);

const char *ScaleModeStr(ScaleMode q);
ScaleMode StrScaleMode(const char *str);

typedef enum
{
	GORE_NONE,
	GORE_LOW,
	GORE_MEDIUM,
	GORE_HIGH
} GoreAmount;
const char *GoreAmountStr(GoreAmount g);
GoreAmount StrGoreAmount(const char *s);

typedef enum
{
	LASER_SIGHT_NONE,
	LASER_SIGHT_PLAYERS,
	LASER_SIGHT_ALL
} LaserSight;
const char *LaserSightStr(LaserSight l);
LaserSight StrLaserSight(const char *s);

typedef struct
{
	bool FriendlyFire;
	unsigned int RandomSeed;
	difficulty_e Difficulty;
	bool SlowMotion;
	int EnemyDensity;
	int NonPlayerHP;
	int PlayerHP;
	int Lives;
	bool Fog;
	int SightRange;
	bool Shadows;
	FireMoveStyle FireMoveStyle;
	SwitchMoveStyle SwitchMoveStyle;
	bool ShotsPushback;
	AllyCollision AllyCollision;
	bool HealthPickups;
	GoreAmount Gore;
	LaserSight LaserSight;
} GameConfig;

typedef enum
{
	SPLITSCREEN_NORMAL,
	SPLITSCREEN_ALWAYS,
	SPLITSCREEN_NEVER
} SplitscreenStyle;

const char *SplitscreenStyleStr(SplitscreenStyle s);
SplitscreenStyle StrSplitscreenStyle(const char *str);

typedef enum
{
	AICHATTER_NONE,
	AICHATTER_SELDOM,
	AICHATTER_OFTEN,
	AICHATTER_ALWAYS
} AIChatterFrequency;
const char *AIChatterStr(AIChatterFrequency c);
AIChatterFrequency StrAIChatter(const char *str);

typedef struct
{
	bool ShowFPS;
	bool ShowTime;
	SplitscreenStyle Splitscreen;
	bool ShowHUDMap;
	AIChatterFrequency AIChatter;
} InterfaceConfig;

typedef enum
{
	QUICKPLAY_QUANTITY_ANY,
	QUICKPLAY_QUANTITY_SMALL,
	QUICKPLAY_QUANTITY_MEDIUM,
	QUICKPLAY_QUANTITY_LARGE
} QuickPlayQuantity;

const char *QuickPlayQuantityStr(QuickPlayQuantity s);
QuickPlayQuantity StrQuickPlayQuantity(const char *str);

typedef struct
{
	QuickPlayQuantity MapSize;
	QuickPlayQuantity WallCount;
	QuickPlayQuantity WallLength;
	QuickPlayQuantity RoomCount;
	QuickPlayQuantity SquareCount;
	QuickPlayQuantity EnemyCount;
	QuickPlayQuantity EnemySpeed;
	QuickPlayQuantity EnemyHealth;
	bool EnemiesWithExplosives;
	QuickPlayQuantity ItemCount;
} QuickPlayConfig;

typedef struct
{
	GameConfig Game;
	GraphicsConfig Graphics;
	InputConfig Input;
	InterfaceConfig Interface;
	SoundConfig Sound;
	QuickPlayConfig QuickPlay;
} Config;

extern Config gConfig;
extern Config gLastConfig;

void ConfigLoad(Config *config, const char *filename);
void ConfigSave(Config *config, const char *filename);
int ConfigApply(Config *config);
void ConfigLoadDefault(Config *config);
int ConfigGetVersion(FILE *f);
int ConfigIsMouseUsed(InputConfig *config);

#endif
