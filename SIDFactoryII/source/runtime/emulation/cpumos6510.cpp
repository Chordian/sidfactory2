#include "cpumos6510.h"

namespace Emulation
{
	#pragma warning(disable: 4100)

	//------------------------------------------------------------------------------------------------------------------------------
	// Instruction matrix
	//------------------------------------------------------------------------------------------------------------------------------
	CPUmos6510::Instruction CPUmos6510::ms_aInstructions[0x100] =
	{
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x00
		{ ORA, izx, 2, 6, "ORA", am_IZX},		// 0x01
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x02 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x03 SLO izx, 8
		{ BRK, imp, 1, 3, "BRK", am_IMP},		// 0x04 NOP zp , 3
		{ ORA, zp , 2, 3, "ORA", am_ZP },		// 0x05 
		{ ASL, zp , 2, 5, "ASL", am_ZP },		// 0x06
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x07 SLO zp , 5
		{ PHP, imp, 1, 3, "PHP", am_IMP},		// 0x08 
		{ ORA, imm, 2, 2, "ORA", am_IMM},		// 0x09
		{ ASL, imp, 1, 2, "ASL", am_IMP},		// 0x0a
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x0b ANC imm, 2
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x0c NOP abs, 4
		{ ORA, abs, 3, 4, "ORA", am_ABS},		// 0x0d
		{ ASL, abs, 3, 6, "ASL", am_ABS},		// 0x0e
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x0f SLO abs, 6

		{ BPL, rel, 2, 2, "BPL", am_REL},		// 0x10
		{ ORA, izy, 2, 5, "ORA", am_IZY},		// 0x11
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x12 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x13 SLO izy, 8
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x14 NOP zpx, 4
		{ ORA, zpx, 2, 4, "ORA", am_ZPX},		// 0x15
		{ ASL, zpx, 2, 6, "ASL", am_ZPX},		// 0x16
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x17 SLO zpx, 6
		{ CLC, imp, 1, 2, "CLC", am_IMP},		// 0x18
		{ ORA, aby, 3, 4, "ORA", am_ABY},		// 0x19
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x1a NOP imp, 2
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x1b SLO aby, 7
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x1c NOP abx, 4
		{ ORA, abx, 3, 4, "ORA", am_ABX},		// 0x1d
		{ ASL, abx, 3, 7, "ASL", am_ABX},		// 0x1e
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x1f SLO abx, 7

		{ JSR, abs, 3, 6, "JSR", am_ABS},		// 0x20
		{ AND, izx, 2, 6, "AND", am_IZX},		// 0x21
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x22 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x23 RLA izx, 8
		{ BIT, zp , 2, 3, "BIT", am_ZP },		// 0x24 
		{ AND, zp , 2, 3, "AND", am_ZP },		// 0x25
		{ ROL, zp , 2, 5, "ROL", am_ZP },		// 0x26
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x27 RLA zp , 5
		{ PLP, imp, 1, 4, "PLP", am_IMP},		// 0x28 
		{ AND, imm, 2, 2, "AND", am_IMM},		// 0x29
		{ ROL, imp, 1, 2, "ROL", am_IMP},		// 0x2a
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x2b ANC imm, 2
		{ BIT, abs, 3, 4, "BIT", am_ABS},		// 0x2c
		{ AND, abs, 3, 4, "AND", am_ABS},		// 0x2d
		{ ROL, abs, 3, 6, "ROL", am_ABS},		// 0x2e
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x2f RLA abs, 6

		{ BMI, rel, 2, 2, "BMI", am_REL},		// 0x30
		{ AND, izy, 2, 5, "AND", am_IZY},		// 0x31
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x32 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x33 RLA izy, 8
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x34 NOP zpx, 4
		{ AND, zpx, 2, 4, "AND", am_ZPX},		// 0x35
		{ ROL, zpx, 2, 6, "ROL", am_ZPX},		// 0x36
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x37 RLA zp , 5
		{ SEC, imp, 1, 2, "SEC", am_IMP},		// 0x38
		{ AND, aby, 3, 4, "AND", am_ABY},		// 0x39
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x3a NOP imp, 2
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x3b RLA aby, 7
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x3c NOP abx, 4
		{ AND, abx, 3, 4, "AND", am_ABX},		// 0x3d
		{ ROL, abx, 3, 7, "ROL", am_ABX},		// 0x3e
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x3f RLA abx, 7

		{ RTI, imp, 1, 6, "RTI", am_IMP},		// 0x40
		{ EOR, izx, 2, 6, "EOR", am_IZX},		// 0x41
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x42 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x43 SRE izx, 8
		{ BRK, imp, 1, 3, "BRK", am_IMP},		// 0x44 NOP zp , 3
		{ EOR, zp , 2, 3, "EOR", am_ZP },		// 0x45
		{ LSR, zp , 2, 5, "LSR", am_ZP },		// 0x46
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x47 SRE zp , 5
		{ PHA, imp, 1, 3, "PHA", am_IMP},		// 0x48
		{ EOR, imm, 2, 2, "EOR", am_IMM},		// 0x49
		{ LSR, imp, 1, 2, "LSR", am_IMP},		// 0x4a
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x4b ALR imm, 2
		{ JMP, abs, 3, 3, "JMP", am_ABS},		// 0x4c
		{ EOR, abs, 3, 4, "EOR", am_ABS},		// 0x4d
		{ LSR, abs, 3, 6, "LSR", am_ABS},		// 0x4e
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x4f SRE abs, 6

		{ BVC, rel, 2, 2, "BVC", am_REL},		// 0x50
		{ EOR, izy, 2, 5, "EOR", am_IZY},		// 0x51
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x52 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x53 SRE izy, 8
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x54 NOP zpx, 4
		{ EOR, zpx, 2, 4, "EOR", am_ZPX},		// 0x55
		{ LSR, zpx, 2, 6, "LSR", am_ZPX},		// 0x56
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x57 SRE zpx, 6
		{ CLI, imp, 1, 2, "CLI", am_IMP},		// 0x58
		{ EOR, aby, 3, 4, "EOR", am_ABY},		// 0x59
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x5a NOP imp, 2
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x5b SRE aby, 7
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x5c NOP abx, 4
		{ EOR, abx, 3, 4, "EOR", am_ABX},		// 0x5d
		{ LSR, abx, 3, 7, "LSR", am_ABX},		// 0x5e
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x5f SRE abx, 7

		{ RTS, imp, 1, 6, "RTS", am_IMP},		// 0x60
		{ ADC, izx, 2, 6, "ADC", am_IZX},		// 0x61
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x62 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x63 RRA izx, 8
		{ BRK, imp, 1, 3, "BRK", am_IMP},		// 0x64 NOP zp , 3
		{ ADC, zp , 2, 3, "ADC", am_ZP },		// 0x65
		{ ROR, zp , 2, 5, "ROR", am_ZP },		// 0x66
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x67 RRA zp , 5
		{ PLA, imp, 1, 4, "PLA", am_IMP},		// 0x68
		{ ADC, imm, 2, 2, "ADC", am_IMM},		// 0x69
		{ ROR, imp, 1, 2, "ROR", am_IMP},		// 0x6a
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x6b ARR imm, 2
		{ JMP, ind, 3, 5, "JMP", am_IND},		// 0x6c
		{ ADC, abs, 3, 4, "ADC", am_ABS},		// 0x6d
		{ ROR, abs, 3, 6, "ROR", am_ABS},		// 0x6e
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x6f RRA abs, 6

		{ BVS, rel, 2, 2, "BVS", am_REL},		// 0x70
		{ ADC, izy, 2, 5, "ADC", am_IZY},		// 0x71
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x72 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0x73 RRA izy, 8
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x74 NOP zpx, 4
		{ ADC, zpx, 2, 4, "ADC", am_ZPX},		// 0x75
		{ ROR, zpx, 2, 6, "ROR", am_ZPX},		// 0x76
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x77 RRA zpx, 6
		{ SEI, imp, 1, 2, "SEI", am_IMP},		// 0x78
		{ ADC, aby, 3, 4, "ADC", am_ABY},		// 0x79
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x7a NOP imp, 2
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x7b RRA aby, 7
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x7c NOP abx, 4
		{ ADC, abx, 3, 4, "ADC", am_ABX},		// 0x7d
		{ ROR, abx, 3, 7, "ROR", am_ABX},		// 0x7e
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x7f RRA abx, 7

		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x80 NOP imm, 2
		{ STA, izx, 2, 6, "STA", am_IZX},		// 0x81
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x82 NOP imm, 2
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x83 SAX izx, 6
		{ STY, zp , 2, 3, "STY", am_ZP },		// 0x84
		{ STA, zp , 2, 3, "STA", am_ZP },		// 0x85
		{ STX, zp , 2, 3, "STX", am_ZP },		// 0x86
		{ BRK, imp, 1, 3, "BRK", am_IMP},		// 0x87 SAX zp , 3
		{ DEY, imp, 1, 2, "DEY", am_IMP},		// 0x88
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x89 NOP imm, 2
		{ TXA, imp, 1, 2, "TXA", am_IMP},		// 0x8a
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0x8b XAA imm, 2 (Highly unstable)
		{ STY, abs, 3, 4, "STY", am_ABS},		// 0x8c
		{ STA, abs, 3, 4, "STA", am_ABS},		// 0x8d
		{ STX, abs, 3, 4, "STX", am_ABS},		// 0x8e
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x8f SAX abs, 4

		{ BCC, rel, 2, 2, "BCC", am_REL},		// 0x90
		{ STA, izy, 2, 6, "STA", am_IZY},		// 0x91
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0x92 KIL
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0x93 AHZ izy, 6 (unstable)
		{ STY, zpx, 2, 4, "STY", am_ZPX},		// 0x94
		{ STA, zpx, 2, 4, "STA", am_ZPX},		// 0x95
		{ STX, zpy, 2, 4, "STX", am_ZPY},		// 0x96
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0x97 SAX zpy, 4
		{ TYA, imp, 1, 2, "TYA", am_IMP},		// 0x98 
		{ STA, aby, 3, 5, "STA", am_ABY},		// 0x99
		{ TXS, imp, 1, 2, "TXS", am_IMP},		// 0x9a
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x9b TAS aby, 5 (unstable)
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x9c SHU abx, 5 (unstable)
		{ STA, abx, 3, 5, "STA", am_ABX},		// 0x9d
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x9e SHX aby, 5
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0x9f AHX aby, 5

		{ LDY, imm, 2, 2, "LDY", am_IMM},		// 0xa0
		{ LDA, izx, 2, 6, "LDA", am_IZX},		// 0xa1
		{ LDX, imm, 2, 2, "LDX", am_IMM},		// 0xa2
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0xa3 LAX izx, 6
		{ LDY, zp , 2, 3, "LDY", am_ZP },		// 0xa4
		{ LDA, zp , 2, 3, "LDA", am_ZP },		// 0xa5
		{ LDX, zp , 2, 3, "LDX", am_ZP },		// 0xa6
		{ BRK, imp, 1, 3, "BRK", am_IMP},		// 0xa7 LAX zp , 3
		{ TAY, imp, 1, 2, "TAY", am_IMP},		// 0xa8
		{ LDA, imm, 2, 2, "LDA", am_IMM},		// 0xa9
		{ TAX, imp, 1, 2, "TAX", am_IMP},		// 0xaa
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xab LAX imm, 2 (highly unstable)
		{ LDY, abs, 3, 4, "LDY", am_ABS},		// 0xac
		{ LDA, abs, 3, 4, "LDA", am_ABS},		// 0xad
		{ LDX, abs, 3, 4, "LDX", am_ABS},		// 0xae
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xaf LAX abs, 4

		{ BCS, rel, 2, 2, "BCS", am_REL},		// 0xb0
		{ LDA, izy, 2, 5, "LDA", am_IZY},		// 0xb1
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0xb2 KIL
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0xb3 LAX izy, 5
		{ LDY, zpx, 2, 4, "LDY", am_ZPX},		// 0xb4
		{ LDA, zpx, 2, 4, "LDA", am_ZPX},		// 0xb5
		{ LDX, zpy, 2, 4, "LDX", am_ZPY},		// 0xb6
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xb7 LAX zpy, 4
		{ CLV, imp, 1, 2, "CLV", am_IMP},		// 0xb8
		{ LDA, aby, 3, 4, "LDA", am_ABY},		// 0xb9
		{ TSX, imp, 1, 2, "TSX", am_IMP},		// 0xba
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xbb LAS aby, 4
		{ LDY, abx, 3, 4, "LDY", am_ABX},		// 0xbc
		{ LDA, abx, 3, 4, "LDA", am_ABX},		// 0xbd
		{ LDX, aby, 3, 4, "LDX", am_ABY},		// 0xbe
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xbf LAX aby, 4

		{ CPY, imm, 2, 2, "CPY", am_IMM},		// 0xc0
		{ CMP, izx, 2, 6, "CMP", am_IZX},		// 0xc1
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xc2 NOP imm, 2
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0xc3 DCP izx, 8
		{ CPY, zp , 2, 3, "CPY", am_ZP },		// 0xc4
		{ CMP, zp , 2, 3, "CMP", am_ZP },		// 0xc5
		{ DEC, zp , 2, 5, "DEC", am_ZP },		// 0xc6
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0xc7 DCP zp , 5
		{ INY, imp, 1, 2, "INY", am_IMP},		// 0xc8
		{ CMP, imm, 2, 2, "CMP", am_IMM},		// 0xc9
		{ DEX, imp, 1, 2, "DEX", am_IMP},		// 0xca
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xcb AXS imm, 2
		{ CPY, abs, 3, 4, "CPY", am_ABS},		// 0xcc
		{ CMP, abs, 3, 4, "CMP", am_ABS},		// 0xcd
		{ DEC, abs, 3, 6, "DEC", am_ABS},		// 0xce
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0xcf DCP abs, 6

		{ BNE, rel, 2, 2, "BNE", am_REL},		// 0xd0
		{ CMP, izy, 2, 5, "CMP", am_IZY},		// 0xd1
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0xd2 KILL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0xd3 DCP izy, 8
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xd4 NOP zpx, 4
		{ CMP, zpx, 2, 4, "CMP", am_ZPX},		// 0xd5
		{ DEC, zpx, 2, 6, "DEC", am_ZPX},		// 0xd6
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0xd7 DCP zpx, 6
		{ CLD, imp, 1, 2, "CLD", am_IMP},		// 0xd8
		{ CMP, aby, 3, 4, "CMP", am_ABY},		// 0xd9
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xda NOP, imp, 2
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0xdb DCP, aby, 7
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xdc NOP abs, 4
		{ CMP, abx, 3, 4, "CMP", am_ABX},		// 0xdd
		{ DEC, abx, 3, 7, "DEC", am_ABX},		// 0xde
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0xdf DCP abx, 7

		{ CPX, imm, 2, 2, "CPX", am_IMM},		// 0xe0
		{ SBC, izx, 2, 6, "SBC", am_IZX},		// 0xe1
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xe2 NOP imm, 2
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0xe3 ISC izx, 8
		{ CPX, zp , 2, 3, "CPX", am_ZP },		// 0xe4
		{ SBC, zp , 2, 3, "SBC", am_ZP },		// 0xe5
		{ INC, zp , 2, 5, "INC", am_ZP },		// 0xe6
		{ BRK, imp, 1, 5, "BRK", am_IMP},		// 0xe7 ISC zp , 5
		{ INX, imp, 1, 2, "INX", am_IMP},		// 0xe8
		{ SBC, imm, 2, 2, "SBC", am_IMM},		// 0xe9
		{ NOP, imp, 1, 2, "NOP", am_IMP},		// 0xea
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xeb SBC imm, 2
		{ CPX, abs, 3, 4, "CPX", am_ABS},		// 0xec
		{ SBC, abs, 3, 4, "SBC", am_ABS},		// 0xed
		{ INC, abs, 3, 6, "INC", am_ABS},		// 0xee
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0xef ISC abs, 6

		{ BEQ, rel, 2, 2, "BEQ", am_REL},		// 0xf0
		{ SBC, izy, 2, 5, "SBC", am_IZY},		// 0xf1
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0xf2 KIL
		{ BRK, imp, 1, 8, "BRK", am_IMP},		// 0xf3 ISC izy, 8
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xf4 NOP zpx, 4
		{ SBC, zpx, 2, 4, "SBC", am_ZPX},		// 0xf5
		{ INC, zpx, 2, 6, "INC", am_ZPX},		// 0xf6
		{ BRK, imp, 1, 6, "BRK", am_IMP},		// 0xf7 ISC zpx, 6
		{ SED, imp, 1, 2, "SED", am_IMP},		// 0xf8
		{ SBC, aby, 3, 4, "SBC", am_ABY},		// 0xf9
		{ BRK, imp, 1, 2, "BRK", am_IMP},		// 0xfa NOP, imp, 2
		{ BRK, imp, 1, 7, "BRK", am_IMP},		// 0xfb ISC aby, 7
		{ BRK, imp, 1, 4, "BRK", am_IMP},		// 0xfc NOP abx, 4
		{ SBC, abx, 3, 4, "SBC", am_ABX},		// 0xfd
		{ INC, abx, 3, 7, "INC", am_ABX},		// 0xfe
		{ BRK, imp, 1, 7, "BRK", am_IMP}		// 0xff ISC abx, 7
	};

