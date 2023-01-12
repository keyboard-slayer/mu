const std = @import("std");
const utils = @import("../../../../meta/utils.zig");
const Allocator = std.mem.Allocator;
const LibExeObjStep = std.build.LibExeObjStep;
const Features = std.Target.x86.Feature;
const builtin = @import("builtin");
const Step = std.build.Step;

pub const RunStep = struct {
    step: Step,
    alloc: Allocator,
    debug: bool,

    pub fn init(alloc: Allocator, debug: bool) RunStep {
        return .{
            .step = Step.init(.run, "Arch runstep", alloc, RunStep.doStep),
            .debug = debug,
            .alloc = alloc,
        };
    }

    pub fn doStep(step: *Step) anyerror!void {
        const self = @fieldParentPtr(RunStep, "step", step);
        const ovmf = try std.fs.path.join(self.alloc, &.{ ".", "zig-cache", "tmp", "OVMF.fd" });

        try utils.downloadFile("https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd", ovmf, self.alloc);
    }
};

pub fn import(exe: *LibExeObjStep, flags: []const []const u8, qemu_args: *std.ArrayList([]const u8)) !void {
    const dirname = try std.fs.path.join(exe.builder.allocator, &.{ ".", "src", "kernel", "arch", "x86_64" });
    const ovmf = try std.fs.path.join(exe.builder.allocator, &.{ ".", "zig-cache", "tmp", "OVMF.fd" });

    var target: std.zig.CrossTarget = .{
        .cpu_arch = .x86_64,
        .os_tag = .freestanding,
        .abi = .none,
    };

    target.cpu_features_sub.addFeature(@enumToInt(Features.mmx));
    target.cpu_features_sub.addFeature(@enumToInt(Features.sse));
    target.cpu_features_sub.addFeature(@enumToInt(Features.sse2));
    target.cpu_features_sub.addFeature(@enumToInt(Features.avx));
    target.cpu_features_sub.addFeature(@enumToInt(Features.avx2));
    target.cpu_features_add.addFeature(@enumToInt(Features.soft_float));

    try qemu_args.append("qemu-system-x86_64");

    switch (builtin.os.tag) {
        .windows => {
            try qemu_args.appendSlice(&.{
                "-cpu", "max"
            });
        },
        .linux => {
            try qemu_args.appendSlice(&.{
                "-enable-kvm", "-cpu", "host"
            });
        },
        .macos => {
            try qemu_args.appendSlice(&.{
                "-cpu", "max"
            });
        },
        else => {
            std.log.err("Unknown {}", .{builtin.os.tag});
            std.os.exit(1);
        },
    }

    try qemu_args.*.appendSlice(&.{
        "-no-reboot",
        "-no-shutdown",
        "-d", "guest_errors",
        "-serial", "mon:stdio",
        "-bios", ovmf,
        "-smp", "4",
        "-M", "q35",
        "-drive", "file=fat:rw:.sysroot,media=disk,format=raw",
    });

    exe.setTarget(target);
    exe.addCSourceFiles(try utils.include(dirname, ".c", exe.builder.allocator), flags);

    for (try utils.include(dirname, ".s", exe.builder.allocator)) |as| {
        exe.addAssemblyFile(as);
    }
}
