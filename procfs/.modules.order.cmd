cmd_/home/debian/Programming/bbb_kernel/procfs/modules.order := {   echo /home/debian/Programming/bbb_kernel/procfs/procfs_test.ko; :; } | awk '!x[$$0]++' - > /home/debian/Programming/bbb_kernel/procfs/modules.order