	//------------------------------------------------------------------------------------------------------------------------------
	// State
	//------------------------------------------------------------------------------------------------------------------------------

	CPUmos6510::State::State()
		: m_Memory(nullptr)
		, m_WriteCallback(nullptr)
	{
		Reset();
	}

	CPUmos6510::State::~State()
	{

	}

	//------------------------------------------------------------------------------------------------------------------------------

	void CPUmos6510::State::Reset()
	{
		// Clear program counter and stack pointer
		m_PC = 0;
		m_SP = 0;

		// Clear status register
		m_Status = 0;

		// Clear other register
		m_RegA = 0;
		m_RegX = 0;
		m_RegY = 0;

		// Reset cycle counter
		m_Cycle = 0;

		// Suspend the CPU
		m_IsSuspended = true;
	}

	//------------------------------------------------------------------------------------------------------------------------------
	// CPU, mos6510
	//------------------------------------------------------------------------------------------------------------------------------

	CPUmos6510::CPUmos6510()
	{
		m_State.Reset();
	}


	CPUmos6510::~CPUmos6510()
	{
	}


	void CPUmos6510::Reset()
	{
		m_State.Reset();
	}


	short CPUmos6510::ExecuteInstruction()
	{
		if(m_State.IsValid() && !m_State.IsSuspended())
		{
			// Variable holding the number of added cycles
			int added_cycles = 0;

			// Get the opcode to process
			unsigned char opcode = m_State.GetMemory()[m_State.m_PC];

			// Get the address of the processing if any, according to the opcode addressing mode
			const void* inAddress = ms_aInstructions[opcode].m_pmAdressingMode(m_State, added_cycles);

			// Execute the instruction
			if (ms_aInstructions[opcode].m_pmInstruction(m_State, inAddress))
				added_cycles++;

			// Bump the cycle counter
			m_State.AddCycles((int)ms_aInstructions[opcode].m_ucBaseCycles + added_cycles);

			// Return instruction cycle count
			return ms_aInstructions[opcode].m_ucBaseCycles;
		}

		// Cpu is suspended
		return 0;
	}


