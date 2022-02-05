#pragma once


#include <cstdint>

#if _WIN32 || _WIN64
#if _WIN64
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif

#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64BIT
#else
#define ENV32BIT
#endif
#endif


#if defined(ENV64BIT)

namespace platform {
    using integer = std::int64_t;
}

#else

namespace platform {

namespace platform {
    using integer = std::int32_t;
}

}

#endif