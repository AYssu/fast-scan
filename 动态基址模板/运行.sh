#!/bin/bash

# 定义源文件路径和目标路径
source_file="./libs/arm64-v8a/fscan_pointer"
target_dir="/data/"
target_file="${target_dir}/fscan_pointer"

# 检查源文件是否存在
if [ ! -f "$source_file" ]; then
    echo "错误：源文件 $source_file 不存在"
    exit 1
fi

# 检查目标目录是否存在
if [ ! -d "$target_dir" ]; then
    echo "错误：目标目录 $target_dir 不存在"
    exit 1
fi

# 检查是否有权限写入目标目录
if [ ! -w "$target_dir" ]; then
    echo "错误：没有权限写入目标目录 $target_dir"
    exit 1
fi

# 移动文件到目标目录
echo "正在将文件移动到 $target_dir..."
if ! mv "$source_file" "$target_file"; then
    echo "错误：无法移动文件到 $target_dir"
    exit 1
fi

# 给文件添加可执行权限
echo "正在为文件添加可执行权限..."
if ! chmod +x "$target_file"; then
    echo "错误：无法为文件添加可执行权限"
    exit 1
fi

# 执行文件
echo "正在执行文件..."
"$target_file"

# 检查执行是否成功
if [ $? -ne 0 ]; then
    echo "错误：文件执行失败"
    exit 1
fi

echo "文件执行成功！"