	const unsigned char CPUmos6510::GetOpcodeByteSize(const unsigned char inOpcode) 
	{
		return ms_aInstructions[inOpcode].m_ucSize;
	}


	const CPUmos6510::AddressingMode CPUmos6510::GetOpcodeAddressingMode(const unsigned char inOpcode) 
	{
		return ms_aInstructions[inOpcode].m_eMode;
	}


	//------------------------------------------------------------------------------------------------------------------------------

	//---------------------------------------------------------------------------
	// Logical and arithmetic commands

	bool CPUmos6510::ORA(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA |= *(unsigned char*)inAddress;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}


	bool CPUmos6510::AND(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA &= *(unsigned char*)inAddress;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}


	bool CPUmos6510::EOR(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA ^= *(unsigned char*)inAddress;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}


	bool CPUmos6510::ADC(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short a = (unsigned short)ioState.m_RegA;
		unsigned char sign = (unsigned short)ioState.m_RegA & 0x80;

		a += (unsigned short)(*(unsigned char*)inAddress) + (ioState.IsStatusFlagSet(SF_C) ? 1:0);

		if((a & 0xff00) != 0)
			ioState.SetStatusFlag(SF_C);
		else
			ioState.ClearStatusFlag(SF_C);

		if((sign ^ (a & 0x80)) != 0)
			ioState.SetStatusFlag(SF_V);
		else
			ioState.ClearStatusFlag(SF_V);

		ioState.m_RegA = (unsigned char)a;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}


