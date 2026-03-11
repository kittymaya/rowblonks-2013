#include "pch.h"
#include "RBXCookieJar.h"
#include "Config.h"

#include <wininet.h>
#include <wincrypt.h>
#include <shlobj.h>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <base64.h>

static std::string readModernClientRobloSecurity()
{
	// open the cookie jar file

	std::string path;
	path.resize(MAX_PATH);

	if (SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, &path[0]) != S_OK)
		throw std::runtime_error("Could not get LocalAppData path");

	path.resize(path.find('\0'));
	path += "\\Roblox\\LocalStorage\\RobloxCookies.dat";

	std::ifstream cookieJarFile(path);
	if (!cookieJarFile.is_open())
		throw std::runtime_error("Could not open Roblox cookie jar");

	// read and validate json data

	rapidjson::Document document;
	rapidjson::IStreamWrapper stream(cookieJarFile);
	document.ParseStream(stream);

	cookieJarFile.close();

	if (document.HasParseError())
		throw std::runtime_error("Could not parse Roblox cookie jar");

	auto& version = document["CookiesVersion"];
	if (!version.IsString() || std::strcmp(version.GetString(), "1") != 0)
		throw std::runtime_error("Unsupported Roblox cookie jar version");

	auto& data = document["CookiesData"];
	if (!data.IsString())
		throw std::runtime_error("Invalid Roblox cookie jar data");

	// decrypt the cookie jar data

	std::string cookiesEncrypted = base64_decode(data.GetString());

	DATA_BLOB cookiesEncryptedBlob;
	cookiesEncryptedBlob.pbData = reinterpret_cast<BYTE*>(&cookiesEncrypted[0]);
	cookiesEncryptedBlob.cbData = static_cast<DWORD>(cookiesEncrypted.size());

	DATA_BLOB cookiesDecryptedBlob;

	if (!CryptUnprotectData(&cookiesEncryptedBlob, nullptr, nullptr, nullptr, nullptr, 0, &cookiesDecryptedBlob))
		throw std::runtime_error("Could not decrypt Roblox cookie jar data");

	const std::string cookiesDecrypted(reinterpret_cast<char*>(cookiesDecryptedBlob.pbData), cookiesDecryptedBlob.cbData);
	LocalFree(cookiesDecryptedBlob.pbData);

	// extract the .ROBLOSECURITY cookie

	const std::string robloSecurityKey = ".ROBLOSECURITY\t";

	size_t robloSecurityPos = cookiesDecrypted.find(robloSecurityKey);
	if (robloSecurityPos == std::string::npos)
		return "";

	robloSecurityPos += robloSecurityKey.size();

	size_t robloSecuritySize = cookiesDecrypted.find(';', robloSecurityPos);
	if (robloSecuritySize == std::string::npos)
		return cookiesDecrypted.substr(robloSecurityPos);

	robloSecuritySize -= robloSecurityPos;

	return cookiesDecrypted.substr(robloSecurityPos, robloSecuritySize);
}

void RBXCookieJar::setRobloSecurity()
{
	if (Config::readModernClientCookieJar && Config::robloSecurityCookie.empty())
		Config::robloSecurityCookie = readModernClientRobloSecurity();

	// authenticate to avoid assetdelivery rate limits
	InternetSetCookieEx("https://assetdelivery.roblox.com", ".ROBLOSECURITY", Config::robloSecurityCookie.c_str(), INTERNET_COOKIE_HTTPONLY, 0);
}
