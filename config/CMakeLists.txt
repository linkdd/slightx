cmake_minimum_required(VERSION 3.22.0)
project(slightx-config)

install(
  FILES limine.conf
  DESTINATION iso/boot
)

install(
  FILES boot.conf
  DESTINATION initrd
)
