#!/bin/bash
#
# Qt 6.9.1 源码下载脚本 (Linux/macOS)
# 用途: 下载Qt 6.9.1源码并解压到qt_src/qt6.9.1目录
#

set -e

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 配置
QT_VERSION="6.9.1"
QT_MAJOR="6.9"  # URL路径中的主版本号
QT_FILE="qt-everywhere-src-${QT_VERSION}.tar.xz"
QT_URL="https://mirrors.aliyun.com/qt/official_releases/qt/${QT_MAJOR}/${QT_VERSION}/single/${QT_FILE}"
# 备用官方源
QT_URL_OFFICIAL="https://download.qt.io/official_releases/qt/${QT_MAJOR}/${QT_VERSION}/single/${QT_FILE}"
DEST_DIR="qt6.9.1"

# 获取脚本所在目录
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Qt ${QT_VERSION} 源码下载脚本${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# 检查是否已存在
if [ -d "$DEST_DIR" ]; then
    echo -e "${YELLOW}警告: $DEST_DIR 目录已存在${NC}"
    read -p "是否删除并重新下载? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo -e "${YELLOW}删除现有目录...${NC}"
        rm -rf "$DEST_DIR"
    else
        echo -e "${YELLOW}取消操作${NC}"
        exit 0
    fi
fi

# 下载源码
echo -e "${GREEN}[1/3] 下载 Qt ${QT_VERSION} 源码...${NC}"
echo "URL: $QT_URL"

if command -v wget &> /dev/null; then
    wget --progress=bar:force -O "$QT_FILE" "$QT_URL" || {
        echo -e "${YELLOW}阿里云镜像下载失败，尝试官方源...${NC}"
        wget --progress=bar:force -O "$QT_FILE" "$QT_URL_OFFICIAL"
    }
elif command -v curl &> /dev/null; then
    curl -L -o "$QT_FILE" "$QT_URL" || {
        echo -e "${YELLOW}阿里云镜像下载失败，尝试官方源...${NC}"
        curl -L -o "$QT_FILE" "$QT_URL_OFFICIAL"
    }
else
    echo -e "${RED}错误: 未找到 wget 或 curl 命令${NC}"
    exit 1
fi

# 验证文件
echo -e "${GREEN}[2/3] 验证下载文件...${NC}"
if [ ! -f "$QT_FILE" ]; then
    echo -e "${RED}错误: 下载文件不存在${NC}"
    exit 1
fi

FILE_SIZE=$(du -h "$QT_FILE" | cut -f1)
echo "文件大小: $FILE_SIZE"

# 解压
echo -e "${GREEN}[3/3] 解压源码...${NC}"
if command -v tar &> /dev/null; then
    tar -xf "$QT_FILE"
    # 解压后目录名是 qt-everywhere-src-6.9.1，重命名为 qt6.9.1
    if [ -d "qt-everywhere-src-${QT_VERSION}" ]; then
        mv "qt-everywhere-src-${QT_VERSION}" "$DEST_DIR"
    fi
else
    echo -e "${RED}错误: 未找到 tar 命令${NC}"
    exit 1
fi

# 清理压缩包
read -p "是否删除压缩包 ${QT_FILE}? (y/N): " -n 1 -r
echo
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -f "$QT_FILE"
    echo -e "${GREEN}已删除压缩包${NC}"
fi

# 显示结果
echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  下载完成!${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "目录: ${SCRIPT_DIR}/${DEST_DIR}"
echo ""

# 显示模块列表
if [ -d "$DEST_DIR" ]; then
    echo -e "${GREEN}包含的模块 (前15个):${NC}"
    ls -1 "$DEST_DIR" | head -15
    TOTAL=$(ls -1 "$DEST_DIR" | wc -l)
    if [ "$TOTAL" -gt 15 ]; then
        echo "  ... (共 $TOTAL 个模块/目录)"
    fi
fi
