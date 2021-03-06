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
#include "player_select_menus.h"

#include <assert.h>
#include <stddef.h>

#include <cdogs/actors.h>	// for shades
#include <cdogs/draw.h>
#include <cdogs/font.h>
#include <cdogs/player_template.h>


static char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ !#?:.-0123456789";
static char smallLetters[] = "abcdefghijklmnopqrstuvwxyz !#?:.-0123456789";


static const char *shadeNames[] = {
	"Blue",
	"Skin",
	"Brown",
	"Green",
	"Yellow",
	"Purple",
	"Red",
	"Light Gray",
	"Gray",
	"Dark Gray",
	"Asian",
	"Dark Skin",
	"Black",
	"Golden"
};
const char *IndexToShadeStr(int idx)
{
	if (idx >= 0 && idx < SHADE_COUNT)
	{
		return shadeNames[idx];
	}
	return shadeNames[0];
}


static void DrawNameMenu(
	const menu_t *menu, GraphicsDevice *g,
	const Vec2i pos, const Vec2i size, const void *data)
{
	const PlayerSelectMenuData *d = data;

#define ENTRY_COLS	8
#define	ENTRY_SPACING	7

	int x = pos.x;
	int y = CENTER_Y(
		pos, size, FontH() * ((strlen(letters) - 1) / ENTRY_COLS));

	UNUSED(menu);
	UNUSED(g);

	int i;
	for (i = 0; i < (int)strlen(letters); i++)
	{
		Vec2i menuPos = Vec2iNew(
			x + (i % ENTRY_COLS) * ENTRY_SPACING,
			y + (i / ENTRY_COLS) * FontH());
		FontChMask(
			letters[i], menuPos,
			i == d->nameMenuSelection ? colorRed : colorWhite);
	}

	DisplayMenuItem(
		Vec2iNew(
			x + (i % ENTRY_COLS) * ENTRY_SPACING,
			y + (i / ENTRY_COLS) * FontH()),
		"(End)", i == d->nameMenuSelection, 0, colorBlack);
}

static int HandleInputNameMenu(int cmd, void *data)
{
	PlayerSelectMenuData *d = data;
	PlayerData *p = CArrayGet(&gPlayerDatas, d->display.playerIndex);

	if (cmd & CMD_BUTTON1)
	{
		if (d->nameMenuSelection == (int)strlen(letters))
		{
			MenuPlaySound(MENU_SOUND_ENTER);
			return 1;
		}

		if (strlen(p->name) < sizeof p->name - 1)
		{
			size_t l = strlen(p->name);
			p->name[l + 1] = 0;
			if (l > 0 && p->name[l - 1] != ' ')
			{
				p->name[l] = smallLetters[d->nameMenuSelection];
			}
			else
			{
				p->name[l] = letters[d->nameMenuSelection];
			}
			MenuPlaySound(MENU_SOUND_ENTER);
		}
		else
		{
			MenuPlaySound(MENU_SOUND_ERROR);
		}
	}
	else if (cmd & CMD_BUTTON2)
	{
		if (p->name[0])
		{
			p->name[strlen(p->name) - 1] = 0;
			MenuPlaySound(MENU_SOUND_BACK);
		}
		else
		{
			MenuPlaySound(MENU_SOUND_ERROR);
		}
	}
	else if (cmd & CMD_LEFT)
	{
		if (d->nameMenuSelection > 0)
		{
			d->nameMenuSelection--;
			MenuPlaySound(MENU_SOUND_SWITCH);
		}
	}
	else if (cmd & CMD_RIGHT)
	{
		if (d->nameMenuSelection < (int)strlen(letters))
		{
			d->nameMenuSelection++;
			MenuPlaySound(MENU_SOUND_SWITCH);
		}
	}
	else if (cmd & CMD_UP)
	{
		if (d->nameMenuSelection >= ENTRY_COLS)
		{
			d->nameMenuSelection -= ENTRY_COLS;
			MenuPlaySound(MENU_SOUND_SWITCH);
		}
	}
	else if (cmd & CMD_DOWN)
	{
		if (d->nameMenuSelection <= (int)strlen(letters) - ENTRY_COLS)
		{
			d->nameMenuSelection += ENTRY_COLS;
			MenuPlaySound(MENU_SOUND_SWITCH);
		}
		else if (d->nameMenuSelection < (int)strlen(letters))
		{
			d->nameMenuSelection = (int)strlen(letters);
			MenuPlaySound(MENU_SOUND_SWITCH);
		}
	}

	return 0;
}

