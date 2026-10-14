#pragma once

#include <string>
#include <utility>
#include <functional>

#include "java/Type.h"

class jstring : public std::u16string
{
public:
	using std::u16string::u16string;
	using std::u16string::operator=;

	jstring() = default;
	jstring(const std::u16string &value) : std::u16string(value) {}
	jstring(std::u16string &&value) noexcept : std::u16string(std::move(value)) {}
	jstring(const std::string &value);
	jstring(const char *value);

	operator std::string() const;
};

inline bool operator==(const jstring &left, const jstring &right)
{
	return static_cast<const std::u16string &>(left) == static_cast<const std::u16string &>(right);
}

inline bool operator!=(const jstring &left, const jstring &right) { return !(left == right); }

inline bool operator==(const jstring &left, const char16_t *right)
{
	return static_cast<const std::u16string &>(left) == right;
}

inline bool operator==(const char16_t *left, const jstring &right) { return right == left; }
inline bool operator!=(const jstring &left, const char16_t *right) { return !(left == right); }
inline bool operator!=(const char16_t *left, const jstring &right) { return !(right == left); }

inline bool operator<(const jstring &left, const jstring &right)
{
	return static_cast<const std::u16string &>(left) < static_cast<const std::u16string &>(right);
}

inline jstring operator+(const jstring &left, const jstring &right)
{
	std::u16string result(left);
	result.append(right);
	return result;
}

inline jstring operator+(const jstring &left, const char16_t *right)
{
	return left + jstring(right);
}

inline jstring operator+(const char16_t *left, const jstring &right)
{
	return jstring(left) + right;
}

inline jstring operator+(const jstring &left, char16_t right)
{
	jstring result(left);
	result.push_back(right);
	return result;
}

inline jstring operator+(char16_t left, const jstring &right)
{
	jstring result(1, left);
	result.append(right);
	return result;
}

inline jstring operator+(const jstring &left, const std::string &right) { return left + jstring(right); }
inline jstring operator+(const std::string &left, const jstring &right) { return jstring(left) + right; }
inline jstring operator+(const jstring &left, const char *right) { return left + jstring(right); }
inline jstring operator+(const char *left, const jstring &right) { return jstring(left) + right; }

// std::hash specialization so jstring works in unordered_map/set
namespace std
{
template<>
struct hash<jstring>
{
    size_t operator()(const jstring &s) const noexcept
    {
        return hash<std::u16string>()(static_cast<const std::u16string &>(s));
    }
};
} // namespace std

namespace String
{

jstring fromUTF8(const std::string &str);
std::string toUTF8(const jstring &str);

jstring toString(int_t v, int_t base = 10);
jstring toString(long_t v, int_t base = 10);
jstring toString(uint_t v, int_t base = 10);
jstring toString(ulong_t v, int_t base = 10);
jstring toString(float v);
jstring toString(double v);

inline jstring fromInt(int_t v) { return toString(v); }

}
