// ─────────────────────────────────────────────────────────────────────────────
// cxa_shims.cpp — runtime helpers that iOS 5's libc++abi.dylib predates.
//
// __cxa_bad_typeid / __cxa_bad_cast are raised by typeid/dynamic_cast on null
// or polymorphically-broken objects; the game's entity registry does both at
// runtime, so these must exist. The system dylib doesn't export them, so we
// provide the canonical implementations here (they just throw).
// ─────────────────────────────────────────────────────────────────────────────

#include <cxxabi.h>
#include <new>
#include <typeinfo>

extern "C" {

void __cxa_bad_typeid()
{
	throw std::bad_typeid();
}

void __cxa_bad_cast()
{
	throw std::bad_cast();
}

} // extern "C"
