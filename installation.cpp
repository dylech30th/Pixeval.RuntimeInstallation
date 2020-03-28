#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "Urlmon.lib")

#include <iostream>
#include <regex>
#include <cstdio>
#include <sstream>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <WinInet.h>
#include <ShlObj.h>
#include <KnownFolders.h>

#define OUTPUT(x) std::cout << (x) << std::endl

bool check_runtime_installed() {
	char tmp[1024];
	std::string buf;
	auto* child = _popen("dotnet --list-runtimes", "r");
	if (child == nullptr)
		throw std::runtime_error("we'd encountered unexpected problem");
	while (std::fgets(tmp, sizeof tmp, child))
		buf += tmp;
	_pclose(child);
	std::vector<std::string> vec;
	std::stringstream ss(buf);
	std::string t;
	while (std::getline(ss, t, '\n')) {
		if (!std::all_of(t.begin(), t.end(), [](const char& s) { return std::isspace(s); }) && t.find("preview") == std::string::npos) {
			vec.push_back(t.substr(0, t.find(" [")));
		}
	}
	const std::regex reg(R"(Microsoft.WindowsDesktop.App 3\.1\.[0-9]+)");
	return std::any_of(vec.begin(), vec.end(), [&reg](const std::string& s) { return std::regex_match(s, reg); });
}

bool download_file(const wchar_t* src, const wchar_t* dst) {
	DeleteUrlCacheEntry(src);
	if (URLDownloadToFile(nullptr, src, dst, 0, nullptr) == S_OK) {
		return true;
	}
	return false;
}

std::wstring get_download_directory() {
	wchar_t* buf;
	SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &buf);
	const std::wstring download_folder(buf);
	CoTaskMemFree(buf);
	return download_folder;
}

int main(int argc, char* argv[]) {
	if (check_runtime_installed()) {
		OUTPUT("你已经安装有版本号高于3.1的.NET Core Desktop Runtime, 无需再次安装");
		system("pause");
		return 0;
	}
	const std::wstring url = L"https://download.visualstudio.microsoft.com/download/pr/3240250e-6fe0-4258-af69-85abef6c00de/e01ee0af6c65d894f4a02bdf6705ec7b/windowsdesktop-runtime-3.1.2-win-x64.exe";
	const auto file_name = get_download_directory().append(L"win-desktop-runtime.exe");
	OUTPUT("正在下载...");
	if (download_file(url.c_str(), file_name.c_str())) {
		ShellExecute(nullptr, L"open", file_name.c_str(), nullptr, nullptr, SW_HIDE);
		return 0;
	}
	OUTPUT("下载例程出现错误");
	system("pause");
}
