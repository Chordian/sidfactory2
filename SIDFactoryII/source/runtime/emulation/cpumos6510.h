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

			void SetWriteCallback(ICPUWriteCallback* pCallback) { m_pWriteCallback = pCallback; }

			void Reset();
			void SetMemory(CPUMemory* pMemory) { m_Memory = pMemory; }
			
			inline bool IsOk() const { return m_Memory != nullptr; }

			inline void Lock() const { m_Memory->Lock(); }
			inline void Unlock() const { m_Memory->Unlock(); }

			inline void SetStatusFlag(StatusFlag flag) { m_ucStatusReg |= (1 << flag); };
			inline void ClearStatusFlag(StatusFlag flag) { m_ucStatusReg &= ~(1 << flag); };

			inline bool IsStatusFlagSet(StatusFlag flag) const { return ((m_ucStatusReg & (1 << flag)) != 0); };

			// Stack
			inline void StackPush(unsigned char ucVal)
			{
				if(m_Memory != nullptr) 
				{ 
					(*m_Memory)[0x0100 + m_ucSP] = ucVal; 
					m_ucSP--; 
				} 

			}

			inline unsigned char StackPull()
			{
				if(m_Memory != nullptr)
				{
					m_ucSP++;
					return (*m_Memory)[0x0100 + m_ucSP];
				}

				return 0;
			}

			// Suspended
			inline void SetSuspended(bool bSuspended) { m_bSuspended = bSuspended; }
			inline bool IsSuspended() const { return m_bSuspended; }

			// Cycle counter
			inline void ResetCycleCounter() { m_iCycle = 0; }
			inline void SetCycle(int iCycle) { m_iCycle = iCycle; }
			inline int GetCycle() const { return m_iCycle; }
			inline void AddCycles(int iCycles) { m_iCycle += iCycles; }

			// Memory
			inline CPUMemory& GetMemory() { assert(m_Memory); return *m_Memory; }
			inline void MemoryWrite(void* pAddress, unsigned char ucVal)
			{
				if(m_pWriteCallback != nullptr)
				{
					const unsigned int address = m_Memory->GetAddress(pAddress);
					m_pWriteCallback->Write(static_cast<unsigned short>(address), ucVal, m_iCycle);
				}
			}

			// Registers
			unsigned char m_ucRegA;
			unsigned char m_ucRegX;
			unsigned char m_ucRegY;

			// Program pointer
			unsigned short m_usPC;

			// Stack pointer
			unsigned char m_ucSP;

			// Flags
			unsigned char m_ucStatusReg;

		private:
			// Suspended
			bool m_bSuspended;

			// Memory
			CPUMemory* m_Memory;
			ICPUWriteCallback* m_pWriteCallback;

			// Cycle counter
			int m_iCycle;
		};

		struct Instruction
		{
			bool (*m_pmInstruction)(State *state, void *address);
			void *(*m_pmAdressingMode)(State *state, int& outAddedCycles);

			unsigned char m_ucSize;
			unsigned char m_ucBaseCycles;
			const char* m_acOpcode;
			const AddressingMode m_eMode;
		};

		// Instruction set matrix
		static Instruction ms_aInstructions[0x100];

		// CPU Class implementation

	public:

		// Addressing modes
		static void* imp(State* pData, int& outAddedCycles);	// Implicit
		static void* imm(State* pData, int& outAddedCycles);	// Immediate
		static void* zp(State* pData, int& outAddedCycles);	// Zeropage
		static void* zpx(State* pData, int& outAddedCycles);	// Zeropage,x
		static void* zpy(State* pData, int& outAddedCycles);	// Zeropage,y
		static void* izx(State* pData, int& outAddedCycles);	// Indirect - (Zeropage,x)
		static void* izy(State* pData, int& outAddedCycles);	// Indirect - (Zeropage),y
		static void* abs(State* pData, int& outAddedCycles);	// Absolute
		static void* abx(State* pData, int& outAddedCycles);	// Absolute,x
		static void* aby(State* pData, int& outAddedCycles);	// Absolute,y
		static void* ind(State* pData, int& outAddedCycles);	// Indirect
		static void* rel(State* pData, int& outAddedCycles);	// Relative

		// Logical and arithmetic commands
		static bool ORA(State* pData, void* pAddress);
		static bool AND(State* pData, void* pAddress);
		static bool EOR(State* pData, void* pAddress);
		static bool ADC(State* pData, void* pAddress);
		static bool SBC(State* pData, void* pAddress);
		static bool CMP(State* pData, void* pAddress);
		static bool CPX(State* pData, void* pAddress);
		static bool CPY(State* pData, void* pAddress);
		static bool DEC(State* pData, void* pAddress);
		static bool DEX(State* pData, void* pAddress);
		static bool DEY(State* pData, void* pAddress);
		static bool INC(State* pData, void* pAddress);
		static bool INX(State* pData, void* pAddress);
		static bool INY(State* pData, void* pAddress);
		static bool ASL(State* pData, void* pAddress);
		static bool ROL(State* pData, void* pAddress);
		static bool LSR(State* pData, void* pAddress);
		static bool ROR(State* pData, void* pAddress);

		// Move commands
		static bool LDA(State* pData, void* pAddress);
		static bool STA(State* pData, void* pAddress);
		static bool LDX(State* pData, void* pAddress);
		static bool STX(State* pData, void* pAddress);
		static bool LDY(State* pData, void* pAddress);
		static bool STY(State* pData, void* pAddress);
		static bool TAX(State* pData, void* pAddress);
		static bool TXA(State* pData, void* pAddress);
		static bool TAY(State* pData, void* pAddress);
		static bool TYA(State* pData, void* pAddress);
		static bool TSX(State* pData, void* pAddress);
		static bool TXS(State* pData, void* pAddress);
		static bool PLA(State* pData, void* pAddress);
		static bool PHA(State* pData, void* pAddress);
		static bool PLP(State* pData, void* pAddress);
		static bool PHP(State* pData, void* pAddress);

		// Jump/Flag commands
		static bool BPL(State* pData, void* pAddress);
		static bool BMI(State* pData, void* pAddress);
		static bool BVC(State* pData, void* pAddress);
		static bool BVS(State* pData, void* pAddress);
		static bool BCC(State* pData, void* pAddress);
		static bool BCS(State* pData, void* pAddress);
		static bool BNE(State* pData, void* pAddress);
		static bool BEQ(State* pData, void* pAddress);
		static bool BRK(State* pData, void* pAddress);
		static bool RTI(State* pData, void* pAddress);
		static bool JSR(State* pData, void* pAddress);
		static bool RTS(State* pData, void* pAddress);
		static bool JMP(State* pData, void* pAddress);
		static bool BIT(State* pData, void* pAddress);
		static bool CLC(State* pData, void* pAddress);
		static bool SEC(State* pData, void* pAddress);
		static bool CLD(State* pData, void* pAddress);
		static bool SED(State* pData, void* pAddress);
		static bool CLI(State* pData, void* pAddress);
		static bool SEI(State* pData, void* pAddress);
		static bool CLV(State* pData, void* pAddress);
		static bool NOP(State* pData, void* pAddress);

		CPUmos6510();
		~CPUmos6510();

		void Reset();

		// Lock and unlock
		inline void Lock() const
		{
			if (m_pState != nullptr)
				m_pState->Lock();
		}

		inline void Unlock() const
		{
			if (m_pState != nullptr)
				m_pState->Unlock();
		}

		// Suspend
		inline bool IsSuspended() const
		{ 
			if(m_pState != nullptr && m_pState->IsOk())
				return m_pState->IsSuspended(); 

			return true;
		}

		inline void SetSuspended(bool bSuspend) 
		{
			if(m_pState != nullptr)
				m_pState->SetSuspended(bSuspend); 
		}

		// Memory
		inline void SetMemory(CPUMemory* pMemory)
		{
			if(m_pState != nullptr)
				m_pState->SetMemory(pMemory);
		}

		// Cycle counter
		inline void CycleCounterReset()
		{
			if(m_pState != nullptr)
				m_pState->ResetCycleCounter();
		}

		inline void CycleCounterSetCurrent(int iCycle)
		{
			if(m_pState != nullptr)
				m_pState->SetCycle(iCycle);
		}

		inline int CycleCounterGetCurrent()
		{
			if(m_pState != nullptr)
				return m_pState->GetCycle();

			return -1;
		}

		// Program counter
		inline void SetPC(unsigned short usAddress) 
		{ 
			if(m_pState != nullptr) 
			{
				m_pState->m_usPC = usAddress;
				m_pState->SetSuspended(false);
			}
		}

		inline void SetAccumulator(unsigned char inValue)
		{
			if (m_pState != nullptr)
				m_pState->m_ucRegA = inValue;
		}

		inline unsigned short GetPC()
		{
			if(m_pState != nullptr)
				return m_pState->m_usPC;

			return 0;
		}

		// Write callback
		inline void SetWriteCallback(ICPUWriteCallback* pCallback)
		{
			if(m_pState != nullptr)
				m_pState->SetWriteCallback(pCallback);
		}

		// Execution
		short ExecuteInstruction();

		// Opcode
		static const unsigned char GetOpcodeByteSize(const unsigned char inOpcode);
		static const AddressingMode GetOpcodeAddressingMode(const unsigned char inOpcode);

	private:
		// CPU State
		State* m_pState;
	};
}

#endif //__CPU_MOS6510_H__