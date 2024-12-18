// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#include "spdlog/details/context.h"

#include "spdlog/logger.h"

#ifndef SPDLOG_DISABLE_GLOBAL_LOGGER
    #include "spdlog/sinks/stdout_color_sinks.h"
#endif  // SPDLOG_DISABLE_GLOBAL_LOGGER

#include <memory>

namespace spdlog {
namespace details {

context::context(std::unique_ptr<logger> global_logger) { global_logger_ = std::move(global_logger); }

std::shared_ptr<logger> context::global_logger() { return global_logger_; }

// Return raw ptr to the global logger.
// To be used directly by the spdlog default api (e.g. spdlog::info)
// This make the default API faster, but cannot be used concurrently with set_global_logger().
// e.g do not call set_global_logger() from one thread while calling spdlog::info() from another.
logger *context::global_logger_raw() const noexcept { return global_logger_.get(); }

// set global logger
void context::set_logger(std::shared_ptr<logger> new_global_logger) { global_logger_ = std::move(new_global_logger); }

void context::set_tp(std::shared_ptr<thread_pool> tp) {
    std::lock_guard lock(tp_mutex_);
    tp_ = std::move(tp);
}

std::shared_ptr<thread_pool> context::get_tp() {
    std::lock_guard lock(tp_mutex_);
    return tp_;
}

// clean all resources and threads started by the registry
void context::shutdown() {
    std::lock_guard lock(tp_mutex_);
    tp_.reset();
}

std::recursive_mutex &context::tp_mutex() { return tp_mutex_; }

}  // namespace details
}  // namespace spdlog
