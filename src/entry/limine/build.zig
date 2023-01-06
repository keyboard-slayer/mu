const std = @import("std");
const utils = @import("../../../meta/utils.zig");
const LibExeObjStep = std.build.LibExeObjStep;
const Allocator = std.mem.Allocator;
const Step = std.build.Step;


pub const RunStep = struct {
    step: Step,
    legacy: bool,
    alloc: Allocator,

    pub fn init(alloc: Allocator, legacy: bool) RunStep {
        return .{
            .step = Step.init(.run, "Bootloader runstep", alloc, RunStep.doStep),
            .legacy = legacy,
            .alloc = alloc
        };
    }

    pub fn doStep(step: *Step) anyerror!void {
        const self = @fieldParentPtr(RunStep, "step", step);
        const limine_path = try std.fs.path.join(self.alloc, &.{".", ".sysroot", "EFI", "BOOT", "BOOTX64.EFI"});

        if (self.legacy) {
            // TODO
            unreachable;
        } else {
            try utils.downloadFile(
                "https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI",
                limine_path,
                self.alloc 
            );
        }
    }
};

pub fn import(exe: *LibExeObjStep, flags: []const []const u8) !void {
    const dirname = try std.fs.path.join(exe.builder.allocator, &.{".", "src", "entry", "limine"});
    const linkpath = try std.fs.path.join(exe.builder.allocator, &.{dirname, "linker.ld"});


    try std.fs.cwd().copyFile(
        try std.fs.path.join(exe.builder.allocator, &.{dirname, "limine.cfg"}), 
        std.fs.cwd(),
        try std.fs.path.join(exe.builder.allocator, &.{".", ".sysroot", "boot", "limine.cfg"}),
        .{}
    );    

    exe.setLinkerScriptPath(std.build.FileSource {.path = linkpath});

    const files = try utils.include(dirname, ".c", exe.builder.allocator);

    exe.addCSourceFiles(files, flags);
}
