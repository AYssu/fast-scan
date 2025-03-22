#!/bin/bash

# 定义目标URL
URL="https://www.easyverify.top/"

# 使用curl发送请求并获取HTML内容
response=$(curl -s "$URL")

# 检查返回内容中是否包含特定的<title>标签
if echo "$response" | grep -q '<title>易验 Easy Verify</title>'; then
    echo "验证成功"
else
    echo "验证失败"
fi
