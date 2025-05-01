# Outline of a
# SIMPLE 6502 EMULATOR
# ====================
#
# Complete opcode overview at https://www.pagetable.com/c64ref/6502/?tab=2
#
# CPU implementation
# ------------------
# all registers and flags implemented
# cycle counting implemented
# decimal mode, stack, and interrupt routines not implemented
#
# Opcode implementation table
# ---------------------------
# $00  BRK           works
# $81  STA ($vw,X)   under construction
# $85  STA  $vw      works
# $8D  STA  $vwxy    under construction
# $91  STA ($xy),Y   under construction
# $95  STA  $vx,X    works
# $99  STA  $vwxy,Y  under construction
# $9D  STA  $vwxy,X  under construction
# $A0  LDY #$xy      works
# $A1  LDA ($xy,X)   works
# $A2  LDX #$xy      works
# $A4  LDY  $xy      works
# $A5  LDA  $xy      works
# $A6  LDX  $xy      works
# $A9  LDA #$xy      works
# $AC  LDY  $vwxy    works
# $AD  LDA  $vwxy    works
# $AE  LDX  $vwxy    works
# $B1  LDA ($xy),Y   works
# $B4  LDY  $xy,X    works
# $B5  LDA  $xy,X    works
# $B6  LDX  $xy,Y    works
# $B9  LDA  $vwxy,Y  works
# $BC  LDY  $vwxy,X  works
# $BD  LDA  $vwxy,X  works
# $BE  LDX  $vwxy,Y  works
#
# Next opcodes to be implemented
# ------------------------------
# $84  STY  $xy      under construction
# $86  STX  $vw      under construction
# $8C  STY  $vwxy    under construction
# $8E  STX  $vwxy    under construction
# $94  STY  $vw,X    under construction
# $96  STX  $vw,Y    under construction
#
# Next features to be implemented
# -------------------------------
# basic debugging (set breakpoint/s)
# read program data from binary file
# implementing the top-down 6502 stack (no idea how to do that in Python)
# differentiate RAM and ROM parts of memory
# what about paging?
# what about interrupts? (I dread them) :)
#
# Checks: do zeropage addresses properly wrap around?
# ------  when should flags be cleared?
#
# Illusionary stuff
# -----------------
# A full emulator would possibly use a "class Memory" instead of the array,
# and for a whole system, every computer chip would be one class.
# It would get really interesting (and complex) if the data and address bus systems would be emulated as well.
# Right now I don't know enough about the 6502 architecture for this.


import cc6502										# import dictionary of cycle counts
import settings

