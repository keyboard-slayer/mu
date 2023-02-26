<div align="center">
  <img height=256 src="doc/mumu.png"/>
</div>

<br />

# The µ Microkernel
* [Introduction](#introduction)
* [Building](#building)
* [Running](#running)
* [Contributing](#contributing)
* [License](#license)

## Introduction
µ is a microkernel written in C23. It is designed to be a minimal and simple microkernel. It aim to be used as a base for other projects. It is not designed to be a full featured operating system.

## Building
Before building µ, you need to install the LLVM toolchain. You can find the instructions [here](https://llvm.org/docs/GettingStarted.html).

µ uses [osdk](https://github.com/cute-engineering/osdk) to build. You can go to the osdk repository to find the instructions to install it.

You can build µ by running the following command:
```sh
osdk i                    # Download the dependencies
osdk b --target=x86_64    # Build the dependencies
```

## Running
An installation of QEMU is required to run µ. We also recommend you to run QEMU with KVM enabled. (You can find the instructions [here](https://wiki.qemu.org/Hosts/Linux#KVM_support))

You can run µ by running the following command:
```sh
osdk s
```

## Contributing
If you want to contribute to µ, you can open a pull request. If you want to discuss something, you can open an issue or in a discussion.

## License

<a href="https://www.gnu.org/licenses/gpl-3.0.en.html"><img align="right" src="https://www.gnu.org/graphics/gplv3-with-text-136x68.png" alt="GPLv3 Logo" width="120" height="60"/></a>

µ is licensed under the GNU General Public License v3.0. You can find the full license [here](LICENSE).