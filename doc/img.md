

To extract image 

https://github.com/sviehb/jefferson.git 
jefferson system.img -d  arvind1

or 

binwalk -e system.img 



/proc# cat /proc/mtd 
dev:    size   erasesize  name
mtd0: 00030000 00008000 "uboot"
mtd1: 00470000 00008000 "boot"
mtd2: 00b00000 00008000 "system"
mtd3: 00050000 00008000 "cfg"
mtd4: 00010000 00008000 "provisioning"


mkfs.jffs2 -o otasystem.img -r _system.img.extracted/jffs2-root/  -e 0x8000 -s 0xb00000  -n -l -X zlib 

                                         

#!/bin/sh

#rootfs
flash_eraseall /dev/mtd2
sync
flashcp -v /mnt/OTA/otasystem.img /dev/mtd2
sync
cp /tmp/Upgrade/upGradeFlag.system /configs/upGradeFlag
sync

reboot

