// =========================
// Cpcdos OS3 - Build System
// =========================
//
// Build Options
// =============
//
// -Ddebug=[true|false] -> Enable debug mode and if in run step, enable GDB server
// -Dlegacy=[true|false] -> Enable legacy boot (MBR), if set to false enables UEFI
// -Dbootloader=[bootloader] -> Set the bootloader you want to use
// -Dtarget=[arch] -> Set the architecture for the build (default is x86_64)
//
// Custom Steps
// ============
//
// run -> Run Cpcdos using QEMU
//

const std = @import("std");
const builtin = @import("builtin");
const Builder = std.build.Builder;
const Arch = std.Target.Cpu.Arch;
const Allocator = std.mem.Allocator;

// Bootloaders
const limine = @import("src/entry/limine/build.zig");

// Archs
const x86_64 = @import("src/kernel/arch/x86_64/build.zig");

// Utilities
const utils = @import("meta/utils.zig");

const buildError = error{
    InvalidArchError,
    LoaderInvalidArch,
};

const Bootloader = enum {
    limine,
};

const cflags = &.{
    "-Wall",
    "-Wextra",
    "-Werror",
};

pub fn build(b: *Builder) !void {
    const exe = b.addExecutable("krnl64.elf", null);

    const run = b.step("run", "Runs Cpcdos OS3");
    const debug = b.option(bool, "debug", "Enable debug build") orelse false;
    const legacy = b.option(bool, "legacy", "Enable legacy boot (if false, enables UEFI)") orelse false;
    const bootloader = b.option(Bootloader, "bootloader", "Bootloader") orelse Bootloader.limine;
    const arch = b.option(Arch, "target", "Target architecture") orelse Arch.x86_64;

    const src = try std.fs.path.join(b.allocator, &.{ ".", "src" });
    const libpath = try std.fs.path.join(b.allocator, &.{ src, "libs" });
    const corepath = try std.fs.path.join(b.allocator, &.{ src, "kernel", "core" });

    const core = try utils.include(corepath, ".c", b.allocator);
    const libs = try utils.include(libpath, ".c", b.allocator);

    var qemu_args = std.ArrayList([]const u8).init(b.allocator);
    defer qemu_args.deinit();

    exe.want_lto = false;
    exe.red_zone = false;
    exe.code_model = .kernel;

    try utils.create_sysroot(exe, legacy);

    switch (bootloader) {
        Bootloader.limine => {
            if (arch != Arch.x86_64) {
                return error.LoaderInvalidArch;
            }

            try limine.import(exe, cflags);

            var boot_run_step = b.allocator.create(limine.RunStep) catch unreachable;
            boot_run_step.* = limine.RunStep.init(b.allocator, legacy);
            boot_run_step.step.dependOn(&exe.step);
            run.dependOn(&boot_run_step.step);
        },
    }

    switch (arch) {
        Arch.x86_64 => {
            try x86_64.import(exe, cflags, &qemu_args);

            var arch_run_step = b.allocator.create(x86_64.RunStep) catch unreachable;
            arch_run_step.* = x86_64.RunStep.init(b.allocator, debug);
            arch_run_step.step.dependOn(&exe.step);
            run.dependOn(&arch_run_step.step);
        },
        else => {
            return error.InvalidArchError;
        },
    }

    if (debug) {
        exe.setBuildMode(.Debug);
        try qemu_args.appendSlice(&.{ "-S", "-s" });
    } else {
        exe.setBuildMode(.ReleaseFast);
    }

    const qemu_cmd = b.addSystemCommand(qemu_args.toOwnedSlice());
    run.dependOn(&qemu_cmd.step);

    exe.addIncludePath(libpath);
    exe.addCSourceFiles(core, cflags);
    exe.addCSourceFiles(libs, cflags);

    exe.install();
}
