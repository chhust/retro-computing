# 6502 Cycle Counts
#
# The 6502 needs several cycles per opcode. This varies per opcode and per situation.
#
# Cycle counts are taken from the 6502 documentation.
#
# Basic cycle management is done via a dictionary (opcodes: cycles).
# Page crosses and branching conditions will need extra coding.
#
# Basically, this might even allow synchronization to the actual 6502 speed (if the actual computer is fast enough).
# I would need to be able to stop the computer after each command until the cycles pass.

cycle_counts = {
	0x00: 7,	# BRK
	0x81: 6,	# STA Indirect, X
	0x85: 3,	# STA Zero Page
	0x8D: 4,	# STA Absolute
	0x91: 6,	# STA Indirect, Y
	0x95: 4,	# STA Zero Page, X
	0x99: 5,	# STA Absolute, Y
	0x9D: 5,	# STA Absolute, X
	0xA0: 2,	# LDY Immediate
	0xA1: 6,	# LDA Indirect, X
	0xA2: 2,	# LDX Immediate
	0xA4: 3,	# LDY Zero Page
	0xA5: 3,	# LDA Zero Page
	0xA6: 3,	# LDX Zero Page
	0xA9: 2,	# LDA Immediate
	0xAC: 4,	# LDY Absolute
	0xAD: 4,	# LDA Absolute
	0xAE: 4,	# LDX Absolute
	0xB1: 5,	# LDA Indirect, Y (+1 if page crossed)					+1 implemented
	0xB4: 4,	# LDY Zero Page, X
	0xB5: 4,	# LDA Zero Page, X
	0xB6: 4,	# LDX Zero Page, Y
	0xB9: 4,	# LDA Absolute, Y (+1 if page crossed)					+1 implemented
	0xBC: 4,	# LDY Absolute, X (+1 if page crossed)					+1 not yet implemented
	0xBD: 4,	# LDA Absolute, X (+1 if page crossed)					+1 implemented
	0xBE: 4		# LDX Absolute, Y (+1 if page crossed)					+1 not yet implemented
}
