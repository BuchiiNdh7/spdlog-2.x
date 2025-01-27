// Copyright(c) 2016 Alexander Dalshov & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#include <mutex>

#include "../details/null_mutex.h"
#include "./base_sink.h"

// Avoid including windows.h (https://stackoverflow.com/a/30741042)
extern "C" __declspec(dllimport) void __stdcall OutputDebugStringA(const char *lpOutputString);
extern "C" __declspec(dllimport) int __stdcall IsDebuggerPresent();

namespace spdlog {
namespace sinks {
/*
 * MSVC sink (logging using OutputDebugStringA)
 */
template <typename Mutex>
class msvc_sink final : public base_sink<Mutex> {
public:
    msvc_sink() = default;
    explicit msvc_sink(bool check_debugger_present)
        : check_debugger_present_{check_debugger_present} {}

protected:
    void sink_it_(const details::log_msg &msg) override {
        if (check_debugger_present_ && !IsDebuggerPresent()) {
            return;
        }
        memory_buf_t formatted;
        base_sink<Mutex>::formatter_->format(msg, formatted);
        formatted.push_back('\0');  // add a null terminator for OutputDebugString
        OutputDebugStringA(formatted.data());
    }

    void flush_() override {}

    bool check_debugger_present_ = true;
};

using msvc_sink_mt = msvc_sink<std::mutex>;
using msvc_sink_st = msvc_sink<details::null_mutex>;

using windebug_sink_mt = msvc_sink_mt;
using windebug_sink_st = msvc_sink_st;

}  // namespace sinks
}  // namespace spdlog
