module;
// put all implementation-provided headers into the global module fragment
// to prevent attachment to this module
#if !defined(_CRT_SECURE_NO_WARNINGS) && defined(_MSC_VER)
#  define _CRT_SECURE_NO_WARNINGS
#endif
#if !defined(WIN32_LEAN_AND_MEAN) && defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#endif

#include <cctype>
#include <cerrno>
#include <climits>
#include <clocale>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>

#include <algorithm>
#include <chrono>
#include <exception>
#include <functional>
#include <iterator>
#include <limits>
#include <locale>
#include <memory>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>
#include <vector>

#if _MSC_VER
#  include <intrin.h>
#endif
#if defined __APPLE__ || defined(__FreeBSD__)
#  include <xlocale.h>
#endif
#if __has_include(<winapifamily.h>)
#  include <winapifamily.h>
#endif
#if (__has_include(<fcntl.h>) || defined(__APPLE__) || \
     defined(__linux__)) &&                            \
    (!defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP))
#  include <fcntl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  ifndef _WIN32
#    include <unistd.h>
#  else
#    include <io.h>
#  endif
#endif
#ifdef _WIN32
#  include <windows.h>
#endif

export module fmt;

#define FMT_MODULE_EXPORT export
#define FMT_MODULE_EXPORT_BEGIN export {
#define FMT_MODULE_EXPORT_END }

// all library-provided declarations and definitions
// must be in the module purview to be exported
#include "fmt/format.h"
#include "fmt/args.h"
#include "fmt/color.h"
#include "fmt/compile.h"
#include "fmt/locale.h"
#include "fmt/chrono.h"
#include "fmt/printf.h"
#include "fmt/os.h"

module : private;

#include "format.cc"
#include "os.cc"