static void PostInputAppearanceMenu(menu_t *menu, int cmd, void *data)
{
	AppearanceMenuData *d = data;
	UNUSED(cmd);
	PlayerData *p = CArrayGet(&gPlayerDatas, d->playerIndex);
	Character *c = &p->Char;
	int *prop = (int *)((char *)&c->looks + d->propertyOffset);
	*prop = menu->u.normal.index;
	CharacterSetColors(c);
}

static menu_t *CreateAppearanceMenu(
	const char *name, AppearanceMenuData *data)
{
	menu_t *menu = MenuCreateNormal(name, "", MENU_TYPE_NORMAL, 0);
	int i;
	menu->u.normal.maxItems = 11;
	for (i = 0; i < data->menuCount; i++)
	{
		MenuAddSubmenu(menu, MenuCreateBack(data->strFunc(i)));
	}
	MenuSetPostInputFunc(menu, PostInputAppearanceMenu, data);
	return menu;
}

static void PostInputLoadTemplate(menu_t *menu, int cmd, void *data)
{
	if (cmd & CMD_BUTTON1)
	{
		PlayerSelectMenuData *d = data;
		PlayerData *p = CArrayGet(&gPlayerDatas, d->display.playerIndex);
		const PlayerTemplate *t =
			CArrayGet(&gPlayerTemplates, menu->u.normal.index);
		memset(p->name, 0, sizeof p->name);
		strncpy(p->name, t->name, sizeof p->name - 1);
		p->Char.looks = t->Looks;
		CharacterSetColors(&p->Char);
	}
}

// Load all the template names to the menu entries
static void PostEnterLoadTemplateNames(menu_t *menu, void *data)
{
	bool *isSave = (bool *)data;
	for (int i = 0; i < (int)gPlayerTemplates.size; i++)
	{
		// Add menu if necessary
		if (i == (int)menu->u.normal.subMenus.size)
		{
			MenuAddSubmenu(menu, MenuCreateBack(""));
		}
		menu_t *subMenu = CArrayGet(&menu->u.normal.subMenus, i);
		const PlayerTemplate *pt = CArrayGet(&gPlayerTemplates, i);
		strcpy(subMenu->name, pt->name);
	}
	if (*isSave && menu->u.normal.subMenus.size == gPlayerTemplates.size)
	{
		MenuAddSubmenu(menu, MenuCreateBack("(new)"));
	}
}

static menu_t *CreateUseTemplateMenu(
	const char *name, PlayerSelectMenuData *data)
{
	menu_t *menu = MenuCreateNormal(name, "", MENU_TYPE_NORMAL, 0);
	menu->u.normal.maxItems = 11;
	MenuSetPostEnterFunc(menu, PostEnterLoadTemplateNames, &gFalse);
	MenuSetPostInputFunc(menu, PostInputLoadTemplate, data);
	return menu;
}

static void PostInputSaveTemplate(menu_t *menu, int cmd, void *data)
{
	if (cmd & CMD_BUTTON1)
	{
		PlayerSelectMenuData *d = data;
		PlayerData *p = CArrayGet(&gPlayerDatas, d->display.playerIndex);
		PlayerTemplate *t =
			CArrayGet(&gPlayerTemplates, menu->u.normal.index);
		memset(t->name, 0, sizeof t->name);
		strncpy(t->name, p->name, sizeof t->name - 1);
		t->Looks = p->Char.looks;
	}
}

