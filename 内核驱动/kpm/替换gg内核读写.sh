#!/bin/bash

# 设置错误时退出
set -e

# 获取脚本所在目录
SCRIPT_DIR=$(cd "$(dirname "$0")" && pwd)

# 定义文件路径（与脚本在同一目录）
FIX_GG="$SCRIPT_DIR/fix_gg"
LIB5_SO="$SCRIPT_DIR/lib5.so"
LIBKPM_SO="$SCRIPT_DIR/libkpm.so"

# Android 目标目录
TARGET_DIR="/data/local/tmp"

echo "========================================="
echo "Starting deployment process..."
echo "========================================="

# 检查文件是否存在
echo "Checking files..."
if [ ! -f "$FIX_GG" ]; then
    echo "Error: fix_gg not found at $FIX_GG"
    exit 1
fi

if [ ! -f "$LIB5_SO" ]; then
    echo "Error: lib5.so not found at $LIB5_SO"
    exit 1
fi

if [ ! -f "$LIBKPM_SO" ]; then
    echo "Error: libkpm.so not found at $LIBKPM_SO"
    exit 1
fi

echo "All files found!"

# 推送文件到设备
echo ""
echo "Pushing files to $TARGET_DIR..."
cp -r "$FIX_GG" "$TARGET_DIR/fix_gg"
cp -r "$LIB5_SO" "$TARGET_DIR/lib5.so"
cp -r "$LIBKPM_SO" "$TARGET_DIR/libkpm.so"

echo ""
echo "Setting permissions..."
# 设置 fix_gg 权限为 777
chmod 777 "$TARGET_DIR/fix_gg"

echo ""
echo "========================================="
echo "Executing fix_gg on device..."
echo "========================================="

# 执行 fix_gg
cd $TARGET_DIR && ./fix_gg

echo ""
echo "========================================="
echo "Deployment and execution completed!"
echo "========================================="