	bool CPUmos6510::SBC(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short a = (unsigned short)ioState.m_RegA;
		unsigned char sign = (unsigned short)ioState.m_RegA & 0x80;

		a -= ((unsigned short)*((unsigned char*)inAddress) + (ioState.IsStatusFlagSet(SF_C) ? 0:1));

		if((a & 0xff00) != 0)
			ioState.ClearStatusFlag(SF_C);
		else
			ioState.SetStatusFlag(SF_C);

		if((sign ^ (a & 0x80)) != 0)
			ioState.SetStatusFlag(SF_V);
		else
			ioState.ClearStatusFlag(SF_V);

		ioState.m_RegA = (unsigned char)a;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}


	bool CPUmos6510::CMP(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short a = ioState.m_RegA;
		a -= *(unsigned char*)inAddress;

		if(a == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.SetStatusFlag(SF_C);
			ioState.ClearStatusFlag(SF_N);
		}
		else 
		{
			ioState.ClearStatusFlag(SF_Z);

			if((a & 0x80) == 0)
			{
				ioState.SetStatusFlag(SF_C);
				ioState.ClearStatusFlag(SF_N);
			}
			else
			{
				ioState.ClearStatusFlag(SF_C);
				ioState.SetStatusFlag(SF_N);
			}
		}

		return false;
	}