static void SaveTemplateDisplayTitle(
	const menu_t *menu, GraphicsDevice *g,
	const Vec2i pos, const Vec2i size, const void *data)
{
	UNUSED(g);
	const PlayerSelectMenuData *d = data;
	char buf[256];

	UNUSED(menu);
	UNUSED(size);

	// Display "Save <template>..." title
	const PlayerData *p = CArrayGet(&gPlayerDatas, d->display.playerIndex);
	sprintf(buf, "Save %s...", p->name);
	FontStr(buf, Vec2iAdd(pos, Vec2iNew(0, 0)));
}

static menu_t *CreateSaveTemplateMenu(
	const char *name, PlayerSelectMenuData *data)
{
	menu_t *menu = MenuCreateNormal(name, "", MENU_TYPE_NORMAL, 0);
	menu->u.normal.maxItems = 11;
	MenuSetPostEnterFunc(menu, PostEnterLoadTemplateNames, &gTrue);
	MenuSetPostInputFunc(menu, PostInputSaveTemplate, data);
	MenuSetCustomDisplay(menu, SaveTemplateDisplayTitle, data);
	return menu;
}

static void CheckReenableLoadMenu(menu_t *menu, void *data)
{
	menu_t *loadMenu = MenuGetSubmenuByName(menu, "Load");
	UNUSED(data);
	assert(loadMenu);
	loadMenu->isDisabled = gPlayerTemplates.size == 0;
}
static menu_t *CreateCustomizeMenu(
	const char *name, PlayerSelectMenuData *data, const int playerIndex);
