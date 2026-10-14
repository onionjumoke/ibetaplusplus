#include "java/File.h"

#include <stack>
#include <string>
#include <iostream>

jstring File::getName() const
{
	size_t pos = path.find_last_of(u"/\\");
	if (pos == jstring::npos)
		return path;
	return path.substr(pos + 1);
}

jstring File::toString() const
{
	return path;
}

jstring File::toURL() const
{
	return u"file://" + path;
}

bool File::mkdirs() const
{
	// Get directory up to last one that exists
	std::stack<std::unique_ptr<File>> back;

	jstring current = path;
	while (!current.empty())
	{
		auto fp = File::open(current);
		if (fp->isDirectory())
			break;

		size_t npos = current.find_last_of(u"/\\");
		if (npos == std::string::npos)
			break;
		back.emplace(std::move(fp));
		current = current.substr(0, npos);
	}

	if (back.empty())
		return false;

	// Create directories
	while (!back.empty())
	{
		auto fp = back.top().get();
		if (!fp->mkdir())
			return false;
		back.pop();
	}

	return true;
}
