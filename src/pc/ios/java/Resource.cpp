#include "java/Resource.h"

#include <algorithm>
#include <fstream>
#include <cassert>
#include <memory>
#include <sstream>
#include <stdexcept>

#include "java/File.h"

#include "util/Memory.h"

static const File &ResourceFile()
{
	static std::unique_ptr<File> resource_file(File::openResourceDirectory());
	return *resource_file;
}

namespace Resource
{

std::istream *getResource(const jstring &name)
{
	const File &resource_file = ResourceFile();
	std::unique_ptr<File> file(File::open(resource_file, name));
	std::unique_ptr<std::istream> is(file->toStreamIn());
	if (!is)
	{
		// Assets are user-supplied and loaded during dyld static initializers
		// (SharedConstants reads font.txt pre-main). Returning an empty stream
		// keeps startup alive so the delegate can show the "assets required"
		// alert instead of crashing.
		return new std::istringstream();
	}
	return is.release();
}

}
