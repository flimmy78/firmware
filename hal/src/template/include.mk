
# Define the prefix to this directory.
# Note: The name must be unique within this build and should be
#       based on the root of the project
TARGET_HAL_SRC_INC_PATH = $(TARGET_HAL_PATH)/src/$(PLATFORM_NAME)

INCLUDE_DIRS += $(TARGET_HAL_SRC_INC_PATH)


# if hal is used as a make dependency (linked) then add linker commands
ifneq (,$(findstring hal,$(MAKE_DEPENDENCIES)))

LDFLAGS += -Tlinker_$(PLATFORM_DEVICE_LC).ld
#LDFLAGS += -Tlinker_$(PLATFORM_DEVICE_LC)_dfu.ld
LDFLAGS += -L$(COMMON_BUILD)/linker/arm
LDFLAGS += --specs=nano.specs -lc -lnosys
LDFLAGS += -Wl,--defsym,__STACKSIZE__=400

USE_PRINTF_FLOAT ?= y
ifeq ("$(USE_PRINTF_FLOAT)","y")
LDFLAGS += -u _printf_float
endif
LDFLAGS += -Wl,-Map,$(TARGET_BASE).map

# assembler startup script
ASRC_STARTUP += $(COMMON_BUILD)/startup/arm/startup_$(PLATFORM_DEVICE_LC).S
ASFLAGS += -I$(COMMON_BUILD)/startup/arm
ASFLAGS +=  -Wa,--defsym -Wa,INTOROBOT_INIT_STARTUP=1
#
endif
