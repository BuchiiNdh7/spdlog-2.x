//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

//
// bench.cpp : spdlog benchmarks
//
#include <fmt/format.h>

#include <atomic>
#include <cstdlib>  // EXIT_FAILURE
#include <memory>
#include <string>
#include <thread>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/sinks/null_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"
#include "utils.h"

void bench(int howmany, std::shared_ptr<spdlog::logger> log);
void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, size_t thread_count);

// void bench_default_api(int howmany, std::shared_ptr<spdlog::logger> log);
// void bench_c_string(int howmany, std::shared_ptr<spdlog::logger> log);

static const size_t file_size = 30 * 1024 * 1024;
static const size_t rotating_files = 5;
static const int max_threads = 1000;

void bench_threaded_logging(size_t threads, int iters) {
    spdlog::info("**************************************************************");
    spdlog::info(
        spdlog::fmt_lib::format(std::locale("en_US.UTF-8"), "Multi threaded: {:L} threads, {:L} messages", threads, iters));
    spdlog::info("**************************************************************");

    auto basic_mt = spdlog::basic_logger_mt("basic_mt", "logs/basic_mt.log", true);
    bench_mt(iters, std::move(basic_mt), threads);

    spdlog::info("");
    auto rotating_mt = spdlog::rotating_logger_mt("rotating_mt", "logs/rotating_mt.log", file_size, rotating_files);
    bench_mt(iters, std::move(rotating_mt), threads);

    spdlog::info("");
    auto daily_mt = spdlog::daily_logger_mt("daily_mt", "logs/daily_mt.log");
    bench_mt(iters, std::move(daily_mt), threads);

    spdlog::info("");
    auto empty_logger = std::make_shared<spdlog::logger>("level-off");
    empty_logger->set_level(spdlog::level::off);
    bench(iters, empty_logger);
}

void bench_single_threaded(int iters) {
    spdlog::info("**************************************************************");
    spdlog::info(spdlog::fmt_lib::format(std::locale("en_US.UTF-8"), "Single threaded: {} messages", iters));
    spdlog::info("**************************************************************");

    auto basic_st = spdlog::basic_logger_st("basic_st", "logs/basic_st.log", true);
    bench(iters, std::move(basic_st));

    spdlog::info("");
    auto rotating_st = spdlog::rotating_logger_st("rotating_st", "logs/rotating_st.log", file_size, rotating_files);
    bench(iters, std::move(rotating_st));

    spdlog::info("");
    auto daily_st = spdlog::daily_logger_st("daily_st", "logs/daily_st.log");
    bench(iters, std::move(daily_st));

    spdlog::info("");
    auto empty_logger = std::make_shared<spdlog::logger>("level-off");
    empty_logger->set_level(spdlog::level::off);
    bench(iters, empty_logger);
}

int main(int argc, char *argv[]) {
    spdlog::global_logger()->set_pattern("[%^%l%$] %v");
    int iters = 250000;
    size_t threads = 4;
    try {
        if (argc > 1) {
            iters = std::stoi(argv[1]);
        }
        if (argc > 2) {
            threads = std::stoul(argv[2]);
        }

        if (threads > max_threads) {
            throw std::runtime_error(spdlog::fmt_lib::format("Number of threads exceeds maximum({})", max_threads));
        }

        bench_single_threaded(iters);
        bench_threaded_logging(1, iters);
        bench_threaded_logging(threads, iters);
    } catch (std::exception &ex) {
        spdlog::error(ex.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void bench(int howmany, std::shared_ptr<spdlog::logger> log) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i) {
        log->info("Hello logger: msg number {}", i);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();

    spdlog::info(spdlog::fmt_lib::format(std::locale("en_US.UTF-8"), "{:<30} Elapsed: {:0.2f} secs {:>16L}/sec", log->name(),
                                         delta_d, static_cast<size_t>(howmany / delta_d)));
}

void bench_mt(int howmany, std::shared_ptr<spdlog::logger> log, size_t thread_count) {
    using std::chrono::duration;
    using std::chrono::duration_cast;
    using std::chrono::high_resolution_clock;

    std::vector<std::thread> threads;
    threads.reserve(thread_count);
    auto start = high_resolution_clock::now();
    for (size_t t = 0; t < thread_count; ++t) {
        threads.emplace_back([&]() {
            for (int j = 0; j < howmany / static_cast<int>(thread_count); j++) {
                log->info("Hello logger: msg number {}", j);
            }
        });
    }

    for (auto &t : threads) {
        t.join();
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::info(spdlog::fmt_lib::format(std::locale("en_US.UTF-8"), "{:<30} Elapsed: {:0.2f} secs {:>16L}/sec", log->name(),
                                         delta_d, static_cast<size_t>(howmany / delta_d)));
}

/*
void bench_default_api(int howmany, std::shared_ptr<spdlog::logger> log)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::duration_cast;

    auto orig_default = spdlog::global_logger();
    spdlog::set_global_logger(log);
    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        spdlog::info("Hello logger: msg number {}", i);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::drop(log->name());
    spdlog::set_global_logger(std::move(orig_default));
    spdlog::info("{:<30} Elapsed: {:0.2f} secs {:>16}/sec", log->name(), delta_d, int(howmany /
delta_d));
}

void bench_c_string(int howmany, std::shared_ptr<spdlog::logger> log)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration;
    using std::chrono::duration_cast;

    const char *msg = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum pharetra
metus cursus " "lacus placerat congue. Nulla egestas, mauris a tincidunt tempus, enim lectus
volutpat mi, eu consequat sem " "libero nec massa. In dapibus ipsum a diam rhoncus gravida. Etiam
non dapibus eros. Donec fringilla dui sed " "augue pretium, nec scelerisque est maximus. Nullam
convallis, sem nec blandit maximus, nisi turpis ornare " "nisl, sit amet volutpat neque massa eu
odio. Maecenas malesuada quam ex, posuere congue nibh turpis duis.";

    auto orig_default = spdlog::global_logger();
    spdlog::set_global_logger(log);
    auto start = high_resolution_clock::now();
    for (auto i = 0; i < howmany; ++i)
    {
        spdlog::log(spdlog::level::info, msg);
    }

    auto delta = high_resolution_clock::now() - start;
    auto delta_d = duration_cast<duration<double>>(delta).count();
    spdlog::drop(log->name());
    spdlog::set_global_logger(std::move(orig_default));
    spdlog::info("{:<30} Elapsed: {:0.2f} secs {:>16}/sec", log->name(), delta_d, int(howmany /
delta_d));
}

*/
