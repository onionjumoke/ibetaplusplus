// ─────────────────────────────────────────────────────────────────────────────
// typeinfo_shims.cpp — the iOS 5 libc++abi.dylib predates several C++11
// exception types; their typeinfo objects never made it into the system
// runtime. Taking strong addresses below emits them into this archive so the
// linker finds local copies instead of failing.
// ─────────────────────────────────────────────────────────────────────────────

#include <new>
#include <typeinfo>

namespace
{

__attribute__((used)) static const void *const g_forcedTypeinfos[] = {
    &typeid(std::bad_array_new_length),
    &typeid(std::bad_alloc),
    &typeid(std::bad_cast),
    &typeid(std::bad_exception),
    &typeid(std::bad_typeid),
    &typeid(std::exception),
};

} // namespace
