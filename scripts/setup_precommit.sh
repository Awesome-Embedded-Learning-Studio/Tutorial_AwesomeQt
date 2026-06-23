#!/usr/bin/env bash
# 一键安装本地提交前自检钩子。用法：pnpm hooks:install（或 bash scripts/setup_precommit.sh）
# 会清理可能存在的 core.hooksPath 冲突，再 pre-commit install。
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=============================="
echo " Pre-commit Hook Setup"
echo "=============================="
echo ""

if ! git -C "$PROJECT_ROOT" rev-parse --git-dir >/dev/null 2>&1; then
    echo -e "${RED}ERROR: 当前不在 Git 仓库里。${NC}"
    exit 1
fi

if ! command -v pre-commit >/dev/null 2>&1; then
    echo -e "${RED}ERROR: 需要先安装 pre-commit。${NC}"
    echo ""
    echo "例如："
    echo "  pipx install pre-commit"
    echo "  # 或：python3 -m pip install --user pre-commit"
    exit 1
fi

cd "$PROJECT_ROOT"

# 清掉可能存在的 core.hooksPath，让 .git/hooks/pre-commit 生效
current_hooks_path="$(git -C "$PROJECT_ROOT" config --get core.hooksPath || true)"
if [ -n "$current_hooks_path" ]; then
    echo -e "${YELLOW}清理 core.hooksPath=$current_hooks_path，改用 .git/hooks/pre-commit。${NC}"
    git -C "$PROJECT_ROOT" config --unset core.hooksPath
fi

pre-commit install --config "$PROJECT_ROOT/.pre-commit-config.yaml"

echo ""
echo -e "${GREEN}已安装 pre-commit 钩子。${NC}"
echo ""

# 检查钩子用到的外部工具
missing_tools=()
for tool in python3 clang-format; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        missing_tools+=("$tool")
    fi
done

if [ "${#missing_tools[@]}" -gt 0 ]; then
    echo -e "${YELLOW}缺少钩子用到的工具：${NC}"
    printf '  - %s\n' "${missing_tools[@]}"
    echo ""
    echo "提交对应类型文件前请先装好。"
else
    echo -e "${GREEN}检测到钩子工具：${NC}"
    echo "  - python3: $(python3 --version 2>&1)"
    echo "  - clang-format: $(clang-format --version 2>&1)"
fi

echo ""
echo "每次提交前会跑："
echo "  - markdownlint（文章格式，规则 .markdownlint.json）"
echo "  - clang-format（C/C++ 源码格式）"
echo "  - check-added-large-files / end-of-file-fixer / trailing-whitespace / check-yaml"
echo ""
echo "手动跑全部："
echo "  pre-commit run --all-files"
echo ""
echo "必要时跳过（仅紧急情况）："
echo "  git commit --no-verify -m 'message'"
