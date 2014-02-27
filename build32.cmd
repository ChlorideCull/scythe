msbuild /p:Configuration="Release" build\reaper.sln
msbuild /p:Configuration="Release" build_cpu\reaper.sln

del ..\packages\reaperv13beta4_32.zip
cd build\release
zip -9 -q -r ../../../packages/reaperv13beta4_32.zip *
cd ..\..
cd build_cpu\release
xcopy /Y reaper.exe reaper_cpuonly.exe
zip -9 -q -r ../../../packages/reaperv13beta4_32.zip reaper_cpuonly.exe
cd ..\..
