@echo off
for /r %%i in (*.frag) do %VULKAN_SDK%/Bin/glslangValidator.exe -V  -o %%i.spv %%i
for /r %%i in (*.vert) do %VULKAN_SDK%/Bin/glslangValidator.exe -V  -o %%i.spv %%i
