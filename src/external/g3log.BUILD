# Description:
#   G3log is a logging library that allows us to publish at multiple logging levels,
#   log to custom sinks, etc.
#   https://github.com/KjellKod/g3log

# G3log uses cmake to generate some files, so we need to run it
# before we build
genrule(
    name = "g3log_cmake",
    outs = ["include/g3log/generated_definitions.hpp"],
    srcs = glob(["**/*"]),
    cmd = "\n".join([
        # Run cmake, silencing both stdout (">") and stderr ("2>")
        "cmake -DCHANGE_G3LOG_DEBUG_TO_DBUG external/g3log > /dev/null 2> /dev/null",
        # Copy the generated header to the location bazel expects it
        "mv include/g3log/generated_definitions.hpp $@",
    ]),
)

cc_library(
    name = "g3log",
    srcs = glob(
        ["src/*.cpp", "src/*.ipp"],
        exclude = [
            "src/crashhandler_windows.cpp",
            "src/stacktrace_win.cpp",
            "src/stacktrace_windows.cpp",
        ],
    ),
    hdrs = glob(
               ["src/g3log/*.hpp"],
               exclude = ["src/g3log/stacktrace_windows.hpp"],
           ) +
           # We explicitly require this header so that bazel knows to run the genrule
           # above to create it
           ["include/g3log/generated_definitions.hpp"],
    includes = ["./src", "./include"],
    visibility = ["//visibility:public"],
)
