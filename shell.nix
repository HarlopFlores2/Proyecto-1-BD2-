{ pkgs ? import (builtins.fetchTarball {
  name = "nixos-clangd-working";
  url =
    "https://github.com/nixos/nixpkgs/archive/9e49886b3d83d18ca59f66ec7b388315fa3d2e60.tar.gz";
  sha256 = "0bkdm4wf5vlg6mgv6r6672imx84axi5s159jd1f90gj5da2mzmfm";
}) { } }:

with pkgs;

mkShell rec {
  buildInputs = [ llvmPackages_latest.libcxx ];

  packages = [
    clang_14
    gcc

    cgdb
    clang-tools
    cling
    gdb
    valgrind

    cmake
    just
    meson
    ninja
    pkg-config
  ];

  CPATH = lib.makeSearchPathOutput "dev" "include" buildInputs;
}
