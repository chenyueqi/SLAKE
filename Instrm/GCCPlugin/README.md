This plugin instruments panic statement or record static analysis records

## To add this plugin to kernel
```
ln -s ./fengshui_plugin.c path_to_kernel/scripts/gcc-plugins/fengshui_plugin.c
```

add following to path\_to\_kernel/scripts/Makefile.gcc-plugins
```c
gcc-plugin-$(CONFIG_GCC_PLUGIN_FENGSHUI)  += fengshui_plugin.so
gcc-plugin-cflags-$(CONFIG_GCC_PLUGIN_FENGSHUI) += -DFENGSHUI_PLUGIN
```

add following to path\_to\_kernel/arch/Kconfig
```c
config GCC_PLUGIN_FENGSHUI
	bool "instrument panic or collect metadata for fengshui"
	depends on GCC_PLUGINS
	help
		If you say Y here, fengshui plugin works
```
