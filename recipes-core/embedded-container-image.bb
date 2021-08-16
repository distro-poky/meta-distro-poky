require recipes-core/images/core/core-images-base.bb

DESCRIPTION = "Core image with embedded container images"

IMAGE_FSTYPES += "sqauashfs"

WKS_FILE = "embedded-container.wks"

IMAGE_FEATURES_append = " \
    debug-tweaks \
    post-install-loggin \
    read-only-rootf \
    ssh-server-dropbear \
    "

IMAGE_INSTALL_append = " \
    container-images \
    docker \
    "
