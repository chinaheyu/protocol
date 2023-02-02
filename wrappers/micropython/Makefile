# Location of top-level MicroPython directory
# MPY_DIR = ../../..
# Architecture to build for (x86, x64, armv7m, xtensa, xtensawin)
# ARCH = x64

# Name of module
MOD = py_protocol

# Source files (.c or .py)
SRC = py_protocol.c ../../src/protocol.c

# Definitions
CFLAGS += -I../../include -DPROTOCOL_USING_STDLIB=0 -DPROTOCOL_USING_STRING=0

# Include to get the rules for compiling and linking the module
include $(MPY_DIR)/py/dynruntime.mk
