#!/bin/bash

# 定义ndk路径列表
ndk_paths=(
    "/data/user/0/aidepro.top/no_backup/ndksupport-1710240003/android-ndk-aide/ndk-build"
    "/data/data/com.termux/files/home/android-ndk-r21e/ndk-build"
)

# 遍历ndk路径列表
for ndk_path in "${ndk_paths[@]}"; do
    # 判断文件是否存在
    if [ -f "$ndk_path" ]; then
        echo "找到ndk-build路径：$ndk_path"
        # 执行ndk-build命令
        "$ndk_path"
        exit 0
    fi
done

# 如果所有路径都不存在，提示用户并退出
echo "未找到ndk-build文件，请先配置ndk路径"
exit 1