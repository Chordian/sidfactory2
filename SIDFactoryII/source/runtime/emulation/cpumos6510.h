#if !defined(__CPU_MOS6510_H__)
#define __CPU_MOS6510_H__

#include "cpumemory.h"
#include "icpuwritecallback.h"

namespace Emulation
{
	class CPUmos6510
	{
	public:
		// Status flags of the CPU
		enum StatusFlag
		{
			SF_N,			// Negative
			SF_V,			// Overflow
			SF_Undefined1,
			SF_B,			// Break
			SF_D,			// Decimal m_eMode
			SF_I,			// IRQ
			SF_Z,			// Zero
			SF_C			// Carry
		};

		// Addressing modes
		enum AddressingMode 
		{
			am_IMP,
			am_IMM,
			am_ZP,
			am_ZPX,
			am_ZPY,
			am_IZX,
			am_IZY,
			am_ABS,
			am_ABX,
			am_ABY,
			am_IND,
			am_REL
		};

	private:
		// State of the CPU
		class State
		{
		public:
			State();
			~State();

			void SetWriteCallback(ICPUWriteCallback* pCallback) { m_WriteCallback = pCallback; }

			void Reset();
			void SetMemory(CPUMemory* pMemory) { m_Memory = pMemory; }
			
			inline bool IsValid() const { return m_Memory != nullptr; }

			inline void SetStatusFlag(StatusFlag flag) { m_Status |= (1 << flag); };
			inline void ClearStatusFlag(StatusFlag flag) { m_Status &= ~(1 << flag); };

			inline bool IsStatusFlagSet(StatusFlag flag) const { return ((m_Status & (1 << flag)) != 0); };

			// Stack
			inline void StackPush(unsigned char ucVal)
			{
				if(m_Memory != nullptr) 
				{ 
					(*m_Memory)[0x0100 + m_SP] = ucVal; 
					m_SP--; 
				} 

			}

			inline unsigned char StackPull()
			{
				if(m_Memory != nullptr)
				{
					m_SP++;
					return (*m_Memory)[0x0100 + m_SP];
				}

				return 0;
			}

			// Suspended
			inline void SetSuspended(bool bSuspended) { m_IsSuspended = bSuspended; }
			inline bool IsSuspended() const { return m_IsSuspended; }

			// Cycle counter
			inline void ResetCycleCounter() { m_Cycle = 0; }
			inline void SetCycle(int iCycle) { m_Cycle = iCycle; }
			inline int GetCycle() const { return m_Cycle; }
			inline void AddCycles(int iCycles) { m_Cycle += iCycles; }

			// Memory
			inline CPUMemory& GetMemory() { FOUNDATION_ASSERT(m_Memory); return *m_Memory; }
			inline void MemoryWrite(const void* pAddress, unsigned char ucVal)
			{
				if(m_WriteCallback != nullptr)
				{
					const unsigned int address = m_Memory->GetAddress(pAddress);
					m_WriteCallback->Write(static_cast<unsigned short>(address), ucVal, m_Cycle);
				}
			}

			// Registers
			unsigned char m_RegA;
			unsigned char m_RegX;
			unsigned char m_RegY;

			// Program pointer
			unsigned short m_PC;

			// Stack pointer
			unsigned char m_SP;

			// Flags
			unsigned char m_Status;

		private:
			// Suspended
			bool m_IsSuspended;

			// Memory
			CPUMemory* m_Memory;
			ICPUWriteCallback* m_WriteCallback;

			// Cycle counter
			int m_Cycle;
		};

		struct Instruction
		{
			bool (*m_pmInstruction)(State& inState, const void* inAddress);
			void* (*m_pmAdressingMode)(State& inState, int& outAddedCycles);

			const unsigned char m_ucSize;
			const unsigned char m_ucBaseCycles;
			const char* m_acOpcode;
			const AddressingMode m_eMode;
		};

		// Instruction set matrix
		static Instruction ms_aInstructions[0x100];

		// CPU Class implementation

	public:

		// Addressing modes
		static void* imp(State& ioState, int& outAddedCycles);	// Implicit
		static void* imm(State& ioState, int& outAddedCycles);	// Immediate
		static void* zp(State& ioState, int& outAddedCycles);	// Zeropage
		static void* zpx(State& ioState, int& outAddedCycles);	// Zeropage,x
		static void* zpy(State& ioState, int& outAddedCycles);	// Zeropage,y
		static void* izx(State& ioState, int& outAddedCycles);	// Indirect - (Zeropage,x)
		static void* izy(State& ioState, int& outAddedCycles);	// Indirect - (Zeropage),y
		static void* abs(State& ioState, int& outAddedCycles);	// Absolute
		static void* abx(State& ioState, int& outAddedCycles);	// Absolute,x
		static void* aby(State& ioState, int& outAddedCycles);	// Absolute,y
		static void* ind(State& ioState, int& outAddedCycles);	// Indirect
		static void* rel(State& ioState, int& outAddedCycles);	// Relative

