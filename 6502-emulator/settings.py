# General settings and emulator control


# Flags for program logic

SHOW_STATUS_INFORMATION = True
TRACK_BYTES             = True


# CPU flags

FLAG_N = 0x80										# N (negative) flag           1000 0000
FLAG_V = 0x40										# V (overflow) flag           0100 0000
FLAG_U = 0x20										# U (unused) flag             0010 0000
FLAG_B = 0x10										# B (break command) flag      0001 0000
FLAG_D = 0x08										# D (decimal mode) flag       0000 1000
FLAG_I = 0x04										# I (interrupt disable) flag  0000 0100
FLAG_Z = 0x02										# Z (zero) flag               0000 0010
FLAG_C = 0x01										# C (carry) flag              0000 0001


# System design

MEMORY_SIZE = 65536									# 64 KB memory

RAM_AREA      = (0x0000, 0x9FFF)
ZERO_PAGE     = (0x0000, 0x00FF)					# zero page for faster addressing
STACK_AREA    = (0x0100, 0x01FF)					# system stack
SYSTEM_VARS   = (0x0200, 0x02FF)					# memory for core system variables
GEN_USE1      = (0x0300, 0x03FF)					# general use chunk
SCREEN_MEM    = (0x0400, 0x07FF)					# screen memory chunk
GEN_USE2      = (0x0800, 0x9FFF)					# code / data memory
GENERAL_RAM1  = (0x1000, 0x1FFF)					# according to documentation
BASIC_ROM     = (0xA000, 0xBFFF)					# BASIC interpreter
GENERAL_RAM2  = (0xC000, 0xCFFF)					# according to documentation
IO_AREA       = (0xD000, 0xDFFF)					# I/O memory
VIC_RANGE     = (0xD000, 0xD3FF)					# Video Interface Chip registers
SID_RANGE     = (0xD400, 0xD7FF)					# Sound Interface Device registers
COLOR_RAM     = (0xD800, 0xDBFF)					# bitmap for color ram
CIA_1		  = (0xDC00, 0xDCFF)					# Complex Interface Adapter 1
CIA_2		  = (0xDD00, 0xDDFF)					# Complex Interface Adapter 2
CHARACTER_ROM = (0xD000, 0xDFFF)					# hidden memory, typically mapped into IO area
													#                unless specifically accessed
KERNAL_ROM    = (0xE000, 0xFFFF)					# BIOS functions


# Additional CPU settings

CLOCK_SPEED   = 1000000								# 1 MHz clock rate
