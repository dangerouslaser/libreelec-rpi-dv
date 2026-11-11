# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026-present Team LibreELEC (https://libreelec.tv)

PKG_NAME="pi-dv-engine"
PKG_VERSION="0.5.0"
PKG_SHA256="5e256d79db91a16a7ba5705f76cef00841face6b8a0bc7ea412c81c61cd02306"
PKG_LICENSE="GPL-2.0-or-later"
PKG_SITE="https://github.com/dangerouslaser/libreelec-rpi-dv"
PKG_URL="https://github.com/dangerouslaser/libreelec-rpi-dv/releases/download/engine-${PKG_VERSION}/${PKG_NAME}-${PKG_VERSION}.tar.xz"
PKG_DEPENDS_TARGET="toolchain ffmpeg vulkan-headers vulkan-loader"
PKG_LONGDESC="Dolby Vision reconstruction for the Raspberry Pi 5 GPU (Vulkan), for Kodi."
PKG_TOOLCHAIN="cmake"

PKG_CMAKE_OPTS_TARGET="-DPI_BUILD_BENCH=OFF \
                       -DPI_SHARED=ON"

post_makeinstall_target() {
  rm -rf ${INSTALL}/usr/share/doc
}