		// Logical and arithmetic commands
		static bool ORA(State& ioState, const void* inAddress);
		static bool AND(State& ioState, const void* inAddress);
		static bool EOR(State& ioState, const void* inAddress);
		static bool ADC(State& ioState, const void* inAddress);
		static bool SBC(State& ioState, const void* inAddress);
		static bool CMP(State& ioState, const void* inAddress);
		static bool CPX(State& ioState, const void* inAddress);
		static bool CPY(State& ioState, const void* inAddress);
		static bool DEC(State& ioState, const void* inAddress);
		static bool DEX(State& ioState, const void* inAddress);
		static bool DEY(State& ioState, const void* inAddress);
		static bool INC(State& ioState, const void* inAddress);
		static bool INX(State& ioState, const void* inAddress);
		static bool INY(State& ioState, const void* inAddress);
		static bool ASL(State& ioState, const void* inAddress);
		static bool ROL(State& ioState, const void* inAddress);
		static bool LSR(State& ioState, const void* inAddress);
		static bool ROR(State& ioState, const void* inAddress);

		// Move commands
		static bool LDA(State& ioState, const void* inAddress);
		static bool STA(State& ioState, const void* inAddress);
		static bool LDX(State& ioState, const void* inAddress);
		static bool STX(State& ioState, const void* inAddress);
		static bool LDY(State& ioState, const void* inAddress);
		static bool STY(State& ioState, const void* inAddress);
		static bool TAX(State& ioState, const void* inAddress);
		static bool TXA(State& ioState, const void* inAddress);
		static bool TAY(State& ioState, const void* inAddress);
		static bool TYA(State& ioState, const void* inAddress);
		static bool TSX(State& ioState, const void* inAddress);
		static bool TXS(State& ioState, const void* inAddress);
		static bool PLA(State& ioState, const void* inAddress);
		static bool PHA(State& ioState, const void* inAddress);
		static bool PLP(State& ioState, const void* inAddress);
		static bool PHP(State& ioState, const void* inAddress);

		// Jump/Flag commands
		static bool BPL(State& ioState, const void* inAddress);
		static bool BMI(State& ioState, const void* inAddress);
		static bool BVC(State& ioState, const void* inAddress);
		static bool BVS(State& ioState, const void* inAddress);
		static bool BCC(State& ioState, const void* inAddress);
		static bool BCS(State& ioState, const void* inAddress);
		static bool BNE(State& ioState, const void* inAddress);
		static bool BEQ(State& ioState, const void* inAddress);
		static bool BRK(State& ioState, const void* inAddress);
		static bool RTI(State& ioState, const void* inAddress);
		static bool JSR(State& ioState, const void* inAddress);
		static bool RTS(State& ioState, const void* inAddress);
		static bool JMP(State& ioState, const void* inAddress);
		static bool BIT(State& ioState, const void* inAddress);
		static bool CLC(State& ioState, const void* inAddress);
		static bool SEC(State& ioState, const void* inAddress);
		static bool CLD(State& ioState, const void* inAddress);
		static bool SED(State& ioState, const void* inAddress);
		static bool CLI(State& ioState, const void* inAddress);
		static bool SEI(State& ioState, const void* inAddress);
		static bool CLV(State& ioState, const void* inAddress);
		static bool NOP(State& ioState, const void* inAddress);

		CPUmos6510();
		~CPUmos6510();

		void Reset();

		// Suspend
		inline bool IsSuspended() const
		{ 
			return m_State.IsSuspended() || !m_State.IsValid();
		}

		inline void SetSuspended(bool bSuspend) 
		{
			m_State.SetSuspended(bSuspend); 
		}

		// Memory
		inline void SetMemory(CPUMemory* pMemory)
		{
			m_State.SetMemory(pMemory);
		}

		// Cycle counter
		inline void CycleCounterReset()
		{
			m_State.ResetCycleCounter();
		}

		inline void CycleCounterSetCurrent(int iCycle)
		{
			m_State.SetCycle(iCycle);
		}

		inline int CycleCounterGetCurrent()
		{
			return m_State.GetCycle();
		}

		// Program counter
		inline void SetPC(unsigned short usAddress) 
		{ 
			m_State.m_PC = usAddress;
			m_State.SetSuspended(false);
		}

		inline void SetAccumulator(unsigned char inValue)
		{
			m_State.m_RegA = inValue;
		}

		inline unsigned short GetPC()
		{
			return m_State.m_PC;
		}

		// Write callback
		inline void SetWriteCallback(ICPUWriteCallback* pCallback)
		{
			m_State.SetWriteCallback(pCallback);
		}

		// Execution
		short ExecuteInstruction();

		// Opcode
		static const unsigned char GetOpcodeByteSize(const unsigned char inOpcode);
		static const AddressingMode GetOpcodeAddressingMode(const unsigned char inOpcode);
		static const unsigned char GetOpcodeCycles(const unsigned char inOpcode);

	private:
		// CPU State
		State m_State;
	};
}

#endif //__CPU_MOS6510_H__