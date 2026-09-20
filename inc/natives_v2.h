/*
	Additional Features added via the v2 scripthook: https://www.nexusmods.com/reddeadredemption2/mods/1472?tab=description
*/

#pragma once

namespace GRAPHICS
{
	// Draws a line in world space for one frame. Call it every tick to keep it visible. 
	NATIVE_DECL void DRAW_LINE(float fromX, float fromY, float fromZ,
	                           float toX, float toY, float toZ,
	                           int r, int g, int b, int a)
	{
		invoke<Void>(0xEC2AB68B, fromX, fromY, fromZ, toX, toY, toZ, r, g, b, a);
	}
}

namespace HUD
{
	NATIVE_DECL void SET_TEXT_FONT(int fontType) { invoke<Void>(0x3EDE2F10, fontType); }

	// 0 = Center, 1 = Left, 2 = Right
	NATIVE_DECL void SET_TEXT_JUSTIFICATION(int justifyType) { invoke<Void>(0x1B929974, justifyType); }
}
