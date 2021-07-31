# Sombrero-quest
C++ reimplementation of il2cpp classes. Uses bs-hooks and codegen

# Why is it called Sombrero?
Sombreros are cool. 

# How to use:
Add to qpm `sombrero` using `qpm dependency add sombrero` (use version ranges please)

This expects you to have some sort of codegen whether it may be BS codegen, monkecodegen or whatever else may come. 

When published to QPM, this does not publish codegen as a dependency allowing game agnostic use.

Note: This also works without codegen, but you should attempt to use it anyways.

Add any header of your choice, being `sombrero/shared/ColorUtils.hpp` for example.

# Contribute
Make things cool, make changes.

Download NDK, use either CLion or VS Code. Run `qpm restore` to retrieve dependencies. 

Test your changes by adding the header to `test.cpp` and compiling with `powershell ./build.ps1`
