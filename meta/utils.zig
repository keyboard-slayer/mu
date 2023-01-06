const std = @import("std");
const builtin = @import("builtin");
const Allocator = std.mem.Allocator;
const LibExeObjStep = std.build.LibExeObjStep;

pub fn include(dirname: []const u8, ext: []const u8, alloc: Allocator) ![][]const u8 {
    var sources = std.ArrayList([]const u8).init(alloc);

    var dir = try std.fs.cwd().openIterableDir(dirname, .{});
    var walker = try dir.walk(alloc);

    defer walker.deinit();
    defer sources.deinit();

    while (try walker.next()) |entry| {
        const e = std.fs.path.extension(entry.basename);
        if (std.mem.eql(u8, e, ext)) {
            const path = try std.fs.path.join(alloc, &.{ dirname, try alloc.dupe(u8, entry.path) });
            try sources.append(path);
        }
    }

    return sources.toOwnedSlice();
}

pub fn downloadFile(url: []const u8, out: []const u8, alloc: Allocator) !void {
    switch (builtin.os.tag) {
        .windows => {
            const arg = try std.fmt.allocPrint(alloc, "\"Invoke-WebRequest \\\"{s}\\\" -OutFile \\\"{s}\\\"", .{ url, out });

            const argv = &[_][]const u8{
                "powershell",
                "-command",
                arg,
            };

            _ = try std.ChildProcess.exec(.{ .allocator = alloc, .argv = argv });
        },
        else => {
            const argv = &[_][]const u8{ "curl", url, "--output", out };
            _ = try std.ChildProcess.exec(.{ .allocator = alloc, .argv = argv });
        },
    }
}

pub fn create_sysroot(exe: *LibExeObjStep, legacy: bool) !void {
    const boot_dir = try std.fs.path.join(exe.builder.allocator, &.{ ".sysroot", "boot" });

    const to_create = [_][]const u8{boot_dir};

    for (to_create) |dirname| {
        try std.fs.cwd().makePath(dirname);
    }

    if (!legacy) {
        try std.fs.cwd().makePath(try std.fs.path.join(exe.builder.allocator, &.{ ".sysroot", "EFI", "BOOT" }));
    }

    exe.setOutputDir(boot_dir);
}
