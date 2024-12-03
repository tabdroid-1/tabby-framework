#pragma once

#include <unordered_set>
#include <unordered_map>
#include <shared_mutex>
#include <filesystem>
#include <functional>
#include <typeindex>
#include <algorithm>
#include <optional>
#include <iostream>
#include <fstream>
#include <utility>
#include <sstream>
#include <cassert>
#include <cstdint>
#include <atomic>
#include <random>
#include <memory>
#include <vector>
#include <thread>
#include <string>
#include <cstdio>
#include <cmath>
#include <mutex>
#include <tuple>
#include <array>
#include <queue>
#include <any>
#include <bit>
#include <map>
#include <set>

#include "tabby/log.h"
#include "tabby/defines.h"

#ifdef TB_PLATFORM_WINDOWS
#    include <windows.h>
#endif
