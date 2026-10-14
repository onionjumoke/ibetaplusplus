#include "TexturePackList.h"

#include <iostream>
#include <cstdlib>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "TexturePackDefault.h"
#include "TexturePackCustom.h"
#include "Minecraft.h"
#include "GameSettings.h"

namespace fs = std::filesystem;

TexturePackList::TexturePackList(Minecraft *minecraft, const std::string &file) :
	mc(minecraft),
	defaultTexturePack(new TexturePackDefault()),
	selectedTexturePack(nullptr)
{
	texturePackDir = file + "/texturepacks";
	if (!fs::exists(texturePackDir))
	{
		fs::create_directories(texturePackDir);
	}
	currentTexturePack = minecraft->gameSettings->skin;
	updateAvailableTexturePacks();
	selectedTexturePack->loadTexturePack();
}

bool TexturePackList::setTexturePack(TexturePackBase *texturepackbase)
{
	if (texturepackbase == selectedTexturePack)
	{
		return false;
	}
	else
	{
		selectedTexturePack->closeTexturePackFile();
		currentTexturePack = texturepackbase->texturePackFileName;
		selectedTexturePack = texturepackbase;
		mc->gameSettings->skin = currentTexturePack;
		mc->gameSettings->saveOptions();
		selectedTexturePack->loadTexturePack();
		return true;
	}
}

void TexturePackList::updateAvailableTexturePacks()
{
	std::vector<TexturePackBase*> arraylist;
	selectedTexturePack = nullptr;
	arraylist.push_back(defaultTexturePack);

	std::vector<fs::path> scanDirs;
	scanDirs.push_back(texturePackDir);
	// On iOS the writable, iTunes/Finder-shared location is Documents/, so pick
	// up packs dropped there (root or a texturepacks/ subfolder) as well.
	if (const char *home = ::getenv("HOME"))
	{
		fs::path docs = fs::path(home) / "Documents";
		scanDirs.push_back(docs);
		scanDirs.push_back(docs / "texturepacks");
	}
	for (const fs::path &scanDir : scanDirs)
	{
		if (!fs::exists(scanDir) || !fs::is_directory(scanDir)) continue;
		for (const auto &entry : fs::directory_iterator(scanDir))
		{
			if (!entry.is_regular_file()) continue;
			std::string filename = entry.path().filename().string();
			std::string ext = entry.path().extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
			if (ext != ".zip") continue;

			auto file = entry.path();
			std::string s = filename + ":" + std::to_string(fs::file_size(file)) + ":" + std::to_string(fs::last_write_time(file).time_since_epoch().count());

			try
			{
				auto it = soundPool.find(s);
				if (it == soundPool.end())
				{
					TexturePackCustom *texturepackcustom = new TexturePackCustom(file.string());
					texturepackcustom->texturePackFolder = s;
					soundPool[s] = texturepackcustom;
					texturepackcustom->getTexturePackFolder(mc);
				}
				TexturePackBase *texturepackbase1 = soundPool[s];
				if (texturepackbase1->texturePackFileName == currentTexturePack)
				{
					selectedTexturePack = texturepackbase1;
				}
				arraylist.push_back(texturepackbase1);
			}
			catch (std::exception &ioexception)
			{
				std::cerr << ioexception.what() << std::endl;
			}
		}
	}

	if (selectedTexturePack == nullptr)
	{
		selectedTexturePack = defaultTexturePack;
	}

	for (auto *tp : availableTexturePacks)
	{
		bool found = false;
		for (auto *tp2 : arraylist)
		{
			if (tp == tp2) { found = true; break; }
		}
		if (!found)
		{
			soundPool.erase(tp->texturePackFolder);
			delete tp;
		}
	}

	availableTexturePacks = arraylist;
}

const std::vector<TexturePackBase*> &TexturePackList::getAvailableTexturePacks() const
{
	return availableTexturePacks;
}

TexturePackBase *TexturePackList::getSelectedTexturePack() const
{
	return selectedTexturePack;
}
