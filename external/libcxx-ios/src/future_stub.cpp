// ─────────────────────────────────────────────────────────────────────────────
// future_stub.cpp — upstream future.cpp needs __cxa_init_primary_exception,
// which the iOS 5 libc++abi.dylib predates. The game never uses futures
// (verified: no std::future/promise/async anywhere in src/), but thread.o
// references one symbol from that object, so we provide a stub instead of
// dragging the whole translation unit in.
// ─────────────────────────────────────────────────────────────────────────────

#include <future>

_LIBCPP_BEGIN_NAMESPACE_STD

void __assoc_sub_state::__make_ready()
{
	__libcpp_verbose_abort("Beta++ iOS: std::future is not supported on this target");
}

_LIBCPP_END_NAMESPACE_STD