static void ShuffleAppearance(void *data);
void PlayerSelectMenusCreate(
	PlayerSelectMenu *menu,
	int numPlayers, int player, const int playerIndex,
	EventHandlers *handlers, GraphicsDevice *graphics,
	InputConfig *inputConfig, const NameGen *ng)
{
	MenuSystem *ms = &menu->ms;
	PlayerSelectMenuData *data = &menu->data;
	Vec2i pos, size;
	int w = graphics->cachedConfig.Res.x;
	int h = graphics->cachedConfig.Res.y;

	data->nameMenuSelection = (int)strlen(letters);
	data->display.playerIndex = playerIndex;
	data->display.currentMenu = &ms->current;
	data->controls.inputConfig = inputConfig;
	data->controls.playerIndex = playerIndex;
	data->nameGenerator = ng;

	switch (numPlayers)
	{
	case 1:
		// Single menu, entire screen
		pos = Vec2iNew(w / 2, 0);
		size = Vec2iNew(w / 2, h);
		break;
	case 2:
		// Two menus, side by side
		pos = Vec2iNew(player * w / 2 + w / 4, 0);
		size = Vec2iNew(w / 4, h);
		break;
	case 3:
	case 4:
		// Four corners
		pos = Vec2iNew((player & 1) * w / 2 + w / 4, (player / 2) * h / 2);
		size = Vec2iNew(w / 4, h / 2);
		break;
	default:
		CASSERT(false, "not implemented");
		pos = Vec2iNew(w / 2, 0);
		size = Vec2iNew(w / 2, h);
		break;
	}
	MenuSystemInit(ms, handlers, graphics, pos, size);
	ms->align = MENU_ALIGN_LEFT;
	ms->root = ms->current = MenuCreateNormal(
		"",
		"",
		MENU_TYPE_NORMAL,
		0);
	MenuAddSubmenu(
		ms->root,
		MenuCreateCustom(
		"Name", DrawNameMenu, HandleInputNameMenu, data));

	MenuAddSubmenu(
		ms->root, CreateCustomizeMenu("Customize...", data, playerIndex));
	MenuAddSubmenu(
		ms->root,
		MenuCreateVoidFunc("Shuffle", ShuffleAppearance, data));

	MenuAddSubmenu(ms->root, CreateUseTemplateMenu("Load", data));
	MenuAddSubmenu(ms->root, CreateSaveTemplateMenu("Save", data));

	MenuAddSubmenu(ms->root, MenuCreateSeparator(""));
	MenuAddSubmenu(
		ms->root, MenuCreateNormal("Done", "", MENU_TYPE_NORMAL, 0));
	MenuAddExitType(ms, MENU_TYPE_RETURN);
	MenuSystemAddCustomDisplay(ms, MenuDisplayPlayer, data);
	MenuSystemAddCustomDisplay(ms, MenuDisplayPlayerControls, &data->controls);

	// Detect when there have been new player templates created,
	// to re-enable the load menu
	CheckReenableLoadMenu(ms->root, NULL);
	MenuSetPostEnterFunc(ms->root, CheckReenableLoadMenu, NULL);

	PlayerData *p = CArrayGet(&gPlayerDatas, playerIndex);
	CharacterSetColors(&p->Char);
}
static menu_t *CreateCustomizeMenu(
	const char *name, PlayerSelectMenuData *data, const int playerIndex)
{
	menu_t *menu = MenuCreateNormal(name, "", MENU_TYPE_NORMAL, 0);

	data->faceData.playerIndex = playerIndex;
	data->faceData.menuCount = FACE_COUNT;
	data->faceData.strFunc = IndexToFaceStr;
	data->faceData.propertyOffset = offsetof(CharLooks, face);
	MenuAddSubmenu(menu, CreateAppearanceMenu("Face", &data->faceData));

	data->skinData.playerIndex = playerIndex;
	data->skinData.menuCount = SHADE_COUNT;
	data->skinData.strFunc = IndexToShadeStr;
	data->skinData.propertyOffset = offsetof(CharLooks, skin);
	MenuAddSubmenu(menu, CreateAppearanceMenu("Skin", &data->skinData));

	data->hairData.playerIndex = playerIndex;
	data->hairData.menuCount = SHADE_COUNT;
	data->hairData.strFunc = IndexToShadeStr;
	data->hairData.propertyOffset = offsetof(CharLooks, hair);
	MenuAddSubmenu(menu, CreateAppearanceMenu("Hair", &data->hairData));

	data->armsData.playerIndex = playerIndex;
	data->armsData.menuCount = SHADE_COUNT;
	data->armsData.strFunc = IndexToShadeStr;
	data->armsData.propertyOffset = offsetof(CharLooks, arm);
	MenuAddSubmenu(menu, CreateAppearanceMenu("Arms", &data->armsData));

	data->bodyData.playerIndex = playerIndex;
	data->bodyData.menuCount = SHADE_COUNT;
	data->bodyData.strFunc = IndexToShadeStr;
	data->bodyData.propertyOffset = offsetof(CharLooks, body);
	MenuAddSubmenu(menu, CreateAppearanceMenu("Body", &data->bodyData));

	data->legsData.playerIndex = playerIndex;
	data->legsData.menuCount = SHADE_COUNT;
	data->legsData.strFunc = IndexToShadeStr;
	data->legsData.propertyOffset = offsetof(CharLooks, leg);
	MenuAddSubmenu(menu, CreateAppearanceMenu("Legs", &data->legsData));

	MenuAddSubmenu(menu, MenuCreateSeparator(""));
	MenuAddSubmenu(menu, MenuCreateBack("Back"));

	return menu;
}
static void ShuffleOne(AppearanceMenuData *data);
static void ShuffleAppearance(void *data)
{
	PlayerSelectMenuData *pData = data;
	char buf[512];
	NameGenMake(pData->nameGenerator, buf);
	PlayerData *p = CArrayGet(&gPlayerDatas, pData->display.playerIndex);
	strncpy(p->name, buf, 20);
	ShuffleOne(&pData->faceData);
	ShuffleOne(&pData->skinData);
	ShuffleOne(&pData->hairData);
	ShuffleOne(&pData->armsData);
	ShuffleOne(&pData->bodyData);
	ShuffleOne(&pData->legsData);
}
static void ShuffleOne(AppearanceMenuData *data)
{
	PlayerData *p = CArrayGet(&gPlayerDatas, data->playerIndex);
	Character *c = &p->Char;
	int *prop = (int *)((char *)&c->looks + data->propertyOffset);
	*prop = rand() % data->menuCount;
	CharacterSetColors(c);
}