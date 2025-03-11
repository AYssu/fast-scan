#!/bin/bash

folder_path="/storage/emulated/0/fscan/tmp"
log_file="$folder_path/delete_log.txt"

# 日志格式化函数
log() {
    local level="$1"
    local message="$2"
    local timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    echo "[$timestamp] [$level] $message" | tee -a "$log_file"
}

# 检查文件夹路径是否存在
if [ ! -d "$folder_path" ]; then
    log ERROR "错误：文件夹路径 $folder_path 不存在。"
    exit 1
fi

# 清空或创建日志文件
> "$log_file"

# 获取文件夹中所有纯数字命名的文件，并按文件名排序
file_list=($(ls "$folder_path" | grep '^[0-9]\+$' | sort -n))

# 遍历文件列表并删除文件
for file_name in "${file_list[@]}"; do
    file="$folder_path/$file_name"
    if [ -f "$file" ]; then
        # 删除文件
        if rm "$file"; then
            log INFO "文件 $file_name 已删除。"
        else
            log ERROR "文件 $file_name 删除失败。"
        fi
    else
        log WARN "文件 $file_name 不存在，跳过。"
    fi
done

log INFO "所有符合条件的文件已删除。"