class CPU6502:
	def __init__(self):								# CPU initialization
		self.reset()

	def reset(self):									# simulate hard reset, set CPU to initial values
		self.A  = 0x00
		self.X  = 0x00
		self.Y  = 0x00
		self.SP = 0xFD
		self.PC = 0xFFFC
		self.SR = 0x24

		self.cc = 0									# reset cycle counter

	def get_byte(self, memory):						# get byte from PC location and move PC
		byte = memory[self.PC]
		if settings.TRACK_BYTES:
			print(f"{byte:02X} ", end='')
		self.PC += 1
		if self.PC >= settings.MEMORY_SIZE:			# manually wrap memory around
			self.PC = 0
		return byte

	def check_flag(self, flag):						# check if a flag has been set
		return (self.SR & flag) != 0

	def update_flag(self, flag, on_or_off):			# update flag
		if on_or_off:
			self.SR |= flag
		else:
			self.SR &= ~flag

	def lda(self, memory, opcode):
		if opcode == 0xA9:							# LDA #$xy
			self.A = self.get_byte(memory)
		elif opcode == 0xA5:						# LDA $xy
			self.A = memory[self.get_byte(memory)]
		elif opcode == 0xAD:						# LDA $vwxy
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.A = memory[addr]
		elif opcode == 0xA1:						# LDA ($xy,X)
			temp_addr = (self.get_byte(memory) + self.X) & 0xFF
			addr = memory[temp_addr] | (memory[(temp_addr + 1) & 0xFF] << 8)
			self.A = memory[addr]
		elif opcode == 0xB1:						# LDA ($xy),Y
			temp_addr = self.get_byte(memory)
			addr = memory[temp_addr] | (memory[(temp_addr + 1) & 0xFF] << 8)
			self.A = memory[(addr + self.Y) & 0xFFFF]
			if ((addr + self.Y) & 0xFF00) != (addr & 0xFF00):	# page crossing needs one additional cycle
				self.cc += 1
		elif opcode == 0xB5:						# LDA $xy,X
			self.A = memory[(self.get_byte(memory) + self.X) & 0xFF]
		elif opcode == 0xB9:						# LDA $vwxy,Y
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.A = memory[(addr + self.Y) & 0xFFFF]
			if ((addr + self.Y) & 0xFF00) != (addr & 0xFF00):	# page crossing needs one additional cycle
				self.cc += 1
		elif opcode == 0xBD:						# LDA $vwxy,X
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.A = memory[(addr + self.X) & 0xFFFF]
			if ((addr + self.X) & 0xFF00) != (addr & 0xFF00):	# page crossing needs one additional cycle
				self.cc += 1


		self.update_flag(settings.FLAG_Z, self.A == 0)		# update Z if A is 0
		self.update_flag(settings.FLAG_N, self.A & 0x80)

	def ldx(self, memory, opcode):
		if opcode == 0xA2:							# LDX #$xy
			self.X = self.get_byte(memory)
		elif opcode == 0xA6:						# LDX $xy
			self.X = memory[self.get_byte(memory)]
		elif opcode == 0xAE:						# LDX $vwxy
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.X = memory[addr]
		elif opcode == 0xB6:						# LDX $xy,Y
			self.X = memory[(self.get_byte(memory) + self.Y) & 0xFF]
		elif opcode == 0xBE:						# LDX $vwxy,Y
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.X = memory[(addr + self.Y) & 0xFFFF]

		self.update_flag(settings.FLAG_Z, self.X == 0)
		self.update_flag(settings.FLAG_N, self.X & 0x80)

	def ldy(self, memory, opcode):
		if opcode == 0xA0:							# LDY #$xy
			self.Y = self.get_byte(memory)
		elif opcode == 0xA4:						# LDY $xy
			self.Y = memory[self.get_byte(memory)]
		elif opcode == 0xAC:						# LDY $vwxy
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.Y = memory[addr]
		elif opcode == 0xB4:						# LDY $xy,X
			self.Y = memory[(self.get_byte(memory) + self.X) & 0xFF]
		elif opcode == 0xBC:						# LDY $vwxy,X
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			self.Y = memory[(addr + self.X) & 0xFFFF]

		self.update_flag(settings.FLAG_Z, self.Y == 0)
		self.update_flag(settings.FLAG_N, self.Y & 0x80)

	def sta(self, memory, opcode):
		if opcode == 0x81:							# STA ($vw,X)
			temp_addr = (self.get_byte(memory) + self.X) & 0xFF
			addr = memory[temp_addr] | (memory[(temp_addr + 1) & 0xFF] << 8)
			memory[addr] = self.A
		elif opcode == 0x85:						# STA $vw
			memory[self.get_byte(memory)] = self.A
		elif opcode == 0x8D:						# STA $vwxy
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			memory[addr] = self.A
		elif opcode == 0x91:						# STA ($vw),Y
			temp_addr = self.get_byte(memory)
			addr = memory[temp_addr] | (memory[(temp_addr + 1) & 0xFF] << 8)
			memory[(addr + self.Y) & 0xFFFF] = self.A
		elif opcode == 0x95:						# STA $vx,X
			memory[(self.get_byte(memory) + self.X) & 0xFF] = self.A
		elif opcode == 0x99:						# STA $vwxy,Y
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			memory[(addr + self.Y) & 0xFFFF] = self.A
		elif opcode == 0x9D:						# STA $vwxy,X
			addr = self.get_byte(memory) | (self.get_byte(memory) << 8)
			memory[(addr + self.X) & 0xFFFF] = self.A

	def brk(self):
		self.update_flag(settings.FLAG_B, True)		# BRK sets B flag on

	def execute_command(self, memory):
		if settings.TRACK_BYTES:
			print(f".{self.PC:04X}  ", end='')		# status output
		opcode = self.get_byte(memory)
		if opcode in cc6502.cycle_counts:
			self.cc += cc6502.cycle_counts[opcode]
		if opcode in [0xA9, 0xA5, 0xAD, 0xA1, 0xB1, 0xB5, 0xB9, 0xBD]:	# LDA
			self.lda(memory, opcode)
		elif opcode in [0xA2, 0xA6, 0xAE, 0xB6, 0xBE]:					# LDX
			self.ldx(memory, opcode)
		elif opcode in [0xA0, 0xA4, 0xAC, 0xB4, 0xBC]:					# LDY
			self.ldy(memory, opcode)
		elif opcode in [0x81, 0x85, 0x8D, 0x91, 0x95, 0x99, 0x9D]:		# STA
			self.sta(memory, opcode)
		elif opcode == 0x00:											# BRK
			self.brk()
		else:
			print(f"Unknown opcode {opcode:02X}.")
		if settings.TRACK_BYTES:
			print()

	def status(self):
		print(f" A: {self.A:02X}  |   X: {self.X:02X}  |   Y: {self.Y:02X}    |  NV-BDIZC  |  CYCLE COUNT")
		# typecasting to int to get 1/0 from True/False
		print(f"SP: {self.SP:02X}  |  SR: {self.SR:02X}  |  PC: {self.PC:04X}  |  " +
			f"{int(self.check_flag(settings.FLAG_N))}{int(self.check_flag(settings.FLAG_V))}{int(self.check_flag(settings.FLAG_U))}{int(self.check_flag(settings.FLAG_B))}{int(self.check_flag(settings.FLAG_D))}{int(self.check_flag(settings.FLAG_I))}{int(self.check_flag(settings.FLAG_Z))}{int(self.check_flag(settings.FLAG_C))}  |" +
			f"  {self.cc:>11}\n"
		)


