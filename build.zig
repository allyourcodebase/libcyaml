const std = @import("std");

const flags: []const []const u8 = &.{
    "-std=c11",
    "-Wall",
    "-Wextra",
    "-pedantic",
    "-Wconversion",
    "-Wwrite-strings",
    "-Wcast-align",
    "-Wpointer-arith",
    "-Winit-self",
    "-Wshadow",
    "-Wstrict-prototypes",
    "-Wmissing-prototypes",
    "-Wredundant-decls",
    "-Wundef",
    "-Wvla",
    "-Wdeclaration-after-statement",
};

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const libyaml_dep = b.dependency("libyaml", .{
        .target = target,
        .optimize = optimize,
    });

    const cyaml = b.addStaticLibrary(.{
        .name = "cyaml",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    cyaml.defineCMacro("VERSION_MAJOR", "1");
    cyaml.defineCMacro("VERSION_MINOR", "4");
    cyaml.defineCMacro("VERSION_PATCH", "1");
    cyaml.defineCMacro("VERSION_DEVEL", "0");
    cyaml.addIncludePath(b.path("include"));
    cyaml.addCSourceFiles(.{
        .files = &.{
            "src/free.c",
            "src/load.c",
            "src/mem.c",
            "src/save.c",
            "src/utf8.c",
            "src/util.c",
        },
        .flags = flags,
    });
    cyaml.linkLibrary(libyaml_dep.artifact("yaml"));

    cyaml.installHeader(b.path("include/cyaml/cyaml.h"), "cyaml/cyaml.h");
    b.installArtifact(cyaml);

    const test_exe = b.addExecutable(.{
        .name = "test-cyaml",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    test_exe.addCSourceFiles(.{
        .files = &.{
            "test/units/free.c",
            "test/units/load.c",
            "test/units/test.c",
            "test/units/util.c",
            "test/units/errs.c",
            "test/units/file.c",
            "test/units/save.c",
            "test/units/utf8.c",
        },
        .flags = flags,
    });
    test_exe.linkLibrary(cyaml);
    std.fs.cwd().makePath("build") catch {};

    const test_run = b.addRunArtifact(test_exe);
    // TODO: print stderr on failure

    const test_step = b.step("test", "Run test executable");
    test_step.dependOn(&test_run.step);
}
