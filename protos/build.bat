@echo off

echo build: %~dp0

for %%i in (*.proto) do (
    echo build: %%~fi
    H:\scada_work\vcpkg\installed\x64-windows\tools\protobuf\protoc -I=%~dp0 --cpp_out=%~dp0/msg %%i
)
echo build success

@REM echo cp *.pb.h;*.pb.cc to %~dp0..\mmt-collector\rpc-service
@REM xcopy %~dp0\*.pb.?* %~dp0\..\mmt-collector\rpc-service /y /f
@REM echo cp *.pb.h;*.pb.cc to %~dp0..\mmt-collector-client\rpc-service
@REM xcopy %~dp0\*.pb.?* %~dp0\..\mmt-collector-client\rpc-service /y /f
@REM echo build end