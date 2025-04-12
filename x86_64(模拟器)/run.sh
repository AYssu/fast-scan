#!/bin/bash

default_file="executable"
# 使用默认值
file_name=${1:-$default_file}

if [ $# -eq 0 ]; then
    echo "[-] 使用默认文件: $file_name"
fi

if [ ! -f "$file_name" ]; then
    echo "[-] 文件 \"$file_name\" 不存在"
    exit
fi

su -c pkill $file_name 
file_path="/data/local/tmp/$file_name"
su -c cp -r $file_name ${file_path}

# 检查文件是否有执行权限
if [ ! -x "${file_path}" ]; then
    echo "[*] 文件 \"$file_name\" 没有执行权限"
    su -c chmod +x "${file_path}"
    if [ ! -x "${file_path}" ]; then
        echo "[-] 给予权限失败 程序退出"
        exit
    else
        echo "[+] 给予权限成功"
    fi
fi

echo "[+]=================================="
su -c ${file_path}