def enter_code(memory):
	memory[0xFFFC] = 0xA9												# LDA #$FF
	memory[0xFFFD] = 0xFF
	memory[0xFFFE] = 0xA5												# LDA $22
	memory[0xFFFF] = 0x22
	memory[0x0000] = 0xAD												# LDA $1234
	memory[0x0001] = 0x34
	memory[0x0002] = 0x12
	memory[0x0003] = 0xA2												# LDX #$05
	memory[0x0004] = 0x05
	memory[0x0005] = 0xBD												# LDA $1234,X
	memory[0x0006] = 0x34
	memory[0x0007] = 0x12
	memory[0x0008] = 0xB5												# LDA $FF,X
	memory[0x0009] = 0xFF
	memory[0x000A] = 0xA1												# LDA ($02,X)
	memory[0x000B] = 0x02
	memory[0x000C] = 0xA0												# LDY #$03
	memory[0x000D] = 0x03
	memory[0x000E] = 0xB9												# LDA $3456,Y
	memory[0x000F] = 0x56
	memory[0x0010] = 0x34
	memory[0x0011] = 0xB1												# LDA $05,Y
	memory[0x0012] = 0x05
	memory[0x0013] = 0xA6												# LDX $00
	memory[0x0014] = 0x00
	memory[0x0015] = 0xAE												# LDX $1234
	memory[0x0016] = 0x34
	memory[0x0017] = 0x12
	memory[0x0018] = 0xB6												# LDX $05,Y
	memory[0x0019] = 0x05
	memory[0x001A] = 0xBE												# LDX $1231,Y
	memory[0x001B] = 0x31
	memory[0x001C] = 0x12
	memory[0x001D] = 0xA4												# LDY $03
	memory[0x001E] = 0x03
	memory[0x001F] = 0xAC												# LDY $1234
	memory[0x0020] = 0x34
	memory[0x0021] = 0x12
	memory[0x0022] = 0xB4												# LDY $01,X
	memory[0x0023] = 0x01
	memory[0x0024] = 0xBC												# LDY $46CD,X
	memory[0x0025] = 0xCD
	memory[0x0026] = 0x46
	memory[0x0027] = 0x95												# STA $AA,X
	memory[0x0028] = 0xAA
	memory[0x0029] = 0x00												# BRK

	# Data initialization
	memory[0x0022] = 0xB4
	memory[0x0045] = 0x42
	memory[0x00AD] = 0xAA
	memory[0x00C0] = 0x11
	memory[0x00C1] = 0x47
	memory[0x1234] = 0x44
	memory[0x1239] = 0x93
	memory[0x3459] = 0x99
	memory[0x4711] = 0xDA
	memory[0xB512] = 0x77

	print("This code will test LDA, LDX, and LDY commands:")		# I would need to add a disassembler to convert the
	print(".FFFC  A9 FF     LDA #$FF")								# opcodes in memory directly to mnemonics
	print(".FFFE  A5 22     LDA  $22")
	print(".0000  AD 34 12  LDA  $1234")
	print(".0003  A2 05     LDX #$05")
	print(".0005  BD 34 12  LDA  $1234,X")
	print(".0008  B5 FF     LDA  $FF,X")
	print(".000A  A1 02     LDA ($02,X)")
	print(".000C  A0 03     LDY #$03")
	print(".000E  B9 56 34  LDA  $3456,Y")
	print(".0011  B1 05     LDA ($05),Y")
	print(".0013  A6 00     LDX  $00")
	print(".0015  AE 34 12  LDX  $1234")
	print(".0018  B6 05     LDX  $05,Y")
	print(".001A  BE 31 12  LDX  $1231,Y")
	print(".001D  A4 03     LDY  $03")
	print(".001F  AC 34 12  LDY  $1234")
	print(".0022  B4 00     LDY  $01,X")
	print(".0024  BC CD 46  LDY  $46CD,X")
	print(".0027  95 AA     STA  $AA,X")
	print(".0029  00        BRK")
	print()
	
	
