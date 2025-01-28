#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <memory>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <regex>
#include <type_traits>
#include <exception>
#include <stdexcept>
#include <cassert>
#include <cstdarg>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cfloat>
#include <climits>
#include <clocale>
#include <csetjmp>
#include <sched.h>     
#include <ucontext.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <orbis/libkernel.h>

#include "defs/kernelExt.h"
#include "defs/mspace.h"
#include "defs/ptrace.h"

#include "kernelExt.h"
#include "windows_wrapper.h"
#include "buffer.h"
#include "orbis.h"
#include "trampoline.h"

#include "../minhook.h"
