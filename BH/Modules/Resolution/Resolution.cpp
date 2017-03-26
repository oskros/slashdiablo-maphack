#include "Resolution.h"
#include "../../D2Ptrs.h"
#include "../../BH.h"
#include "../../D2Stubs.h"
#include "../../D2Helpers.h"
#include "ScreenRefresh.h"

using namespace std;

void Resolution::OnLoad() {
	isInGame = false;
	newWidth = BH::config->ReadInt("New Width", 1344);
	newHeight = BH::config->ReadInt("New Height", 700);
}

void Resolution::OnUnload() {
}

int Resolution::GetMode(int height) {
	switch(height)
	{
		case 800:
			return 2;
		case 640:
			return 1;
	}
	return 3;
}

//Shaggi's resolution fix, all credits to him
void Resolution::SetResolution(int x, int y) {
	//GetMode(*p_D2CLIENT_ScreenSizeY);
	int mode = GetMode(y);
	*p_D2CLIENT_CropMode = 0;
	D2WIN_SetResolutionMode(mode);
	*p_D2CLIENT_ScreenSizeX = *p_D2CLIENT_MapPosX = *p_D2CLIENT_SizeX1 = *p_D2CLIENT_SizeX2 = x;
	*p_D2CLIENT_ScreenSizeY = *p_D2CLIENT_SizeY1 = y;

	*p_D2CLIENT_MapPosY = y - 40; // subtract 40 to correct offsets
	ScreenRefresh::RefreshDisplay();
	//raise resolution changed event so that other modules can readjust positions
	__raise BH::moduleManager->OnResolutionChanged(x, y);
}

void Resolution::OnDraw() {
	// Credits to /u/TravHatesMe aka blurt for sharing this code.
	// Fixes the Hall of Mirrors effect on the bottom UI.
	if (isInGame && ((*BH::MiscToggles)["Toggle Resolution"].state)) {
		const int UIBoxHalfWidth = 155;
		unsigned int box1Width = (newWidth / 2) - (UIBoxHalfWidth + 165);

		Drawing::Boxhook::Draw(165, 653, box1Width, 50, 0, Drawing::BoxTrans::BTOneFourth);
		Drawing::Boxhook::Draw(827, 653, box1Width, 50, 0, Drawing::BoxTrans::BTOneFourth);
	}
}

void Resolution::OnKey(bool up, BYTE key, LPARAM lParam, bool* block) {
	if (!isInGame)
		return;
	if (key == ((*BH::MiscToggles)["Toggle Resolution"].toggle)) {
		*block = true;
		if (up) {
			bool isLoaded = (*BH::MiscToggles)["Toggle Resolution"].state;
			if (!isLoaded) {
				this->SetResolution(newWidth, newHeight);
				(*BH::MiscToggles)["Toggle Resolution"].state = true;
			}
		}
	}
}

void Resolution::OnGameJoin(const string& name, const string& pass, int diff) {
	isInGame = true;

	//if the user has already toggled it from a previous game, autoload it
	if (((*BH::MiscToggles)["Toggle Resolution"].state)
		&& newWidth > 0 && newHeight > 0) {
		this->SetResolution(newWidth, newHeight);
		ScreenRefresh::RefreshDisplay();
	}
}

void Resolution::OnGameExit() {
	isInGame = false;
	//raise resolution changed event so that other modules can readjust positions
	if ((*BH::MiscToggles)["Toggle Resolution"].state) {
		__raise BH::moduleManager->OnResolutionChanged(800, 600);
	}
}
