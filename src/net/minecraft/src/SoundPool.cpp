#include "SoundPool.h"

#include <algorithm>

SoundPool::SoundPool() :
	rand(),
	nameToSoundPoolEntriesMapping(),
	allSoundPoolEntries(),
	numberOfSoundPoolEntries(0),
	motionX(true)
{
}

SoundPoolEntry *SoundPool::addSound(const jstring &s, const std::string &file)
{
	jstring s1 = s;
	jstring name = s;

	// Remove extension
	size_t dotPos = name.find(u'.');
	if (dotPos != jstring::npos)
		name = name.substr(0, dotPos);

	// Remove trailing digits if motionX is true
	if (motionX)
	{
		while (!name.empty() && name.back() >= u'0' && name.back() <= u'9')
			name.pop_back();
	}

	// Replace '/' with '.'
	std::replace(name.begin(), name.end(), u'/', u'.');

	auto it = nameToSoundPoolEntriesMapping.find(name);
	if (it == nameToSoundPoolEntriesMapping.end())
	{
		nameToSoundPoolEntriesMapping[name] = std::vector<SoundPoolEntry*>();
		it = nameToSoundPoolEntriesMapping.find(name);
	}

	SoundPoolEntry *entry = new SoundPoolEntry(s1, file);
	it->second.push_back(entry);
	allSoundPoolEntries.push_back(entry);
	numberOfSoundPoolEntries++;

	return entry;
}

SoundPoolEntry *SoundPool::getRandomSound()
{
	if (allSoundPoolEntries.empty())
		return nullptr;

	int_t idx = rand.nextInt(allSoundPoolEntries.size());
	return allSoundPoolEntries[idx];
}

SoundPoolEntry *SoundPool::getRandomSoundFromSoundPool(const jstring &s)
{
	auto it = nameToSoundPoolEntriesMapping.find(s);
	if (it == nameToSoundPoolEntriesMapping.end())
		return nullptr;

	std::vector<SoundPoolEntry*> &entries = it->second;
	if (entries.empty())
		return nullptr;

	int_t idx = rand.nextInt(entries.size());
	return entries[idx];
}
