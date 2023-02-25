import os

from osdk.context import loadAllComponents
from osdk import utils, shell, builder
from osdk.cmds import Cmd, append
from osdk.args import Args


def kvmAvailable() -> bool:
    if os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK):
        return True
    return False


def installLimine(bootDir: str, efiBootDir: str) -> None:
    limine = shell.wget(
        "https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI"
    )

    shell.cp(limine, f"{efiBootDir}/BOOTX64.EFI")


def buildPkgs(binDir: str, debug: bool) -> list[str]:
    pkgs = [p.id for p in loadAllComponents() if "src/pkg" in p.dirname()]

    for pkg in pkgs:
        elf = builder.build(pkg, "munix-x86_64:debug")
        shell.cp(elf, f"{binDir}/{os.path.basename(elf)[:-4]}")

    return pkgs


def limineGenConfig(bootDir: str, pkgs: list[str]) -> None:
    with open(f"{bootDir}/limine.cfg", "w") as cfg:
        cfg.write(
            "TIMEOUT=0\n:Munix\nPROTOCOL=limine\nKERNEL_PATH=boot:///boot/kernel.elf\n"
        )

        for pkg in pkgs:
            cfg.write(f"MODULE_PATH=boot:///bin/{pkg}")


def bootCmd(args: Args) -> None:
    debug = "debug" in args.opts
    imageDir = shell.mkdir(".osdk/images/efi-x86_64")
    efiBootDir = shell.mkdir(f"{imageDir}/EFI/BOOT")
    binDir = shell.mkdir(f"{imageDir}/bin")
    bootDir = shell.mkdir(f"{imageDir}/boot")

    ovmf = shell.wget(
        "https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd"
    )

    munix = builder.build("munix-core", "kernel-x86_64:debug")

    shell.cp(munix, f"{bootDir}/kernel.elf")

    pkgs = buildPkgs(binDir, debug)
    installLimine(bootDir, efiBootDir)
    limineGenConfig(bootDir, pkgs)

    qemuCmd: list[str] = [
        "qemu-system-x86_64",
        # "-d" , "int",
        "-machine",
        "q35",
        "-no-reboot",
        "-no-shutdown",
        # "-S", "-s",
        "-serial",
        "mon:stdio",
        # "stdio",
        "-bios",
        ovmf,
        "-m",
        "256M",
        "-smp",
        "4",
        "-drive",
        f"file=fat:rw:{imageDir},media=disk,format=raw",
    ]

    if kvmAvailable():
        qemuCmd += ["-enable-kvm"]
    else:
        print("KVM not available, using QEMU-TCG")

    shell.exec(*qemuCmd)


append(Cmd("s", "start", "Boot the system", bootCmd))
