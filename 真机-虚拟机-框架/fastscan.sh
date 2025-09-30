#!/bin/bash

# 定义包名
PACKAGE_NAME="com.fastscan"
TERMUX_BIN_PATH="/data/data/com.termux/files/usr/bin"
SCRIPT_NAME="fastscan.sh"
LINK_NAME="fscan"

# 检查 Termux 是否安装
if [ ! -d "$TERMUX_BIN_PATH" ]; then
    echo "Termux 未安装，请先安装 Termux。"
    exit 1
fi

# 检查是否为 root 用户
if [ "$(id -u)" -ne 0 ]; then
    echo "[-] 请以 root 方式执行此脚本"
    exit 1
fi

# 检查当前路径是否包含 com.termux
if [[ "$PWD" != *com.termux* ]]; then
    echo "当前路径不包含 com.termux，脚本无法在 Termux 外执行。"
    
    # 将脚本移动到 Termux 的 bin 目录
    cp -r "$0" "$TERMUX_BIN_PATH/$SCRIPT_NAME"
    echo "脚本已安装至 Termux 的 $TERMUX_BIN_PATH/$SCRIPT_NAME"
    chmod 777 "$TERMUX_BIN_PATH/$SCRIPT_NAME"
    # 创建链接
    ln -s "$TERMUX_BIN_PATH/$SCRIPT_NAME" "$TERMUX_BIN_PATH/$LINK_NAME"
    echo "已创建链接 $TERMUX_BIN_PATH/$LINK_NAME 指向 $TERMUX_BIN_PATH/$SCRIPT_NAME"
    
    exit 0
fi

# 检查应用是否存在
pm list packages | grep -q "$PACKAGE_NAME"
if [ $? -eq 0 ]; then
    echo "应用 $PACKAGE_NAME 已安装。"
    
    # 检查可执行文件是否存在
    EXECUTABLE_PATH="/data/user/0/com.fastscan/files/executable"
    if [ -f "$EXECUTABLE_PATH" ]; then
        echo "可执行文件 $EXECUTABLE_PATH 存在。"
        
        # 修改权限为777
        chmod 777 "$EXECUTABLE_PATH"
        echo "已将 $EXECUTABLE_PATH 权限修改为777。"
        
        # 执行可执行文件
        "$EXECUTABLE_PATH"
        echo "$EXECUTABLE_PATH 已执行。"
    else
        echo "应用未初始化，请先至少使用一次。"
    fi
else
    echo "应用 $PACKAGE_NAME 未安装，请先安装APK版本。"
fi
