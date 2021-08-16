DESCRIPTION = ""
SECTION = "package"
DEPENDS = ""
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://LICENSE;md5=96af5705d6f64a88e035781ef00e98a8"
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"

SRCREV = "${AUTOREV}"
SRC_URI = "gitsm://github.com/coreos/rpm-ostree.git;protocol=http;branch=main"
PV = "1.0+git${SRCPV}"

S = "${WORKDIR}/git/"

inherit autotools
