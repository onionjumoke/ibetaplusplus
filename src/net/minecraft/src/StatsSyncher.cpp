#include "StatsSyncher.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "Session.h"
#include "StatFileWriter.h"
#include "StatBase.h"
#include "ThreadStatSyncherReceive.h"
#include "ThreadStatSyncherSend.h"

StatsSyncher::StatsSyncher(Session *session, StatFileWriter *statfilewriter, const std::string &file) :
	isBusy(false),
	statFileWriter(statfilewriter),
	syncCounter(0),
	packetSize(0),
	session(session)
{
	std::string username = session != nullptr ? session->username : "Player";
	std::transform(username.begin(), username.end(), username.begin(), ::tolower);

	unsentDataFile = file + "/stats_" + username + "_unsent.dat";
	dataFile = file + "/stats_" + username + ".dat";
	unsentOldFile = file + "/stats_" + username + "_unsent.old";
	oldFile = file + "/stats_" + username + ".old";
	unsentTempFile = file + "/stats_" + username + "_unsent.tmp";
	tempFile = file + "/stats_" + username + ".tmp";

	if (session != nullptr && username != session->username)
	{
		renameFile(file, "stats_" + session->username + "_unsent.dat", unsentDataFile);
		renameFile(file, "stats_" + session->username + ".dat", dataFile);
		renameFile(file, "stats_" + session->username + "_unsent.old", unsentOldFile);
		renameFile(file, "stats_" + session->username + ".old", oldFile);
		renameFile(file, "stats_" + session->username + "_unsent.tmp", unsentTempFile);
		renameFile(file, "stats_" + session->username + ".tmp", tempFile);
	}

	if (std::ifstream(unsentDataFile).good())
	{
		std::map<StatBase*, int_t> map = loadDataFromFile(unsentDataFile, unsentTempFile, unsentOldFile);
		statfilewriter->setTempStats(map);
	}
	syncFromServer();
}

void StatsSyncher::renameFile(const std::string &file, const std::string &s, const std::string &file1)
{
	std::string file2 = file + "/" + s;
	if (std::ifstream(file2).good() && !std::ifstream(file1).good())
	{
		std::rename(file2.c_str(), file1.c_str());
	}
}

std::map<StatBase*, int_t> StatsSyncher::loadDataFromFile(const std::string &file, const std::string &file1, const std::string &file2)
{
	if (std::ifstream(file).good())
		return parseStatsFile(file);
	if (std::ifstream(file2).good())
		return parseStatsFile(file2);
	if (std::ifstream(file1).good())
		return parseStatsFile(file1);
	return std::map<StatBase*, int_t>();
}

std::map<StatBase*, int_t> StatsSyncher::parseStatsFile(const std::string &file)
{
	std::ifstream bufferedreader(file);
	if (!bufferedreader) return std::map<StatBase*, int_t>();

	std::string s;
	std::ostringstream stringbuilder;
	while (std::getline(bufferedreader, s))
	{
		stringbuilder << s;
	}
	return StatFileWriter::parseStats(stringbuilder.str());
}

void StatsSyncher::writeStatsFile(std::map<StatBase*, int_t> &map, const std::string &file, const std::string &file1, const std::string &file2)
{
	{
		std::ofstream printwriter(file1, std::ios::trunc);
		printwriter << StatFileWriter::serializeStats(session != nullptr ? session->username : "Player", "local", map);
	}
	if (std::ifstream(file2).good())
		std::remove(file2.c_str());
	if (std::ifstream(file).good())
		std::rename(file.c_str(), file2.c_str());
	std::rename(file1.c_str(), file.c_str());
}

void StatsSyncher::syncFromServer()
{
	if (isBusy)
		throw std::runtime_error("Can't get stats from server while StatsSyncher is busy!");
	syncCounter = 100;
	isBusy = true;
	(new ThreadStatSyncherReceive(this))->start();
}

void StatsSyncher::syncToServer(std::map<StatBase*, int_t> &map)
{
	if (isBusy)
		throw std::runtime_error("Can't save stats while StatsSyncher is busy!");
	syncCounter = 100;
	isBusy = true;
	(new ThreadStatSyncherSend(this, map))->start();
}

void StatsSyncher::syncStatsFileWithMap(std::map<StatBase*, int_t> &map)
{
	for (int_t i = 30; isBusy && --i > 0;)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	isBusy = true;
	try
	{
		writeStatsFile(map, unsentDataFile, unsentTempFile, unsentOldFile);
	}
	catch (...)
	{
	}
	isBusy = false;
}

bool StatsSyncher::isIdle()
{
	return syncCounter <= 0 && !isBusy && pendingStats.empty();
}

void StatsSyncher::update()
{
	if (syncCounter > 0) syncCounter--;
	if (packetSize > 0) packetSize--;
	if (!pendingStats.empty())
	{
		statFileWriter->addTempStats(pendingStats);
		pendingStats.clear();
	}
	if (!serverStats.empty())
	{
		statFileWriter->addPendingStats(serverStats);
		serverStats.clear();
	}
}

void StatsSyncher::receiveStats()
{
	try
	{
		if (std::ifstream(dataFile).good())
			serverStats = loadDataFromFile(dataFile, tempFile, oldFile);
	}
	catch (...)
	{
	}
	isBusy = false;
}

void StatsSyncher::sendStats(std::map<StatBase*, int_t> map)
{
	try
	{
		writeStatsFile(map, unsentDataFile, unsentTempFile, unsentOldFile);
	}
	catch (...)
	{
	}
	isBusy = false;
}

void StatsSyncher::setBusy(bool busy)
{
	isBusy = busy;
}
