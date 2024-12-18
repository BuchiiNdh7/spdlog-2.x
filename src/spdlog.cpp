// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/spdlog.h"

#include <cassert>
#include <memory>

#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace spdlog {

static std::shared_ptr s_context =
#ifndef SPDLOG_DISABLE_GLOBAL_LOGGER
    std::make_unique<details::context>(std::make_unique<logger>(std::string(), std::make_unique<sinks::stdout_color_sink_mt>()));
#else
    std::make_unique<details::context>();  // empty context
#endif

void set_context(std::shared_ptr<details::context> context) { s_context = std::move(context); }

std::shared_ptr<details::context> context() { return s_context; }

const std::shared_ptr<details::context> &context_ref() { return s_context; }

std::shared_ptr<logger> global_logger() { return context_ref()->global_logger(); }

void set_global_logger(std::shared_ptr<logger> global_logger) { context()->set_logger(std::move(global_logger)); }

logger *global_logger_raw() noexcept {
    auto *rv = context_ref()->global_logger_raw();
    assert(rv != nullptr);
    return rv;
}

void set_formatter(std::unique_ptr<formatter> formatter) { global_logger()->set_formatter(std::move(formatter)); }

void set_pattern(std::string pattern, pattern_time_type time_type) {
    set_formatter(std::make_unique<pattern_formatter>(std::move(pattern), time_type));
}

level get_level() { return global_logger()->log_level(); }

bool should_log(level level) { return global_logger()->should_log(level); }

void set_level(level level) { global_logger()->set_level(level); }

void flush_on(level level) { global_logger()->flush_on(level); }

void set_error_handler(void (*handler)(const std::string &msg)) { global_logger()->set_error_handler(handler); }

void shutdown() { s_context.reset(); }

}  // namespace spdlog
