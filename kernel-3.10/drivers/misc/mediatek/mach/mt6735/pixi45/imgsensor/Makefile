
include $(srctree)/drivers/misc/mediatek/Makefile.custom

space := 
semicolon :="
kernel_imgsensor_list := $(strip $(subst $(semicolon), $(space), $(CONFIG_CUSTOM_KERNEL_IMGSENSOR)))

ifeq ($(filter ov5670_mipi_raw, $(kernel_imgsensor_list)),ov5670_mipi_raw)
obj-y   +=  ov5670_mipi_raw/
endif

ifeq ($(filter gc0329_yuv, $(kernel_imgsensor_list)),gc0329_yuv)
obj-y   +=  gc0329_yuv/
endif

ifeq ($(filter gc2145_yuv, $(kernel_imgsensor_list)),gc2145_yuv)
obj-y   +=  gc2145_yuv/
endif

ifeq ($(filter ov2686_yuv, $(kernel_imgsensor_list)),ov2686_yuv)
obj-y   +=  ov2686_yuv/
endif

