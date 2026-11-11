# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2026-present Team LibreELEC (https://libreelec.tv)

PKG_NAME="pi-overclock"
PKG_VERSION="1.0"
PKG_LICENSE="GPL-2.0-or-later"
PKG_SITE="https://libreelec.tv"
PKG_URL=""
PKG_DEPENDS_TARGET="toolchain Python3 stress-ng ${OPENGLES}"
PKG_LONGDESC="pi-overclock: CPU and GPU overclocking for the Raspberry Pi 5, tried in a tryboot boot and tested before it is kept"
PKG_TOOLCHAIN="manual"

make_target() {
  ${CC} ${CFLAGS} -o pi-overclock-gpuload ${PKG_DIR}/src/gpuload.c ${LDFLAGS} -lEGL -lGLESv2 -lgbm
}

makeinstall_target() {
  mkdir -p ${INSTALL}/usr/bin
    cp ${PKG_DIR}/scripts/pi-overclock ${INSTALL}/usr/bin
    chmod 0755 ${INSTALL}/usr/bin/pi-overclock
  mkdir -p ${INSTALL}/usr/lib/pi-overclock
    cp pi-overclock-gpuload ${INSTALL}/usr/lib/pi-overclock
}

post_install() {
  enable_service overclock-bootguard.service
  enable_service overclock-trial.service
  enable_service overclock-bootok.timer
}
