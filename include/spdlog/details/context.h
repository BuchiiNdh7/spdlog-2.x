// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

// Loggers registry of unique name->logger pointer
// An attempt to create a logger with an already existing name will result with spdlog_ex exception.
// If user requests a non-existing logger, nullptr will be returned
// This class is thread safe

#include <memory>
#include <mutex>
#include <string>

#include "../common.h"
#include "./periodic_worker.h"

namespace spdlog {
class logger;

namespace details {
class thread_pool;

class SPDLOG_API context {
public:
    context() = default;
    explicit context(std::unique_ptr<logger> global_logger);
    ~context() = default;
    context(const context &) = delete;
    context &operator=(const context &) = delete;

    [[nodiscard]] std::shared_ptr<logger> global_logger();

    // Return raw ptr to the global logger.
    // To be used directly by the spdlog global api (e.g. spdlog::info)
    // This make the global API faster, but cannot be used concurrently with set_global_logger().
    // e.g do not call set_global_logger() from one thread while calling spdlog::info() from
    // another.
    [[nodiscard]] logger *global_logger_raw() const noexcept;

    // set logger instance.
    void set_logger(std::shared_ptr<logger> new_logger);

    void set_tp(std::shared_ptr<thread_pool> tp);

    [[nodiscard]] std::shared_ptr<thread_pool> get_tp();

    // clean all resources
    void shutdown();
    [[nodiscard]] std::recursive_mutex &tp_mutex();

private:
    std::recursive_mutex tp_mutex_;
    std::shared_ptr<thread_pool> tp_;
    std::shared_ptr<logger> global_logger_;
};

}  // namespace details
}  // namespace spdlog
