#include "utils/base.h"
#include <folly/Format.h>
#include <folly/futures/Future.h>
#include <folly/executors/ThreadedExecutor.h>
#include <folly/Uri.h>
#include <folly/FBString.h>
#include <ranges>

static void print_uri(const folly::fbstring& value)
{
    const folly::Uri uri(value);
    spdlog::info("The authority from {} is {}", value, uri.authority());

    auto const ints = {0, 1, 2, 3, 4, 5};
    auto even = [](int i) { return 0 == i % 2; };
    auto square = [](int i) { return i * i; };
    for (int i: ints | std::views::filter(even) | std::views::transform(square)) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}

int main(int argc, char** argv)
{
    folly::ThreadedExecutor executor;
    folly::Promise<std::string> promise;
    folly::Future<std::string> future = promise.getSemiFuture().via(&executor);
    folly::Future<folly::Unit> unit = std::move(future).thenValue(print_uri);
    promise.setValue("https://github.com/bincrafters");
    std::move(unit).get();
}
