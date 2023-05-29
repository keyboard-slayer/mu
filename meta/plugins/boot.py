import os

from cutekit import shell, builder
from cutekit.const import CACHE_DIR, PROJECT_CK_DIR
from cutekit.cmds import Cmd, append
from cutekit.args import Args

def kvmAvailable() -> bool:
    if os.path.exists("/dev/kvm") and os.access("/dev/kvm", os.R_OK):
        return True
    return False

def installLimine(bootDir: str, efiBootDir: str) -> None:
    limine = shell.wget(
        "https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary/BOOTX64.EFI"
    )

    shell.cp(limine, os.path.join(efiBootDir, "BOOTX64.EFI"))

def limineGenConfig(bootDir: str, pkgs: list[str]) -> None:
    with open(os.path.join(bootDir, "limine.cfg"), "w") as cfg:
        cfg.write(
            "TIMEOUT=0\n:Mu\nPROTOCOL=limine\nKERNEL_PATH=boot:///boot/kernel.elf\n"
        )

        for pkg in pkgs:
            cfg.write(f"MODULE_PATH=boot:///bin/{pkg}\n")


def limineAddSubmodules(bootDir: str, modPath: str) -> None:
    with open(os.path.join(bootDir, "limine.cfg")) as cfg:
        cfg.write(f"MODULE_PATH=boot://{modPath}\n")


def downloadOvmf() -> str:
    path = os.path.join(CACHE_DIR, "OVMF.fd")

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
            os.path.join(CACHE_DIR, "data.tar.xz"),
            "-C",
            os.path.dirname(deb),
        ]
    )

    shell.cp(os.path.join(CACHE_DIR, "usr", "share", "ovmf", "OVMF.fd"), path)

    return path


def bootCmd(args: Args) -> None:
    debug = "debug" in args.opts
    no_ui = "no-ui" in args.opts
    imageDir = shell.mkdir(os.path.join(PROJECT_CK_DIR, "boot"))
    efiBootDir = shell.mkdir(os.path.join(imageDir, "EFI", "BOOT"))
    bootDir = shell.mkdir(os.path.join(imageDir, "boot"))

    # ovmf = downloadOvmf()
    ovmf = shell.wget("https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd")
    mu = builder.build("mu-core", "kernel-x86_64:debug")

    shell.cp(mu.outfile(), os.path.join(bootDir, "kernel.elf"))

    installLimine(bootDir, efiBootDir)
    limineGenConfig(bootDir, [])

    qemuCmd: list[str] = [
        "qemu-system-x86_64",
        "-no-reboot",
        "-no-shutdown",
        "-serial", "mon:stdio",
        "-bios", ovmf,
        "-m", "4G",
        "-smp", "4",
        "-drive",
        f"file=fat:rw:{imageDir},media=disk,format=raw",
    ]

    if debug:
        qemuCmd += ["-s", "-S", "-d", "int", "-M", "smm=off"]

    if no_ui:
        qemuCmd += ["-display", "none"]

    if kvmAvailable():
        qemuCmd += ["-enable-kvm", "-cpu", "host"]
    else:
        print("KVM not available, using QEMU-TCG")

    shell.exec(*qemuCmd)

append(Cmd("s", "start", "Boot the system", bootCmd))
