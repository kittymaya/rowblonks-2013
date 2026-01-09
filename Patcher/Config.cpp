#include "pch.h"
#include "Config.h"

#include <INIReader.h>

std::string Config::gameDirectory;

std::vector<std::string> Config::assetOverrides = {
	"1014475", "1014476",						// LinkedSword
	"1014539", "1014540", "1014541", "1014542",	// LinkedPaintballGun
	"1014609", "1014610", "1014611",			// LinkedTrowel
	"1014616", "1014617", "1014618",			// LinkedTimebomb
	"1014631", "1014632", "1014633",			// LinkedSuperball
	"1014650", "1014651", "1014652",			// LinkedSlingshot
	"1014653", "1014654", "1014655",			// LinkedRocketLauncher
};

bool Config::readModernClientCookieJar;
std::string Config::robloSecurityCookie;

bool Config::fpsUnlocked;
int Config::desiredFps;

// this is to help prevent breaking physics
static const std::array<int, 4> allowedFpsValues = { 30, 60, 120, 240 };

bool Config::allowChangingScriptIdentities;

bool Config::replicateAllBrickColors;

void Config::init()
{
	gameDirectory.resize(MAX_PATH);
	GetModuleFileName(nullptr, &gameDirectory[0], MAX_PATH);
	gameDirectory.resize(gameDirectory.rfind('\\') + 1);

	INIReader ini(gameDirectory + "cool.ini");

	std::string assetOverrideList = ini.GetString("Game", "AssetOverrides", "");
	if (!assetOverrideList.empty())
	{
		std::stringstream sstream;
		sstream << assetOverrideList;

		std::string assetId;
		while (std::getline(sstream, assetId, ','))
			assetOverrides.push_back(assetId);
	}

	readModernClientCookieJar = ini.GetBoolean("Game", "ReadModernClientCookieJar", true);
	robloSecurityCookie = ini.GetString("Game", "RobloSecurityCookie", "");

	fpsUnlocked = ini.GetBoolean("Rendering", "ExperimentalFpsUnlock", false);
	desiredFps = ini.GetInteger("Rendering", "DesiredFps", 60);

	auto fpsIt = std::lower_bound(allowedFpsValues.begin(), allowedFpsValues.end(), desiredFps);
	if (fpsIt == allowedFpsValues.end())
		desiredFps = *(fpsIt - 1);
	else
		desiredFps = *fpsIt;

	allowChangingScriptIdentities = ini.GetBoolean("Lua", "AllowChangingScriptIdentities", false);

	replicateAllBrickColors = ini.GetBoolean("Network", "ReplicateAllBrickColors", true);
}
