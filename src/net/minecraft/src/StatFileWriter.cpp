#include "StatFileWriter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "StatBase.h"
#include "Achievement.h"
#include "StatsSyncher.h"
#include "Session.h"
#include "StatList.h"
#include "MD5String.h"
#include "J_JdomParser.h"
#include "J_JsonRootNode.h"
#include "J_JsonNode.h"
#include "J_InvalidSyntaxException.h"

namespace fs = std::filesystem;

StatFileWriter::StatFileWriter(Session *session, const std::string &file) :
	hasUnsentStats(false)
{
	fs::path file1 = fs::path(file) / "stats";
	if (!fs::exists(file1))
		fs::create_directories(file1);
	statsSyncher = new StatsSyncher(session, this, file1.string());
}

void StatFileWriter::readStat(StatBase *statbase, int_t i)
{
	writeStatToMap(writtenStats, statbase, i);
	writeStatToMap(tempStats, statbase, i);
	hasUnsentStats = true;
}

void StatFileWriter::writeStatToMap(std::map<StatBase*, int_t> &map, StatBase *statbase, int_t i)
{
	if (statbase == nullptr) return;
	auto it = map.find(statbase);
	int_t j = (it != map.end()) ? it->second : 0;
	map[statbase] = j + i;
}

std::map<StatBase*, int_t> StatFileWriter::getTempStats()
{
	return writtenStats;
}

void StatFileWriter::setTempStats(std::map<StatBase*, int_t> &map)
{
	if (map.empty()) return;
	hasUnsentStats = true;
	for (auto &pair : map)
	{
		writeStatToMap(writtenStats, pair.first, pair.second);
		writeStatToMap(tempStats, pair.first, pair.second);
	}
}

void StatFileWriter::addPendingStats(std::map<StatBase*, int_t> &map)
{
	if (map.empty()) return;
	for (auto &pair : map)
	{
		auto it = writtenStats.find(pair.first);
		int_t i = (it != writtenStats.end()) ? it->second : 0;
		tempStats[pair.first] = pair.second + i;
	}
}

void StatFileWriter::addTempStats(std::map<StatBase*, int_t> &map)
{
	if (map.empty()) return;
	hasUnsentStats = true;
	for (auto &pair : map)
	{
		writeStatToMap(writtenStats, pair.first, pair.second);
	}
}

std::map<StatBase*, int_t> StatFileWriter::parseStats(const std::string &s)
{
	std::map<StatBase*, int_t> hashmap;
	try
	{
		std::string sessionKey = "local";
		std::string checksumInput;
		J_JsonRootNode *root = J_JdomParser().parse(s);
		std::vector<J_JsonNode *> list = root->getArrayAt({std::string("stats-change")});
		for (J_JsonNode *node : list)
		{
			std::map<std::string, J_JsonNode *> fields = node->getFields();
			if (fields.empty())
				continue;
			auto it = fields.begin();
			int_t statId = std::stoi(it->first);
			int_t statVal = std::stoi(it->second->getText());
			StatBase *statbase = StatList::getStatById(statId);
			if (statbase == nullptr)
			{
				std::cout << statId << " is not a valid stat" << std::endl;
			}
			else
			{
				checksumInput += statbase->statGuid + ",";
				checksumInput += std::to_string(statVal) + ",";
				hashmap[statbase] = statVal;
			}
		}
		MD5String md5(sessionKey);
		std::string computed = md5.computeHash(checksumInput);
		std::string expected = root->getStringAt({std::string("checksum")});
		if (computed != expected)
		{
			std::cout << "CHECKSUM MISMATCH" << std::endl;
			return {};
		}
	}
	catch (J_InvalidSyntaxException &e)
	{
		std::cerr << e.what() << std::endl;
	}
	return hashmap;
}

std::string StatFileWriter::serializeStats(const std::string &s, const std::string &s1, std::map<StatBase*, int_t> &map)
{
	std::string result;
	std::string checksumInput;
	bool flag = true;
	result += "{\r\n";
	if (!s.empty() && !s1.empty())
	{
		result += "  \"user\":{\r\n";
		result += "    \"name\":\"" + s + "\",\r\n";
		result += "    \"sessionid\":\"" + s1 + "\"\r\n";
		result += "  },\r\n";
	}
	result += "  \"stats-change\":[";
	for (auto &pair : map)
	{
		StatBase *statbase = pair.first;
		if (statbase == nullptr) continue;
		int_t value = pair.second;
		if (!flag)
			result += "},";
		else
			flag = false;
		result += "\r\n    {\"" + std::to_string(statbase->statId) + "\":" + std::to_string(value);
		checksumInput += statbase->statGuid + ",";
		checksumInput += std::to_string(value) + ",";
	}
	if (!flag)
		result += "}";
	MD5String md5(s1);
	result += "\r\n  ],\r\n";
	result += "  \"checksum\":\"" + md5.computeHash(checksumInput) + "\"\r\n";
	result += "}";
	return result;
}

bool StatFileWriter::hasAchievementUnlocked(Achievement *achievement)
{
	return tempStats.find(achievement) != tempStats.end();
}

bool StatFileWriter::canUnlockAchievement(Achievement *achievement)
{
	return achievement->parentAchievement == nullptr || hasAchievementUnlocked(achievement->parentAchievement);
}

int_t StatFileWriter::writeStat(StatBase *statbase)
{
	auto it = tempStats.find(statbase);
	return (it != tempStats.end()) ? it->second : 0;
}

void StatFileWriter::func_27175_b() // Java: empty
{
}

void StatFileWriter::syncStats() // Java: statsSyncher.syncStatsFileWithMap(func_27176_a()) — synchronous local save
{
	std::map<StatBase*, int_t> map = getTempStats();
	statsSyncher->syncStatsFileWithMap(map);
}

void StatFileWriter::func_27178_d() // per-tick: flush dirty stats to local file, then update
{
	if (hasUnsentStats && statsSyncher->isIdle())
	{
		std::map<StatBase*, int_t> map = getTempStats();
		statsSyncher->syncToServer(map);
	}
	statsSyncher->update();
}
