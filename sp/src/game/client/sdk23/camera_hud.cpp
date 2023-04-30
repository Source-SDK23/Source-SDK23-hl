#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "c_basehlplayer.h" //alternative #include "c_baseplayer.h"

#include <vgui/IScheme.h>
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>

#include "camera_hud.h"

// memdbgon must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"

DECLARE_HUDELEMENT(CCameraViewfinder);
DECLARE_HUD_MESSAGE(CCameraViewfinder, ShowCameraViewfinder);

using namespace vgui;

/**
 * Constructor - generic HUD element initialization stuff. Make sure our 2 member variables
 * are instantiated.
 */
CCameraViewfinder::CCameraViewfinder(const char* pElementName) : CHudElement(pElementName), BaseClass(NULL, "CameraViewfinder")
{
	vgui::Panel* pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_bShow = false;
	m_pViewfinder_ul = 0;
	m_pViewfinder_halfcircle = 0;

	// Scope will not show when the player is dead
	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	// fix for users with diffrent screen ratio (Lodle)
	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);
}

/**
 * Hook up our HUD message, and make sure we are not showing the scope
 */
void CCameraViewfinder::Init()
{
	HOOK_HUD_MESSAGE(CCameraViewfinder, ShowCameraViewfinder);

	m_bShow = false;
}

/**
 * Load  in the scope material here
 */
void CCameraViewfinder::ApplySchemeSettings(vgui::IScheme* scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	if (!m_pViewfinder_ul)
	{
		m_pViewfinder_ul = gHUD.GetIcon("camera_viewfinder_ul");
		m_pViewfinder_halfcircle = gHUD.GetIcon("camera_viewfinder_halfcircle");
	}
}

/**
 * Simple - if we want to show the scope, draw it. Otherwise don't.
 */
