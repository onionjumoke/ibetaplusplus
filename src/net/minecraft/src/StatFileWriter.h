#pragma once

#include <map>
#include <string>
#include <memory>
#include "java/Type.h"

class Session;
class StatBase;
class Achievement;
class StatsSyncher;

// net.minecraft.src.StatFileWriter
class StatFileWriter
{
public:
	StatFileWriter(Session *session, const std::string &file);

	void readStat(StatBase *statbase, int_t i);
	std::map<StatBase*, int_t> getTempStats();
	void setTempStats(std::map<StatBase*, int_t> &map);
	void addPendingStats(std::map<StatBase*, int_t> &map);
	void addTempStats(std::map<StatBase*, int_t> &map);

	static std::map<StatBase*, int_t> parseStats(const std::string &s);
	static std::string serializeStats(const std::string &s, const std::string &s1, std::map<StatBase*, int_t> &map);

	bool hasAchievementUnlocked(Achievement *achievement);
	bool canUnlockAchievement(Achievement *achievement);
	int_t writeStat(StatBase *statbase);

	void func_27175_b();
	void syncStats();
	void func_27178_d();

private:
	void writeStatToMap(std::map<StatBase*, int_t> &map, StatBase *statbase, int_t i);

	std::map<StatBase*, int_t> tempStats;
	std::map<StatBase*, int_t> writtenStats;
	bool hasUnsentStats;
	StatsSyncher *statsSyncher;
};
