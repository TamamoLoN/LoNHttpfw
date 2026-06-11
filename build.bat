@echo off
REM =========================================================
REM Author: TamamoLoN 1016052306@qq.com
REM Date: 2024-07-15
REM Description: Build script for Windows
REM =========================================================

SETLOCAL ENABLEDELAYEDEXPANSION

REM 设置目录和可执行文件名
SET dir_name=build
SET bin_name=bin

REM 获取 CPU 核心数（Windows 没有 nproc，使用环境变量）
SET cpu_core=%NUMBER_OF_PROCESSORS%

REM 检查 build 目录
IF EXIST "%dir_name%" (
    ECHO 目录 %dir_name% 已存在。
) ELSE (
    ECHO 目录 %dir_name% 不存在，正在创建...
    mkdir "%dir_name%"
    ECHO 目录 %dir_name% 创建成功。
)

REM 检查 bin 目录
IF EXIST "%bin_name%" (
    ECHO 目录 %bin_name% 已存在。
) ELSE (
    ECHO 目录 %bin_name% 不存在，正在创建...
    mkdir "%bin_name%"
    ECHO 目录 %bin_name% 创建成功。
)

REM 更新 Git 子模块
git submodule sync --recursive
git submodule update --init --recursive

REM 进入 build 目录
cd %dir_name%

ECHO CPU 核心数为 %cpu_core%，开始编译...

REM 生成 Visual Studio 项目文件
cmake ..
REM 编译
cmake --build . --config Release --target install

IF %ERRORLEVEL% EQU 0 (
    ECHO 编译成功
) ELSE (
    ECHO 编译失败
    EXIT /B -1
)

REM 如果使用 VS 多配置，可以使用 cmake --build
REM cmake --build .. --config Release --target install

ENDLOCAL