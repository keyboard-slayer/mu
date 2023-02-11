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
    shell.cp(f"src/entry/limine/limine.cfg", f"{bootDir}/limine.cfg")

def buildPkgs(binDir: str) -> None:
    pkgs = [p for p in loadAllComponents() if "src/pkg" in p.dirname()]

    for pkg in pkgs:
        bin = builder.build(pkg.id, "munix-x86_64")
        shell.cp(bin, f"{binDir}/{os.path.basename(bin)}")



def bootCmd(args: Args) -> None:
    imageDir = shell.mkdir(".osdk/images/efi-x86_64")
    efiBootDir = shell.mkdir(f"{imageDir}/EFI/BOOT")
    binDir = shell.mkdir(f"{imageDir}/bin")
    bootDir = shell.mkdir(f"{imageDir}/boot")

    buildPkgs(binDir)

    ovmf = shell.wget("https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd")

    munix = builder.build("core", "kernel-x86_64")
    shell.cp(munix, f"{bootDir}/kernel.elf")
    installLimine(bootDir, efiBootDir)

    qemuCmd: list[str] = [
        "qemu-system-x86_64",
        "-machine",
        "q35",
        "-no-reboot",
        "-no-shutdown",
        # "-d", "guest_errors,cpu_reset,int",
        "-serial",
        "mon:stdio",
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
