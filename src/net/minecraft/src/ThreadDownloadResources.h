#pragma once

#include <string>
#include <thread>
#include <vector>

class Minecraft;

// net.minecraft.src.ThreadDownloadResources
class ThreadDownloadResources
{
public:
	ThreadDownloadResources(const std::string &file, Minecraft *minecraft);
	~ThreadDownloadResources();

	void start();
	void run();
	void reloadResources();
	void closeMinecraft();

	std::string resourcesFolder;

private:
	struct ResourceEntry
	{
		std::string key;
		long long size;
	};

	void loadResource(const std::string &file, const std::string &s);
	void downloadAndInstallResource(const std::string &baseUrl, const std::string &s, long long l, int i);
	void downloadResource(const std::string &url, const std::string &file, long long l);
	std::vector<ResourceEntry> parseResourceList(const std::string &xml);
	bool httpGet(const std::string &url, std::vector<unsigned char> &data);
	static std::string escapeUrlPath(const std::string &s);
	static std::string getXmlTag(const std::string &s, const std::string &tag);

	Minecraft *mc;
	bool closing;
	std::thread worker;
};
