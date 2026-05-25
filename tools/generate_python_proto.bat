@echo off
REM Requires protoc on PATH (from vcpkg or protobuf release)
protoc --python_out=%~dp0 -I%~dp0..\proto %~dp0..\proto\telemetry.proto
echo Generated tools\telemetry_pb2.py
