msbuild /p:Configuration="Release" build64\reaper.sln
msbuild /p:Configuration="Release" build64_cpu\reaper.sln

del ..\packages\reaperv13beta4_64.zip
cd build64\release
zip -9 -q -r ../../../packages/reaperv13beta4_64.zip *
cd ..\..
cd build64_cpu\release
xcopy /Y reaper.exe reaper_cpuonly.exe
zip -9 -q -r ../../../packages/reaperv13beta4_64.zip reaper_cpuonly.exe
cd ..\..
