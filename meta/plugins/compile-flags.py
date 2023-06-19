import os
from cutekit import args, cmds, context, project


def compileFlagsCmd(args: args.Args):
    ctx = context.contextFor("kernel-x86_64")
    flags = ctx.cdefs() + ctx.tools["cc"].args
    flags.extend(map(lambda i: f"-I{i}", ctx.cincls()))

    with open(os.path.join(project.root(), "compile_flags.txt"), "w") as f:
        n = 0
        while n < len(flags):
            if flags[n] != "-target":
                f.write(f"{flags[n]}\n")
            else:
                n += 1
            n += 1


cmds.append(
    cmds.Cmd("f", "flags", "Generate compile_flags.txt", compileFlagsCmd)
)
