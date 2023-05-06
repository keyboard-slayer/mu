import os

from distutils.dir_util import copy_tree
from osdk.context import loadAllComponents
from osdk import utils, shell, builder
from osdk.const import CACHE_DIR
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
    pkgs = [p.id for p in loadAllComponents() if "src/servers" in p.dirname()]

    for pkg in pkgs:
        elf = builder.build(pkg, "mu-x86_64:debug:o0")
        shell.cp(elf, f"{binDir}/{os.path.basename(elf)[:-4]}")

    return pkgs


def limineGenConfig(bootDir: str, pkgs: list[str]) -> None:
    with open(f"{bootDir}/limine.cfg", "w") as cfg:
        cfg.write(
            "TIMEOUT=0\n:Mu\nPROTOCOL=limine\nKERNEL_PATH=boot:///boot/kernel.elf\n"
        )

        for pkg in pkgs:
            cfg.write(f"MODULE_PATH=boot:///bin/{pkg}\n")


def limineAddSubmodules(bootDir: str, modPath: str) -> None:
    with open(f"{bootDir}/limine.cfg", "a") as cfg:
        cfg.write(f"MODULE_PATH=boot://{modPath}\n")


def downloadOvmf() -> str:
    path = f"{CACHE_DIR}/OVMF.fd"

    if os.path.isfile(path):
        return path

    deb = shell.wget(
        "http://ftp.debian.org/debian/pool/main/e/edk2/ovmf_2020.11-2+deb11u1_all.deb"
    )

    shell.exec(*["ar", "x", "--output", CACHE_DIR, deb])
    shell.exec(
        *[
            "tar",
            "xf",
            f"{CACHE_DIR}/data.tar.xz",
            "-C",
            os.path.dirname(deb),
        ]
    )

    shell.cp(f"{CACHE_DIR}/usr/share/ovmf/OVMF.fd", path)

    return path


def bootCmd(args: Args) -> None:
    debug = "debug" in args.opts
    imageDir = shell.mkdir(".osdk/images/mu-x86_64")
    efiBootDir = shell.mkdir(f"{imageDir}/EFI/BOOT")
    binDir = shell.mkdir(f"{imageDir}/bin")
    bootDir = shell.mkdir(f"{imageDir}/boot")

    # ovmf = downloadOvmf()
    ovmf = shell.wget("https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd")

    mu = builder.build("mu-core", "kernel-x86_64:debug")

    shell.cp(mu, f"{bootDir}/kernel.elf")
    copy_tree("meta/sysroot", imageDir)

    pkgs = buildPkgs(binDir, debug)
    installLimine(bootDir, efiBootDir)
    limineGenConfig(bootDir, pkgs)
    limineAddSubmodules(bootDir, "/etc/rc.json")

    qemuCmd: list[str] = [
        "qemu-system-x86_64",
        # "-d", "int",
        # "-M", "smm=off",
        "-no-reboot",
        "-no-shutdown",
        # "-cpu", "host",
        "-serial", "mon:stdio",  # "stdio",
        "-bios", ovmf,
        "-display", "none",
        "-m", "4G",
        "-smp", "4",
        "-drive",
        f"file=fat:rw:{imageDir},media=disk,format=raw",
    ]

    if debug:
        qemuCmd += ["-s", "-S"]

    if kvmAvailable():
        qemuCmd += ["-enable-kvm"]
    else:
        print("KVM not available, using QEMU-TCG")

    shell.exec(*qemuCmd)


append(Cmd("s", "start", "Boot the system", bootCmd))
