DEPENDS += "virtual/fakeroot-native"

fakeroot do_install_append_class-native() {
    install -m 0755 grub-editenv "${D}${bindir}"
}

fakeroot do_install_append_class-target() {
    local menu="${WORKDIR}/boot-menu.inc"

    # Enable the default IMA rules if IMA is enabled and luks is disabled.
    # This is because unseal operation will fail when any PCR is extended
    # due to updating the aggregate integrity value by the default IMA rules.
    [ x"${IMA}" = x"1" -a x"${@bb.utils.contains('DISTRO_FEATURES', 'luks', '1', '0', d)}" != x"1" ] && {
        ! grep -q "ima_policy=tcb" "$menu" &&
            sed -i 's/^\s*linux\s\+.*bzImage.*/& ima_policy=tcb/g' "$menu"
    }

    # Replace the root parameter in boot command line with BOOT_CMD_ROOT,
    # which can be configured. It is helpful when secure boot is enabled.
    [ -n "${BOOT_CMD_ROOT}" ] && {
        sed -i "s,root=/dev/hda2,root=${BOOT_CMD_ROOT},g" "$menu"
    }

    # Install the stacked grub configs.
    install -d "${D}${EFI_BOOT_PATH}"
    install -m 0600 "${WORKDIR}/grub-efi.cfg" "${D}${EFI_BOOT_PATH}/grub.cfg"
    install -m 0600 "$menu" "${D}${EFI_BOOT_PATH}"
    [ x"${UEFI_SB}" = x"1" ] && {
        install -m 0600 "${WORKDIR}/efi-secure-boot.inc" "${D}${EFI_BOOT_PATH}"
        install -m 0600 "${WORKDIR}/password.inc" "${D}${EFI_BOOT_PATH}"
    }

    # Create the initial environment block with empty item.
    grub-editenv "${D}${EFI_BOOT_PATH}/grubenv" create

    install -d "${D}${EFI_BOOT_PATH}/${GRUB_TARGET}-efi"
    grub-mkimage -c ../cfg -p "${GRUB_PREFIX_DIR}" -d "./grub-core" \
        -O "${GRUB_TARGET}-efi" -o "${B}/${GRUB_IMAGE}" \
        ${GRUB_BUILDIN}

    install -m 0644 "${B}/${GRUB_IMAGE}" "${D}${EFI_BOOT_PATH}/${GRUB_IMAGE}"

    # Install the modules to grub-efi's search path
    make -C grub-core install DESTDIR="${D}${EFI_BOOT_PATH}" pkglibdir=""

    # Remove .module
    rm -f ${D}${EFI_BOOT_PATH}/${GRUB_TARGET}-efi/*.module
}

fakeroot python do_sign_class-target() {
    image_dir = d.getVar('D', True)
    efi_boot_path = d.getVar('EFI_BOOT_PATH', True)
    grub_image = d.getVar('GRUB_IMAGE', True)
    dir = image_dir + efi_boot_path + '/'

    sb_sign(dir + grub_image, dir + grub_image, d)
    uks_bl_sign(dir + 'grub.cfg', d)
    uks_bl_sign(dir + 'boot-menu.inc', d)

    if d.getVar('UEFI_SB', True) == "1":
        uks_bl_sign(dir + 'efi-secure-boot.inc', d)
        uks_bl_sign(dir + 'password.inc', d)
}

fakeroot python do_sign() {
}
deltask sign
addtask sign after do_install before do_deploy do_package

fakeroot do_chownboot() {
    if [ "${GRUB_SIGN_VERIFY}" = "1" ]; then
        chown root:root -R "${D}${EFI_BOOT_PATH}/grub.cfg${SB_FILE_EXT}"
        chown root:root -R "${D}${EFI_BOOT_PATH}/boot-menu.inc${SB_FILE_EXT}"
        [ x"${UEFI_SB}" = x"1" ] && {
            chown root:root -R "${D}${EFI_BOOT_PATH}/efi-secure-boot.inc${SB_FILE_EXT}"
            chown root:root -R "${D}${EFI_BOOT_PATH}/password.inc${SB_FILE_EXT}"
        }
    fi
}
deltask chownboot
addtask chownboot after do_deploy before do_package

# Append the do_deploy() in oe-core.
fakeroot do_deploy_append_class-target() {
    install -m 0644 "${D}${EFI_BOOT_PATH}/${GRUB_IMAGE}" "${DEPLOYDIR}"
    install -m 0664 "${DEPLOYDIR}/${GRUB_IMAGE}" "${DEPLOYDIR}/${GRUB_IMAGE_PREFIX}${GRUB_IMAGE}"

    # Deploy the stacked grub configs.
    install -m 0600 "${D}${EFI_BOOT_PATH}/grubenv" "${DEPLOYDIR}"
    install -m 0600 "${D}${EFI_BOOT_PATH}/grub.cfg" "${DEPLOYDIR}"
    install -m 0600 "${D}${EFI_BOOT_PATH}/boot-menu.inc" "${DEPLOYDIR}"
    if [ "${GRUB_SIGN_VERIFY}" = "1" ]; then
        install -m 0600 "${D}${EFI_BOOT_PATH}/grub.cfg${SB_FILE_EXT}" "${DEPLOYDIR}"
        install -m 0600 "${D}${EFI_BOOT_PATH}/boot-menu.inc${SB_FILE_EXT}" "${DEPLOYDIR}"
    fi
    [ x"${UEFI_SB}" = x"1" ] && {
        install -m 0600 "${D}${EFI_BOOT_PATH}/efi-secure-boot.inc" "${DEPLOYDIR}"
        install -m 0600 "${D}${EFI_BOOT_PATH}/password.inc" "${DEPLOYDIR}"
    }

    if [ "${UEFI_SB}" = "1" ] && [ "${GRUB_SIGN_VERIFY}" = "1" ]; then
        install -m 0600 "${D}${EFI_BOOT_PATH}/efi-secure-boot.inc${SB_FILE_EXT}" "${DEPLOYDIR}"
        install -m 0600 "${D}${EFI_BOOT_PATH}/password.inc${SB_FILE_EXT}" "${DEPLOYDIR}"
    fi

    install -d "${DEPLOYDIR}/efi-unsigned"
    install -m 0644 "${B}/${GRUB_IMAGE}" "${DEPLOYDIR}/efi-unsigned"
    install -m 0644 "${B}/${GRUB_IMAGE}" "${DEPLOYDIR}/efi-unsigned/${GRUB_IMAGE_PREFIX}${GRUB_IMAGE}"
    cp -af "${D}${EFI_BOOT_PATH}/${GRUB_TARGET}-efi" "${DEPLOYDIR}/efi-unsigned"
}

CONFFILES_${PN} += "\
    ${EFI_BOOT_PATH}/grub.cfg* \
    ${EFI_BOOT_PATH}/grubenv* \
    ${EFI_BOOT_PATH}/boot-menu.inc* \
    ${EFI_BOOT_PATH}/efi-secure-boot.inc* \
"
