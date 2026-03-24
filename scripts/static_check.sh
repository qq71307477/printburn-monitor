#!/bin/bash
#
# 静态代码检查脚本 - 分层验证
# 用于在无编译环境下发现潜在问题
#
# 使用方法: ./scripts/static_check.sh [--level 1|2|3|4]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 计数器
ERRORS=0
WARNINGS=0

# 输出函数
log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    ((ERRORS++))
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
    ((WARNINGS++))
}

log_success() {
    echo -e "${GREEN}[OK]${NC} $1"
}

log_info() {
    echo "[INFO] $1"
}

# ============================================
# Level 1: 语法层 - 头文件独立性检查
# ============================================
check_level_1() {
    echo ""
    echo "============================================"
    echo "Level 1: 语法层 - 头文件独立性检查"
    echo "============================================"
    echo ""

    local total_headers=0
    local headers_with_guards=0
    local missing_guards=()

    # 查找所有头文件
    while IFS= read -r -d '' header; do
        ((total_headers++))
        local filename=$(basename "$header")

        # 检查 include guards
        if grep -q "#ifndef" "$header" && grep -q "#define" "$header" && grep -q "#endif" "$header"; then
            ((headers_with_guards++))
        else
            missing_guards+=("$filename")
            log_warning "缺少 include guards: $header"
        fi

        # 检查是否有 using namespace std 在头文件中
        if grep -q "using namespace std" "$header"; then
            log_warning "头文件中使用 using namespace std: $header"
        fi

    done < <(find "$PROJECT_ROOT" -name "*.h" -print0)

    echo ""
    log_info "头文件总数: $total_headers"
    log_info "有 include guards: $headers_with_guards"

    if [ ${#missing_guards[@]} -eq 0 ]; then
        log_success "所有头文件都有 include guards"
    else
        log_warning "缺少 include guards 的文件数: ${#missing_guards[@]}"
    fi
}

# ============================================
# Level 2: 声明层 - 声明与定义匹配检查
# ============================================
check_level_2() {
    echo ""
    echo "============================================"
    echo "Level 2: 声明层 - 声明与定义匹配检查"
    echo "============================================"
    echo ""

    local classes_checked=0
    local static_issues=0

    # 检查静态成员定义
    while IFS= read -r -d '' header; do
        # 查找静态成员声明
        if grep -q "static.*;" "$header"; then
            local classname=$(basename "$header" .h)
            local class_lower=$(echo "$classname" | tr '[:upper:]' '[:lower:]')

            # 查找对应的 cpp 文件
            local cpp_file=$(find "$PROJECT_ROOT" -name "${classname}.cpp" -o -name "${class_lower}.cpp" 2>/dev/null | head -1)

            if [ -n "$cpp_file" ]; then
                # 提取静态成员名称
                local static_members=$(grep -oP 'static\s+\w+\s+\K\w+(?=\s*;)' "$header" 2>/dev/null || true)

                if [ -n "$static_members" ]; then
                    while IFS= read -r member; do
                        # 检查是否在 cpp 中定义
                        if ! grep -q "${classname}::${member}" "$cpp_file" 2>/dev/null; then
                            log_warning "静态成员可能未定义: ${classname}::${member} in $header"
                            ((static_issues++))
                        fi
                    done <<< "$static_members"
                fi
            fi
            ((classes_checked++))
        fi
    done < <(find "$PROJECT_ROOT" -name "*.h" -print0)

    echo ""
    log_info "检查的类数量: $classes_checked"

    if [ $static_issues -eq 0 ]; then
        log_success "未发现静态成员定义问题"
    else
        log_warning "潜在静态成员问题: $static_issues"
    fi
}

# ============================================
# Level 3: 依赖层 - 跨编译单元检查
# ============================================
check_level_3() {
    echo ""
    echo "============================================"
    echo "Level 3: 依赖层 - 跨编译单元检查"
    echo "============================================"
    echo ""

    # 检查插件接口一致性
    local plugin_interfaces=(
        "IClientPlugin"
        "ITaskPlugin"
        "IApprovalPlugin"
        "ISecurityPlugin"
    )

    log_info "检查插件接口..."

    for iface in "${plugin_interfaces[@]}"; do
        # 查找接口定义
        local iface_file=$(find "$PROJECT_ROOT/include" -name "*.h" -exec grep -l "class $iface" {} \; 2>/dev/null | head -1)

        if [ -n "$iface_file" ]; then
            log_success "找到接口定义: $iface in $iface_file"
        else
            log_warning "未找到接口定义: $iface"
        fi
    done

    # 检查 Qt 版本兼容性标记
    echo ""
    log_info "检查 Qt 版本兼容性..."

    local qt6_only=$(grep -r "Qt::" "$PROJECT_ROOT/src" --include="*.cpp" 2>/dev/null | grep -v "QString" | grep -v "QVariant" | head -5 || true)

    if [ -n "$qt6_only" ]; then
        log_warning "发现可能的 Qt6 专用语法 (Qt 5.12 可能不兼容):"
        echo "$qt6_only" | head -5
    else
        log_success "未发现 Qt 版本兼容性问题"
    fi

    # 检查 Qt::SkipEmptyParts (Qt 5.14+)
    if grep -rq "Qt::SkipEmptyParts" "$PROJECT_ROOT/src" 2>/dev/null; then
        log_error "发现 Qt::SkipEmptyParts (需要 Qt 5.14+)，应使用 QString::SkipEmptyParts"
    fi
}

# ============================================
# Level 4: 集成层 - 项目结构检查
# ============================================
check_level_4() {
    echo ""
    echo "============================================"
    echo "Level 4: 集成层 - 项目结构检查"
    echo "============================================"
    echo ""

    # 检查 CMakeLists.txt
    if [ -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
        log_success "CMakeLists.txt 存在"

        # 检查是否包含所有源文件
        local cpp_count=$(find "$PROJECT_ROOT/src" -name "*.cpp" 2>/dev/null | wc -l)
        local h_count=$(find "$PROJECT_ROOT/src" -name "*.h" 2>/dev/null | wc -l)
        log_info "源文件数: $cpp_count .cpp, $h_count .h"
    else
        log_error "缺少 CMakeLists.txt"
    fi

    # 检查测试文件
    if [ -d "$PROJECT_ROOT/tests" ]; then
        local test_count=$(find "$PROJECT_ROOT/tests" -name "*.cpp" 2>/dev/null | wc -l)
        log_info "测试文件数: $test_count"
    fi

    # 检查插件目录
    if [ -d "$PROJECT_ROOT/src/plugins" ]; then
        local plugins=$(ls -d "$PROJECT_ROOT/src/plugins"/*/ 2>/dev/null | wc -l)
        log_info "插件数量: $plugins"
        log_success "插件目录存在"
    fi
}

# ============================================
# 主程序
# ============================================
main() {
    local level=${1:-all}

    echo "============================================"
    echo "  静态代码检查 - 分层验证"
    echo "  项目: $PROJECT_ROOT"
    echo "  时间: $(date '+%Y-%m-%d %H:%M:%S')"
    echo "============================================"

    case "$level" in
        1)
            check_level_1
            ;;
        2)
            check_level_2
            ;;
        3)
            check_level_3
            ;;
        4)
            check_level_4
            ;;
        all)
            check_level_1
            check_level_2
            check_level_3
            check_level_4
            ;;
        *)
            echo "用法: $0 [--level 1|2|3|4|all]"
            exit 1
            ;;
    esac

    echo ""
    echo "============================================"
    echo "  检查完成"
    echo "  错误: $ERRORS"
    echo "  警告: $WARNINGS"
    echo "============================================"

    if [ $ERRORS -gt 0 ]; then
        exit 1
    fi
}

# 解析参数
LEVEL="all"
while [[ $# -gt 0 ]]; do
    case $1 in
        --level|-l)
            LEVEL="$2"
            shift 2
            ;;
        *)
            shift
            ;;
    esac
done

main "$LEVEL"