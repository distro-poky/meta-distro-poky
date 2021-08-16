# The original recipe does not sufficently use the fakeroot notation.
# meta-secure-core/meta-efi-secure-boot/recipes-bsp/grub/grub-efi*

require ${@bb.utils.contains('DISTRO_FEATURES', 'efi-secure-boot', 'grub-efi-efi-secure-boot-fakeroot.inc', '', d)}