	bool CPUmos6510::CPX(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char x = ioState.m_RegX;
		x -= *(unsigned char*)inAddress;

		if(x == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_C);
			ioState.ClearStatusFlag(SF_N);
		}
		else 
		{
			ioState.ClearStatusFlag(SF_Z);

			if((x & 0x80) == 0)
			{
				ioState.ClearStatusFlag(SF_C);
				ioState.ClearStatusFlag(SF_N);
			}
			else
			{
				ioState.SetStatusFlag(SF_C);
				ioState.SetStatusFlag(SF_N);
			}
		}

		return false;
	}



	bool CPUmos6510::CPY(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char y = ioState.m_RegY;
		y -= *(unsigned char*)inAddress;

		if(y == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_C);
			ioState.ClearStatusFlag(SF_N);
		}
		else 
		{
			ioState.ClearStatusFlag(SF_Z);

			if((y & 0x80) == 0)
			{
				ioState.ClearStatusFlag(SF_C);
				ioState.ClearStatusFlag(SF_N);
			}
			else
			{
				ioState.SetStatusFlag(SF_C);
				ioState.SetStatusFlag(SF_N);
			}
		}

		return false;
	}


	bool CPUmos6510::DEC(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short val = (unsigned short)(*(unsigned char*)inAddress);
		val--;
		*(unsigned char*)inAddress = (unsigned char)val;

		if((val & 0xff) == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_N);
		}
		else
		{
			ioState.ClearStatusFlag(SF_Z);
			if((val & 0x80) == 0)
				ioState.ClearStatusFlag(SF_N);
			else
				ioState.SetStatusFlag(SF_N);
		}

		ioState.MemoryWrite(inAddress, (unsigned char)val);

		return false;
	}


	bool CPUmos6510::DEX(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short val = (unsigned short)(ioState.m_RegX);
		val--;
		ioState.m_RegX = (unsigned char)val;

		if(val == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_N);
		}
		else
		{
			ioState.ClearStatusFlag(SF_Z);
			if((val & 0x80) == 0)
				ioState.ClearStatusFlag(SF_N);
			else
				ioState.SetStatusFlag(SF_N);
		}

		return false;
	}

	bool CPUmos6510::DEY(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short val = (unsigned short)(ioState.m_RegY);
		val--;
		ioState.m_RegY = (unsigned char)val;

		if(val == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_N);
		}
		else
		{
			ioState.ClearStatusFlag(SF_Z);
			if((val & 0x80) == 0)
				ioState.ClearStatusFlag(SF_N);
			else
				ioState.SetStatusFlag(SF_N);
		}

		return false;
	}


	bool CPUmos6510::INC(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short val = (unsigned short)(*(unsigned char*)inAddress);
		val++;
		*(unsigned char*)inAddress = (unsigned char)val;

		if((val & 0xff) == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_N);
		}
		else
		{
			ioState.ClearStatusFlag(SF_Z);
			if((val & 0x80) == 0)
				ioState.ClearStatusFlag(SF_N);
			else
				ioState.SetStatusFlag(SF_N);
		}

		ioState.MemoryWrite(inAddress, (unsigned char)val);

		return false;
	}


	bool CPUmos6510::INX(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short val = (unsigned short)(ioState.m_RegX);
		val++;
		ioState.m_RegX = (unsigned char)val;

		if(val == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_N);
		}
		else
		{
			ioState.ClearStatusFlag(SF_Z);
			if((val & 0x80) == 0)
				ioState.ClearStatusFlag(SF_N);
			else
				ioState.SetStatusFlag(SF_N);
		}

		return false;
	}

	bool CPUmos6510::INY(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short val = (unsigned short)(ioState.m_RegY);
		val++;
		ioState.m_RegY = (unsigned char)val;

		if(val == 0)
		{
			ioState.SetStatusFlag(SF_Z);
			ioState.ClearStatusFlag(SF_N);
		}
		else
		{
			ioState.ClearStatusFlag(SF_Z);
			if((val & 0x80) == 0)
				ioState.ClearStatusFlag(SF_N);
			else
				ioState.SetStatusFlag(SF_N);
		}

		return false;
	}

	bool CPUmos6510::ASL(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char val = *((unsigned char*)inAddress);

		if((val & 0x80) != 0)
			ioState.SetStatusFlag(SF_C);
		else
			ioState.ClearStatusFlag(SF_C);

		val <<= 1;

		if(val == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((val & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		*((unsigned char*)inAddress) = val;

		return false;
	}


	bool CPUmos6510::ROL(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char val = *((unsigned char*)inAddress);

		const bool insertBit = ioState.IsStatusFlagSet(SF_C);

		if((val & 0x80) != 0)
			ioState.SetStatusFlag(SF_C);
		else
			ioState.ClearStatusFlag(SF_C);

		val <<= 1;

		if(insertBit)
			val |= 1;

		if(val == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((val & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		*((unsigned char*)inAddress) = val;

		return false;
	}


	bool CPUmos6510::LSR(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char val = *((unsigned char*)inAddress);

		if((val & 0x01) != 0)
			ioState.SetStatusFlag(SF_C);
		else
			ioState.ClearStatusFlag(SF_C);

		val >>= 1;

		if(val == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		// This should actually always be false.. but let's do the check in any case!
		if((val & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		*((unsigned char*)inAddress) = val;

		return false;
	}


	bool CPUmos6510::ROR(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char val = *((unsigned char*)inAddress);

		bool insertBit = ioState.IsStatusFlagSet(SF_C);

		if((val & 0x01) != 0)
			ioState.SetStatusFlag(SF_C);
		else
			ioState.ClearStatusFlag(SF_C);

		val >>= 1;

		if(insertBit)
			val |= 0x80;

		if(val == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((val & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		*((unsigned char*)inAddress) = val;

		return false;
	}


	// Move commands
	bool CPUmos6510::LDA(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA = *((unsigned char*)inAddress);

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::STA(CPUmos6510::State& ioState, const void* inAddress)
	{
		*((unsigned char*)inAddress) = ioState.m_RegA;

		ioState.MemoryWrite(inAddress, ioState.m_RegA);

		return false;
	}

	bool CPUmos6510::LDX(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegX = *((unsigned char*)inAddress);

		if(ioState.m_RegX == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegX & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::STX(CPUmos6510::State& ioState, const void* inAddress)
	{
		*((unsigned char*)inAddress) = ioState.m_RegX;

		ioState.MemoryWrite(inAddress, ioState.m_RegX);

		return false;
	}

	bool CPUmos6510::LDY(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegY = *((unsigned char*)inAddress);

		if(ioState.m_RegY == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegY & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::STY(CPUmos6510::State& ioState, const void* inAddress)
	{
		*((unsigned char*)inAddress) = ioState.m_RegY;

		ioState.MemoryWrite(inAddress, ioState.m_RegY);

		return false;
	}

	bool CPUmos6510::TAX(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegX = ioState.m_RegA;

		if(ioState.m_RegX == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegX & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::TXA(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA = ioState.m_RegX;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::TAY(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegY = ioState.m_RegA;

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::TYA(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA = ioState.m_RegY;

		if(ioState.m_RegY == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegY & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::TSX(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegX = ioState.m_SP;

		if(ioState.m_RegX == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegX & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::TXS(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_SP = ioState.m_RegX;

		return false;
	}

	bool CPUmos6510::PLA(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_RegA = ioState.StackPull();

		if(ioState.m_RegA == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		if((ioState.m_RegA & 0x80) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		return false;
	}

	bool CPUmos6510::PHA(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.StackPush(ioState.m_RegA);

		return false;
	}

	bool CPUmos6510::PLP(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_Status = ioState.StackPull();

		return false;
	}

	bool CPUmos6510::PHP(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.StackPush(ioState.m_Status);

		return false;
	}


	// Jump / Flag commands
	bool CPUmos6510::BPL(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(!ioState.IsStatusFlagSet(SF_N))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BMI(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(ioState.IsStatusFlagSet(SF_N))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BVC(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(!ioState.IsStatusFlagSet(SF_V))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BVS(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(ioState.IsStatusFlagSet(SF_V))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BCC(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(!ioState.IsStatusFlagSet(SF_C))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BCS(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(ioState.IsStatusFlagSet(SF_C))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BNE(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(!ioState.IsStatusFlagSet(SF_Z))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BEQ(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(ioState.IsStatusFlagSet(SF_Z))
		{
			ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
			return true;
		}

		return false;
	}

	bool CPUmos6510::BRK(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned short pc = ioState.m_PC;

		ioState.StackPush((unsigned char)(pc & 0xff));
		ioState.StackPush((unsigned char)(pc >> 8));
		ioState.StackPush(ioState.m_Status);

		ioState.m_PC = 0xfffe;

		ioState.SetSuspended(true);

		return false;
	}

	bool CPUmos6510::RTI(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_Status = ioState.StackPull();
		ioState.m_PC = (((unsigned short)ioState.StackPull()) << 8) | ((unsigned short)ioState.StackPull());

		return false;
	}

	bool CPUmos6510::JSR(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.StackPush((unsigned char)(ioState.m_PC & 0xff));
		ioState.StackPush((unsigned char)(ioState.m_PC >> 8));

		ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));

		return false;
	}

	bool CPUmos6510::RTS(CPUmos6510::State& ioState, const void* inAddress)
	{
		if(ioState.m_SP != 0x00)
			ioState.m_PC = (((unsigned short)ioState.StackPull()) << 8) | ((unsigned short)ioState.StackPull());
		else
			ioState.SetSuspended(true);

		return false;
	}

	bool CPUmos6510::JMP(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.m_PC = static_cast<unsigned short>(ioState.GetMemory().GetAddress(inAddress));
		return false;
	}

	bool CPUmos6510::BIT(CPUmos6510::State& ioState, const void* inAddress)
	{
		unsigned char val = *(unsigned char*)inAddress;

		if((val & (1<<7)) != 0)
			ioState.SetStatusFlag(SF_N);
		else
			ioState.ClearStatusFlag(SF_N);

		if((val & (1<<6)) != 0)
			ioState.SetStatusFlag(SF_V);
		else
			ioState.ClearStatusFlag(SF_V);

		if((ioState.m_RegA & val) == 0)
			ioState.SetStatusFlag(SF_Z);
		else
			ioState.ClearStatusFlag(SF_Z);

		return false;
	}

	bool CPUmos6510::CLC(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.ClearStatusFlag(SF_C);
		return false;
	}

	bool CPUmos6510::SEC(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.SetStatusFlag(SF_C);
		return false;
	}

	bool CPUmos6510::CLD(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.ClearStatusFlag(SF_D);
		return false;
	}

	bool CPUmos6510::SED(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.SetStatusFlag(SF_D);
		return false;
	}

	bool CPUmos6510::CLI(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.ClearStatusFlag(SF_I);
		return false;
	}

	bool CPUmos6510::SEI(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.SetStatusFlag(SF_I);
		return false;
	}

	bool CPUmos6510::CLV(CPUmos6510::State& ioState, const void* inAddress)
	{
		ioState.ClearStatusFlag(SF_V);
		return false;
	}

	bool CPUmos6510::NOP(CPUmos6510::State& ioState, const void* inAddress)
	{
		return false;
	}


	// Addressing modes
	void* CPUmos6510::imp(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		ioState.m_PC += 1;
		return (void*)&ioState.m_RegA;
	}


	// Addressing modes
	void* CPUmos6510::imm(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		void *adr = (void*)&ioState.GetMemory()[ioState.m_PC+1];

		ioState.m_PC += 2;

		return adr;
	}

	void* CPUmos6510::zp(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();
		void *adr = (void*)&rMemory[rMemory[ioState.m_PC+1]];

		ioState.m_PC += 2;

		return adr;
	}

	void* CPUmos6510::zpx(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();
		void *adr = (void*)&rMemory[((rMemory[ioState.m_PC+1] + ioState.m_RegX) & 0xff)];

		ioState.m_PC += 2;

		return adr;
	}


	void* CPUmos6510::zpy(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();
		void *adr = (void*)&rMemory[((rMemory[ioState.m_PC+1] + ioState.m_RegY) & 0xff)];

		ioState.m_PC += 2;

		return adr;
	}

	void* CPUmos6510::izx(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();
		unsigned short zp = (unsigned short)(rMemory[ioState.m_PC+1] + ioState.m_RegX) & 0xff;

		ioState.m_PC += 2;

		return (void*)&rMemory[(unsigned short)rMemory[zp] | (((unsigned short)rMemory[(zp+1) & 0xff]) << 8)];
	}

	void* CPUmos6510::izy(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();
		
		unsigned short zp = (unsigned short)(rMemory[ioState.m_PC+1]);
		unsigned short base_target_address = ((unsigned short)rMemory[zp] | (((unsigned short)rMemory[(zp + 1) & 0xff]) << 8));
		unsigned short target_address = base_target_address + ioState.m_RegY;

		outAddedCycles = ((base_target_address & 0xff00) != (target_address & 0xff00)) ? 1 : 0;

		ioState.m_PC += 2;

		return (void*)&rMemory[target_address];
	}

	void* CPUmos6510::abs(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();
		void *ret = (void*)&rMemory[((unsigned short)rMemory[ioState.m_PC+1] | (((unsigned short)rMemory[ioState.m_PC+2]) << 8))];

		ioState.m_PC += 3;

		return ret;
	}


	void* CPUmos6510::abx(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();

		unsigned short target_address = ((unsigned short)rMemory[ioState.m_PC + 1] | (((unsigned short)rMemory[ioState.m_PC + 2]) << 8)) + ioState.m_RegX;
		outAddedCycles = ((target_address & 0xff00) != (ioState.m_PC & 0xff00)) ? 1 : 0;

		ioState.m_PC += 3;

		return (void*)&rMemory[target_address];
	}

	void* CPUmos6510::aby(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();

		unsigned short target_address = ((unsigned short)rMemory[ioState.m_PC + 1] | (((unsigned short)rMemory[ioState.m_PC + 2]) << 8)) + ioState.m_RegY;
		outAddedCycles = ((target_address & 0xff00) != (ioState.m_PC & 0xff00)) ? 1 : 0;

		ioState.m_PC += 3;

		return (void*)&rMemory[target_address];
	}


	void *CPUmos6510::ind(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();

		unsigned short adrL = (unsigned short)rMemory[ioState.m_PC+1];
		unsigned short adrH = (((unsigned short)rMemory[ioState.m_PC+2]) << 8);

		ioState.m_PC += 3;

		return (void*)&rMemory[((unsigned short)rMemory[adrL | adrH] | (((unsigned short)rMemory[((adrL + 1) & 0xff) | adrH]) << 8))];
	}

	void* CPUmos6510::rel(CPUmos6510::State& ioState, int& outAddedCycles)
	{
		CPUMemory& rMemory = ioState.GetMemory();

		char r = (char)rMemory[ioState.m_PC+1];
		unsigned short target_address = (unsigned short)(ioState.m_PC + 2 + (short)r);

		outAddedCycles = ((target_address & 0xff00) != (ioState.m_PC & 0xff00)) ? 1 : 0;

		ioState.m_PC += 2;

		return (void*)&rMemory[target_address];
	}
}
