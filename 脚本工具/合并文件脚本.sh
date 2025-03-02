#!/bin/bash

folder_path="/storage/emulated/0/fscan/tmp"
output_file="$folder_path/total.txt"
log_file="$folder_path/merge_log.txt"

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

# 创建输出文件所在的目录
mkdir -p "$(dirname "$output_file")"

# 清空或创建输出文件
> "$output_file"
> "$log_file"

# 获取文件夹中所有纯数字命名的文件，并按文件名排序
file_list=($(ls "$folder_path" | grep '^[0-9]\+$' | sort -n))

# 开始合并的时间
start_time=$(date +%s)

# 遍历文件列表
for file_name in "${file_list[@]}"; do
    file="$folder_path/$file_name"
    if [ -f "$file" ]; then
        # 开始合并单个文件的时间
        file_start_time=$(date +%s)
        # 使用 dd 将文件内容追加到输出文件
        if dd if="$file" of="$output_file" bs=1M oflag=append conv=notrunc &> /dev/null; then
            rm "$file"
            file_end_time=$(date +%s)
            file_duration=$((file_end_time - file_start_time))
            log INFO "文件 $file_name 已合并并删除，耗时 $file_duration 秒。"
        else
            file_end_time=$(date +%s)
            file_duration=$((file_end_time - file_start_time))
            log ERROR "文件 $file_name 合并失败，未删除，耗时 $file_duration 秒。"
        fi
    else
        log WARN "文件 $file_name 不存在，跳过。"
    fi
done

# 结束合并的时间
end_time=$(date +%s)
total_duration=$((end_time - start_time))

log INFO "所有符合条件的文件已合并到 $output_file，并已删除原文件。总耗时 $total_duration 秒。"