#include "ThreadDownloadResources.h"

#include <algorithm>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <stdexcept>

#include "Minecraft.h"
#include "GameResources.h"
#include "java/JavaNetwork.h"

namespace fs = std::filesystem;

ThreadDownloadResources::ThreadDownloadResources(const std::string &file, Minecraft *minecraft)
	: resourcesFolder((fs::path(GameResources::getExeDir()) / "resources").string())
	, mc(minecraft)
	, closing(false)
{
	(void)file;
	fs::create_directories(resourcesFolder);
}

ThreadDownloadResources::~ThreadDownloadResources()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadDownloadResources::start()
{
	worker = std::thread(&ThreadDownloadResources::run, this);
	worker.detach();
}

void ThreadDownloadResources::run()
{
#ifdef MC_IOS
	// No MinecraftResources endpoint on device and the resources ship in the
	// bundle; load them directly instead of blocking on a doomed HTTP fetch.
	loadResource(resourcesFolder, "");
	return;
#endif
	try
	{
		std::string url = "http://s3.amazonaws.com/MinecraftResources/";
		std::vector<unsigned char> data;
		if (!httpGet(url, data))
			throw std::runtime_error("Could not load MinecraftResources listing");

		std::string xml(data.begin(), data.end());
		std::vector<ResourceEntry> entries = parseResourceList(xml);
		for (int_t i = 0; i < 2; i++)
		{
			for (const ResourceEntry &entry : entries)
			{
				if (entry.size <= 0LL)
					continue;
				downloadAndInstallResource(url, entry.key, entry.size, i);
				if (closing)
					return;
			}
		}
	}
	catch (std::exception &exception)
	{
		loadResource(resourcesFolder, "");
		std::cerr << exception.what() << std::endl;
	}
}

void ThreadDownloadResources::reloadResources()
{
	loadResource(resourcesFolder, "");
}

void ThreadDownloadResources::loadResource(const std::string &file, const std::string &s)
{
	if (!fs::exists(file))
		return;
	for (const auto &entry : fs::directory_iterator(file))
	{
		if (closing)
			return;
		if (entry.is_directory())
		{
			loadResource(entry.path().string(), s + entry.path().filename().string() + "/");
			continue;
		}
		try
		{
			mc->installResource(s + entry.path().filename().string(), entry.path().string());
		}
		catch (...)
		{
			std::cout << "Failed to add " << s << entry.path().filename().string() << std::endl;
		}
	}
}

void ThreadDownloadResources::downloadAndInstallResource(const std::string &baseUrl, const std::string &s, long long l, int i)
{
	try
	{
		std::size_t j = s.find('/');
		if (j == std::string::npos)
			return;
		std::string s1 = s.substr(0, j);
		if (s1 == "sound" || s1 == "newsound")
		{
			if (i != 0)
				return;
		}
		else if (i != 1)
		{
			return;
		}

		fs::path file = fs::path(resourcesFolder) / fs::path(s);
		if (!fs::exists(file) || (long long)fs::file_size(file) != l)
		{
			fs::create_directories(file.parent_path());
			downloadResource(baseUrl + escapeUrlPath(s), file.string(), l);
			if (closing)
				return;
		}
		mc->installResource(s, file.string());
	}
	catch (std::exception &exception)
	{
		std::cerr << exception.what() << std::endl;
	}
}

void ThreadDownloadResources::downloadResource(const std::string &url, const std::string &file, long long)
{
	std::vector<unsigned char> data;
	if (!httpGet(url, data))
		throw std::runtime_error("Failed to download resource: " + url);
	std::ofstream output(file, std::ios::binary | std::ios::trunc);
	if (!output)
		throw std::runtime_error("Failed to write resource: " + file);
	const std::size_t chunk = 4096;
	for (std::size_t pos = 0; pos < data.size(); pos += chunk)
	{
		if (closing)
			return;
		std::size_t count = std::min(chunk, data.size() - pos);
		output.write(reinterpret_cast<const char *>(&data[pos]), (std::streamsize)count);
	}
}

std::vector<ThreadDownloadResources::ResourceEntry> ThreadDownloadResources::parseResourceList(const std::string &xml)
{
	std::vector<ResourceEntry> entries;
	std::size_t pos = 0;
	while (true)
	{
		std::size_t start = xml.find("<Contents>", pos);
		if (start == std::string::npos)
			break;
		std::size_t end = xml.find("</Contents>", start);
		if (end == std::string::npos)
			break;
		std::string node = xml.substr(start, end - start);
		std::string key = getXmlTag(node, "Key");
		std::string size = getXmlTag(node, "Size");
		if (!key.empty() && !size.empty())
			entries.push_back(ResourceEntry{ key, std::stoll(size) });
		pos = end + 11;
	}
	return entries;
}

bool ThreadDownloadResources::httpGet(const std::string &url, std::vector<unsigned char> &data)
{
	return JavaNetwork::readUrl(url, data);
}

std::string ThreadDownloadResources::escapeUrlPath(const std::string &s)
{
	std::string out;
	for (char c : s)
	{
		if (c == ' ')
			out += "%20";
		else
			out += c;
	}
	return out;
}

std::string ThreadDownloadResources::getXmlTag(const std::string &s, const std::string &tag)
{
	std::string open = "<" + tag + ">";
	std::string close = "</" + tag + ">";
	std::size_t start = s.find(open);
	if (start == std::string::npos)
		return "";
	start += open.length();
	std::size_t end = s.find(close, start);
	if (end == std::string::npos)
		return "";
	return s.substr(start, end - start);
}

void ThreadDownloadResources::closeMinecraft()
{
	closing = true;
}
