使用命令sudo insmod register.ko命令插入内核模块后，
	使用lsmod命令看看模块是否插入成功
	使用cat /proc/devices命令查看主设备号，这个在创建设备文件结点时候需要用到
	使用dmesg命令查看内核模块printk打印信息
	使用sudo rmmod register命令卸载内核模块