#!/bin/sh

set -v 

cache="$(dirname $0)/cache"
path="$(dirname $0)/.."

mkdir $cache

remote="https://raw.githubusercontent.com/limine-bootloader/limine/v4.x-branch-binary"
files=(limine.sys limine-deploy.c limine-hdd.h limine-cd.bin limine-cd-efi.bin)

for file in ${files[@]}
do
    echo $file
    wget $remote/$file -O $cache/$file
done

gcc $cache/limine-deploy.c -o $cache/limine-deploy

cp $cache/limine.sys $cache/limine-cd-efi.bin $cache/limine-cd.bin $path/.sysroot/

xorriso -as mkisofs -b limine-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-cd-efi.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$path/.sysroot -o $path/cpcdos.iso

$cache/limine-deploy $path/cpcdos.iso
