{
  description = "Lean OpenGL - C++ 23";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
  };

  outputs =
    { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      llvm = pkgs.llvmPackages_latest;
    in
    {
      devShells.${system}.default =
        pkgs.mkShell.override
          {
            stdenv = llvm.libcxxStdenv;
          }
          {
            packages = [
              llvm.clang-tools
              llvm.lld
              llvm.lldb

              pkgs.cmake
              pkgs.ninja
              pkgs.pkg-config
              pkgs.neocmakelsp
              pkgs.cmake-format

              pkgs.glfw
              pkgs.libepoxy
              pkgs.glm
              pkgs.libGL
              pkgs.libglvnd
              pkgs.libx11
              pkgs.libxcb
              pkgs.libxau
              pkgs.libxdmcp
            ];

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath ([
              pkgs.libGL
              pkgs.libglvnd
              pkgs.wayland
              llvm.libcxx
              pkgs.libepoxy
              pkgs.glfw
            ]);

            shellHook = ''
              export LSAN_OPTIONS="suppressions=$PWD/lsan.supp"

              echo "======== C++23 DevShell ========"
              echo "Compiler : $(clang++ --version | head -1)"
              echo "CMake    : $(cmake --version | head -1)"
              echo "Ninja    : $(ninja --version)"
              echo ""
              echo "Presets  : cmake --preset <debug|release|msan|tsan>"
              echo "Build    : cmake --build --preset <debug|release|msan|tsan>"
            '';
          };
    };
}