def show_memory_dump(start, end, memory):
	print(f"       Memory dump from {start:04X} to {end:04X}")
	location = start
	linebreak_counter = 0
	
	while True:
		if linebreak_counter % 10 == 0 or location == start:
			print(f"\n.{location:04X}  ", end='')
			linebreak_counter = 0
		print(f"{memory[location % settings.MEMORY_SIZE]:02X} ", end='')
		
		if location == end:
			print("\n")
			return
		
		location = (location + 1) % settings.MEMORY_SIZE
		linebreak_counter += 1
		

# MAIN FUNCTION

print("SIMPLE\n\n    ###   #####    ####    #####\n   ##  #  ##      ##  ##  #    ##\n  ##      ##      ##  ##       ##\n  #####   #####   ##  ##      ##\n  ##  ##      ##  ##  ##     ##\n  ##  ##  #   ##  ##  ##    ##  #\n   ####    ####    ####   #######\n\n                           EMULATOR\n\n")

cpu = CPU6502()															# initialize CPU
memory = [0] * settings.MEMORY_SIZE										# initialize memory and fill it with 0s

cpu.reset()
print("Initial CPU status after reset:")
cpu.status()

enter_code(memory)														# write some 6502 code into the memory

while not cpu.check_flag(settings.FLAG_B):								# execute as long as B flag has not been set
	cpu.execute_command(memory)											# do something
	if settings.SHOW_STATUS_INFORMATION:
		cpu.status()

print(59 * "=" + "\n")
print("B flag has been set, program terminated.  Final CPU status:")
cpu.status()

show_memory_dump(0xFFFC, 0x0029, memory)

print(f"On a 1 MHz 6502, this code would have take approximately {cpu.cc} µs to run ({cpu.cc / 1000} ms).")
