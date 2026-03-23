#!/bin/bash

# 打印刻录安全监控系统 Linux打包脚本
# 用于创建DEB和RPM软件包

set -e  # 遇到错误时停止执行

APP_NAME="printburn-monitor"
APP_VERSION="1.0.0"
APP_DESC="打印刻录安全监控系统 - 单机版"
APP_AUTHOR="Security Solutions Inc."
APP_EMAIL="support@example.com"

BUILD_DIR="build"
INSTALLER_DIR="installer/linux"
PACKAGE_DIR="${INSTALLER_DIR}/package"
OUTPUT_DIR="dist"

# 创建必要的目录
mkdir -p ${OUTPUT_DIR}
mkdir -p ${PACKAGE_DIR}/{DEBIAN,opt/usr/local/bin,usr/share/applications,opt/usr/local/lib/${APP_NAME}}

echo "开始构建Linux安装包..."

# 检查构建文件是否存在
if [ ! -f "${BUILD_DIR}/${APP_NAME}" ]; then
    echo "错误: 构建文件 ${BUILD_DIR}/${APP_NAME} 不存在"
    echo "请先运行构建命令: cmake --build ${BUILD_DIR}"
    exit 1
fi

echo "复制可执行文件..."
cp ${BUILD_DIR}/${APP_NAME} opt/usr/local/bin/
chmod +x opt/usr/local/bin/${APP_NAME}

echo "复制Qt库..."
# 在实际情况下，你需要复制所有必要的Qt库
# 这里只是一个示例
# cp -r /usr/lib/x86_64-linux-gnu/libQt6* opt/usr/local/lib/${APP_NAME}/

echo "创建桌面文件..."
cat > usr/share/applications/${APP_NAME}.desktop << EOF
[Desktop Entry]
Name=${APP_DESC}
Comment=打印刻录安全监控系统
Exec=/opt/usr/local/bin/${APP_NAME}
Icon=${APP_NAME}
Terminal=false
Type=Application
Categories=Utility;
EOF

# 创建DEBIAN控制文件
cat > DEBIAN/control << EOF
Package: ${APP_NAME}
Version: ${APP_VERSION}
Section: utils
Priority: optional
Architecture: amd64
Depends: qt6-qmake, qt6-base-dev-tools, libqt6core6t64, libqt6gui6t64, libqt6widgets6t64, libqt6sql6t64, libqt6network6t64
Maintainer: ${APP_AUTHOR} <${APP_EMAIL}>
Description: ${APP_DESC}
 打印刻录安全监控系统是一款专为企业环境设计的安全监控软件，
 用于管理和监控打印和刻录操作，确保数据安全和合规性。
 支持任务申请、审批、设备管理等功能。
EOF

# 创建postinst脚本
cat > DEBIAN/postinst << 'EOF'
#!/bin/bash
# 安装后的配置脚本
update-desktop-database || true
ldconfig
echo "安装完成!"
EOF

# 创建postrm脚本
cat > DEBIAN/postrm << 'EOF'
#!/bin/bash
# 卸载后的清理脚本
update-desktop-database || true
echo "卸载完成!"
EOF

chmod 755 DEBIAN/postinst
chmod 755 DEBIAN/postrm

echo "创建DEB包..."
dpkg-deb --build --root-owner-group ${PACKAGE_DIR} ${OUTPUT_DIR}/${APP_NAME}_${APP_VERSION}_amd64.deb

echo "DEB包创建完成: ${OUTPUT_DIR}/${APP_NAME}_${APP_VERSION}_amd64.deb"

# 如果系统有rpm-build工具，则创建RPM包
if command -v rpmbuild &> /dev/null; then
    echo "创建RPM包..."

    # 创建RPM规范文件
    cat > ${INSTALLER_DIR}/${APP_NAME}.spec << EOF
Name:           ${APP_NAME}
Version:        ${APP_VERSION}
Release:        1%{?dist}
Summary:        ${APP_DESC}

License:        Proprietary
BuildArch:      x86_64

Requires:       qt6-qtbase-devel, qt6-qtbase-private-devel

%description
${APP_DESC}
用于管理和监控打印和刻录操作，确保数据安全和合规性。
支持任务申请、审批、设备管理等功能。

%files
/opt/usr/local/bin/${APP_NAME}
/usr/share/applications/${APP_NAME}.desktop
/opt/usr/local/lib/${APP_NAME}/*

%pre
echo "开始安装 ${APP_NAME}..."

%post
update-desktop-database || true
ldconfig
echo "安装完成!"

%preun
echo "开始卸载 ${APP_NAME}..."

%postun
update-desktop-database || true
echo "卸载完成!"

%changelog
* $(date +"%a %b %d %Y") ${APP_AUTHOR} - ${APP_VERSION}-1
- Initial package
EOF

    # 创建RPM包
    rpmbuild -bb --define "_topdir $(pwd)/${INSTALLER_DIR}" ${INSTALLER_DIR}/${APP_NAME}.spec
    mv ${INSTALLER_DIR}/RPMS/x86_64/${APP_NAME}-${APP_VERSION}* ${OUTPUT_DIR}/ 2>/dev/null || true
    rm -rf ${INSTALLER_DIR}/BUILD ${INSTALLER_DIR}/BUILDROOT ${INSTALLER_DIR}/RPMS ${INSTALLER_DIR}/SOURCES ${INSTALLER_DIR}/SPECS ${INSTALLER_DIR}/SRPMS

    echo "RPM包创建完成 (如果rpmbuild可用)"
else
    echo "未找到rpmbuild命令，跳过RPM包创建"
fi

echo "Linux安装包创建完成，输出目录: ${OUTPUT_DIR}/"