void CCameraViewfinder::Paint(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	if (m_bShow)
	{
		int width = GetWide(); // Width of player's screen resolution.
		int height = GetTall(); // Height of player's screen resolution.

		int blockWidth = width / 2;
		int blockHeight = height / 2; // Our HUD blocks will each be half the height of the screen.

		// Source engine displays UI elements with an inverted-y axis.
		// This means that the point (0,0) is located in the upper left corner of the screen and is why our lower scope blocks must be offset by y = h / 2.

		vgui::surface()->DrawSetColor(Color(0, 0, 0, 255));
		for (int i = 0; i < 3; i++) { // For loop for thickness of lines
			// Draw circles
			vgui::surface()->DrawOutlinedCircle(blockWidth, blockHeight, (blockWidth / 5) + i, 128);
			vgui::surface()->DrawOutlinedCircle(blockWidth, blockHeight, (blockWidth / 4) + i, 128);

			// Draw corners
			vgui::surface()->DrawLine(blockWidth / 4+i, blockHeight / 4, blockWidth / 4+i, blockHeight / 2); // UL
			vgui::surface()->DrawLine(blockWidth / 4, blockHeight / 4+i, blockWidth / 4 + (blockHeight / 4), blockHeight / 4+i);

			vgui::surface()->DrawLine(width - blockWidth / 4 - i, blockHeight / 4, width - blockWidth / 4 - i, blockHeight / 2); // UR
			vgui::surface()->DrawLine(width - blockWidth / 4, blockHeight / 4+i, width - blockWidth / 4 - (blockHeight / 4), blockHeight / 4+i);

			vgui::surface()->DrawLine(blockWidth / 4+i, height - blockHeight / 4, blockWidth / 4+i, height - blockHeight / 2); // LL
			vgui::surface()->DrawLine(blockWidth / 4, height - blockHeight / 4-i, blockWidth / 4 + (blockHeight / 4), height - blockHeight / 4-i);

			vgui::surface()->DrawLine(width - blockWidth / 4-i, height - blockHeight / 4, width - blockWidth / 4 - i, height - blockHeight / 2); // LR
			vgui::surface()->DrawLine(width - blockWidth / 4, height - blockHeight / 4-i, width - blockWidth / 4 - (blockHeight / 4), height - blockHeight / 4 - i);
		}


		/////
		// Calculate UV Stuff (m_pViewfinder_halfcircle)
		int textureId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(textureId, m_pViewfinder_halfcircle->szTextureFile, false, false);

		float texCoords[4];
		int wide, tall;
		vgui::surface()->DrawGetTextureSize(m_pViewfinder_halfcircle->textureId, wide, tall);
		texCoords[0] = (float)(m_pViewfinder_halfcircle->rc.left + 0.5f) / (float)wide;
		texCoords[1] = (float)(m_pViewfinder_halfcircle->rc.top + 0.5f) / (float)tall;
		texCoords[2] = (float)(m_pViewfinder_halfcircle->rc.right - 0.5f) / (float)wide;
		texCoords[3] = (float)(m_pViewfinder_halfcircle->rc.bottom - 0.5f) / (float)tall;

		// Draw main UI (has to be flipped with weird UV stuff)
		vgui::surface()->DrawSetTexture(textureId);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));
		int x = blockWidth - (m_pViewfinder_halfcircle->Width() / 2) / 2;
		int y = blockHeight;
		vgui::surface()->DrawTexturedSubRect(x, y, x+m_pViewfinder_halfcircle->Width()/2, y+m_pViewfinder_halfcircle->Height()/2,
			texCoords[0], texCoords[3], texCoords[2], texCoords[1]);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, 64)); // Top half of circle, more opacity
		y = blockHeight - m_pViewfinder_halfcircle->Height() / 2; // Update height for top half
		vgui::surface()->DrawTexturedSubRect(x, y, x + m_pViewfinder_halfcircle->Width()/2, y + m_pViewfinder_halfcircle->Height()/2,
			texCoords[0], texCoords[1], texCoords[2], texCoords[3]);

		/////
		// Calculate UV Stuff (m_pViewfinder_ul)
		textureId = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(textureId, m_pViewfinder_ul->szTextureFile, false, false);

		vgui::surface()->DrawGetTextureSize(m_pViewfinder_ul->textureId, wide, tall);
		texCoords[0] = (float)(m_pViewfinder_ul->rc.left + 0.5f) / (float)wide;
		texCoords[1] = (float)(m_pViewfinder_ul->rc.top + 0.5f) / (float)tall;
		texCoords[2] = (float)(m_pViewfinder_ul->rc.right - 0.5f) / (float)wide;
		texCoords[3] = (float)(m_pViewfinder_ul->rc.bottom - 0.5f) / (float)tall;

		// Draw main UI (has to be flipped with weird UV stuff)
		vgui::surface()->DrawSetTexture(textureId);
		vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));
		vgui::surface()->DrawTexturedSubRect(0, 0, blockWidth, blockHeight,
			texCoords[0], texCoords[1], texCoords[2], texCoords[3]);
		vgui::surface()->DrawTexturedSubRect(blockWidth, 0, blockWidth*2, blockHeight,
			texCoords[2], texCoords[1], texCoords[0], texCoords[3]);
		vgui::surface()->DrawTexturedSubRect(0, blockHeight, blockWidth, blockHeight*2,
			texCoords[0], texCoords[3], texCoords[2], texCoords[1]);
		vgui::surface()->DrawTexturedSubRect(blockWidth, blockHeight, blockWidth*2, blockHeight*2,
		texCoords[2], texCoords[3], texCoords[0], texCoords[1]);


		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR; // This hides the crosshair while the scope is active.
	}
	else if ((pPlayer->m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) != 0)
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	}
}


/**
 * Callback for our message - set the show variable to whatever
 * boolean value is received in the message
 */
void CCameraViewfinder::MsgFunc_ShowCameraViewfinder(bf_read& msg)
{
	m_bShow = msg.ReadByte();
}