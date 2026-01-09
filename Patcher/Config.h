#pragma once

namespace Config
{
	extern std::string gameDirectory;

	extern std::vector<std::string> assetOverrides;

	extern bool readModernClientCookieJar;
	extern std::string robloSecurityCookie;

	extern bool fpsUnlocked;
	extern int desiredFps;

	extern bool allowChangingScriptIdentities;

	extern bool replicateAllBrickColors;

	void init();
}
