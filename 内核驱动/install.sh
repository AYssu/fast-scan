#!/bin/sh

# 获取当前内核版本
kernel_version=$(uname -r)
echo "当前内核版本: $kernel_version"

# 根据内核版本推荐驱动
case $kernel_version in
    5.10.*)
        recommended_driver="fscan-510.ko"
        ;;
    5.15.*)
        recommended_driver="fscan-515.ko"
        ;;
    6.1.*)
        recommended_driver="fscan-610.ko"
        ;;
    6.6.*)
        recommended_driver="fscan-660.ko"
        ;;
    *)
        echo "无法找到匹配的驱动程序。"
        exit 1
        ;;
esac

# 显示推荐的驱动程序
echo "推荐驱动程序: $recommended_driver"

# 显示菜单
echo "请选择操作："
echo "1. 安装驱动"
echo "2. 卸载驱动"
echo "3. 退出"

# 提示用户输入
echo -n "请输入选项 (1/2/3): "
read choice

# 根据用户选择执行操作
case $choice in
    1)
        echo "正在安装 $recommended_driver..."
        if insmod "$recommended_driver"; then
            echo "驱动安装成功！"
        else
            echo "驱动安装失败，请检查权限或驱动文件是否正确。"
        fi
        ;;
    2)
        echo "正在卸载 $recommended_driver..."
        if rmmod 'fscan'; then
            echo "驱动卸载成功！"
        else
            echo "驱动卸载失败，请检查驱动是否已加载。"
        fi
        ;;
    3)
        echo "退出程序。"
        exit 0
        ;;
    *)
        echo "无效选项，请重新运行脚本。"
        exit 1
        ;;
esac