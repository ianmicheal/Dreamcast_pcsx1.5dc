/*
 * ix86 definitions v0.4.3
 *  Author: linuzappz <linuzappz@pcsx.net>
 */

#ifndef __IX86_H__
#define __IX86_H__

// include basic types
#include "PsxCommon.h"

/* general defines */

#define EAX 0
#define EBX 3
#define ECX 1
#define EDX 2
#define ESI 6
#define EDI 7
#define EBP 5
#define ESP 4

#define MM0 0
#define MM1 1
#define MM2 2
#define MM3 3
#define MM4 4
#define MM5 5
#define MM6 6
#define MM7 7

s8  *x86Ptr;
u8  *j8Ptr[32];
u32 *j32Ptr[32];

void x86Init(char *ptr);
void x86Shutdown();

void x86SetJ8(u8 *j8);
void x86SetJ32(u32 *j32);
void x86Align(int bytes);


/********************/
/* IX86 intructions */
/********************/

/*
 * scale values:
 *  0 - *1
 *  1 - *2
 *  2 - *4
 *  3 - *8
 */

////////////////////////////////////
// mov instructions                /
////////////////////////////////////

/* mov r32 to r32 */
void MOV32RtoR(int to, int from);
/* mov r32 to m32 */
void MOV32RtoM(u32 to, int from);
/* mov m32 to r32 */
void MOV32MtoR(int to, u32 from);
/* mov [r32] to r32 */
void MOV32RmtoR(int to, int from);
/* mov [r32][r32*scale] to r32 */
void MOV32RmStoR(int to, int from, int from2, int scale);
/* mov r32 to [r32] */
void MOV32RtoRm(int to, int from);
/* mov imm32 to r32 */
void MOV32ItoR(int to, u32 from);
/* mov imm32 to m32 */
void MOV32ItoM(u32 to, u32 from);

/* mov r16 to m16 */
void MOV16RtoM(u32 to, int from);
/* mov m16 to r16 */
void MOV16MtoR(int to, u32 from);
/* mov imm16 to m16 */
void MOV16ItoM(u32 to, u16 from);

/* mov r8 to m8 */
void MOV8RtoM(u32 to, int from);
/* mov m8 to r8 */
void MOV8MtoR(int to, u32 from);
/* mov imm8 to m8 */
void MOV8ItoM(u32 to, u8 from);

/* movsx r8 to r32 */
void MOVSX32R8toR(int to, int from);
/* movsx m8 to r32 */
void MOVSX32M8toR(int to, u32 from);
/* movsx r16 to r32 */
void MOVSX32R16toR(int to, int from);
/* movsx m16 to r32 */
void MOVSX32M16toR(int to, u32 from);

/* movzx r8 to r32 */
void MOVZX32R8toR(int to, int from);
/* movzx m8 to r32 */
void MOVZX32M8toR(int to, u32 from);
/* movzx r16 to r32 */
void MOVZX32R16toR(int to, int from);
/* movzx m16 to r32 */
void MOVZX32M16toR(int to, u32 from);

/* cmovne r32 to r32 */
void CMOVNE32RtoR(int to, int from);
/* cmovne m32 to r32*/
void CMOVNE32MtoR(int to, u32 from);
/* cmove r32 to r32*/
void CMOVE32RtoR(int to, int from);
/* cmove m32 to r32*/
void CMOVE32MtoR(int to, u32 from);
/* cmovg r32 to r32*/
void CMOVG32RtoR(int to, int from);
/* cmovg m32 to r32*/
void CMOVG32MtoR(int to, u32 from);
/* cmovge r32 to r32*/
void CMOVGE32RtoR(int to, int from);
/* cmovge m32 to r32*/
void CMOVGE32MtoR(int to, u32 from);
/* cmovl r32 to r32*/
void CMOVL32RtoR(int to, int from);
/* cmovl m32 to r32*/
void CMOVL32MtoR(int to, u32 from);
/* cmovle r32 to r32*/
void CMOVLE32RtoR(int to, int from);
/* cmovle m32 to r32*/
void CMOVLE32MtoR(int to, u32 from);

////////////////////////////////////
// arithmetic instructions         /
////////////////////////////////////

/* add imm32 to r32 */
void ADD32ItoR(int to, u32 from);
/* add imm32 to m32 */
void ADD32ItoM(u32 to, u32 from);
/* add r32 to r32 */
void ADD32RtoR(int to, int from);
/* add r32 to m32 */
void ADD32RtoM(u32 to, int from);
/* add m32 to r32 */
void ADD32MtoR(int to, u32 from);

/* adc imm32 to r32 */
void ADC32ItoR(int to, u32 from);
/* adc r32 to r32 */
void ADC32RtoR(int to, int from);
/* adc m32 to r32 */
void ADC32MtoR(int to, u32 from);

/* inc r32 */
void INC32R(int to);
/* inc m32 */
void INC32M(u32 to);

/* sub imm32 to r32 */
void SUB32ItoR(int to, u32 from);
/* sub r32 to r32 */
void SUB32RtoR(int to, int from);
/* sub m32 to r32 */
void SUB32MtoR(int to, u32 from);

/* sbb imm32 to r32 */
void SBB32ItoR(int to, u32 from);
/* sbb r32 to r32 */
void SBB32RtoR(int to, int from);
/* sbb m32 to r32 */
void SBB32MtoR(int to, u32 from);

/* dec r32 */
void DEC32R(int to);
/* dec m32 */
void DEC32M(u32 to);

/* mul eax by r32 to edx:eax */
void MUL32R(int from);
/* mul eax by m32 to edx:eax */
void MUL32M(u32 from);

/* imul eax by r32 to edx:eax */
void IMUL32R(int from);
/* imul eax by m32 to edx:eax */
void IMUL32M(u32 from);

/* div eax by r32 to edx:eax */
void DIV32R(int from);
/* div eax by m32 to edx:eax */
void DIV32M(u32 from);

/* idiv eax by r32 to edx:eax */
void IDIV32R(int from);
/* idiv eax by m32 to edx:eax */
void IDIV32M(u32 from);

////////////////////////////////////
// shifting instructions           /
////////////////////////////////////

/* shl imm8 to r32 */
void SHL32ItoR(int to, u8 from);
/* shl cl to r32 */
void SHL32CLtoR(int to);

/* shr imm8 to r32 */
void SHR32ItoR(int to, u8 from);
/* shr cl to r32 */
void SHR32CLtoR(int to);

/* sar imm8 to r32 */
void SAR32ItoR(int to, u8 from);
/* sar cl to r32 */
void SAR32CLtoR(int to);

/* sal imm8 to r32 */
#define SAL32ItoR SHL32ItoR
/* sal cl to r32 */
#define SAL32CLtoR SHL32CLtoR

// logical instructions

/* or imm32 to r32 */
void OR32ItoR(int to, u32 from);
/* or imm32 to m32 */
void OR32ItoM(u32 to, u32 from);
/* or r32 to r32 */
void OR32RtoR(int to, int from);
/* or r32 to m32 */
void OR32RtoM(u32 to, int from);
/* or m32 to r32 */
void OR32MtoR(int to, u32 from);

/* xor imm32 to r32 */
void XOR32ItoR(int to, u32 from);
/* xor imm32 to m32 */
void XOR32ItoM(u32 to, u32 from);
/* xor r32 to r32 */
void XOR32RtoR(int to, int from);
/* xor r32 to m32 */
void XOR32RtoM(u32 to, int from);
/* xor m32 to r32 */
void XOR32MtoR(int to, u32 from);

/* and imm32 to r32 */
void AND32ItoR(int to, u32 from);
/* and imm32 to m32 */
void AND32ItoM(u32 to, u32 from);
/* and r32 to r32 */
void AND32RtoR(int to, int from);
/* and r32 to m32 */
void AND32RtoM(u32 to, int from);
/* and m32 to r32 */
void AND32MtoR(int to, u32 from);

/* not r32 */
void NOT32R(int from);
/* neg r32 */
void NEG32R(int from);

////////////////////////////////////
// jump instructions               /
////////////////////////////////////

/* jmp rel8 */
u8*  JMP8(u8 to);

/* jmp rel32 */
u32* JMP32(u32 to);
/* jmp r32 */
void JMP32R(int to);

/* je rel8 */
u8*  JE8(u8 to);
/* jz rel8 */
u8*  JZ8(u8 to);
/* jg rel8 */
u8*  JG8(u8 to);
/* jge rel8 */
u8*  JGE8(u8 to);
/* jl rel8 */
u8*  JL8(u8 to);
/* jle rel8 */
u8*  JLE8(u8 to);
/* jne rel8 */
u8*  JNE8(u8 to);
/* jnz rel8 */
u8*  JNZ8(u8 to);
/* jng rel8 */
u8*  JNG8(u8 to);
/* jnge rel8 */
u8*  JNGE8(u8 to);
/* jnl rel8 */
u8*  JNL8(u8 to);
/* jnle rel8 */
u8*  JNLE8(u8 to);

/* je rel32 */
u32* JE32(u32 to);
/* jz rel32 */
u32* JZ32(u32 to);
/* jg rel32 */
u32* JG32(u32 to);
/* jge rel32 */
u32* JGE32(u32 to);
/* jl rel32 */
u32* JL32(u32 to);
/* jle rel32 */
u32* JLE32(u32 to);
/* jne rel32 */
u32* JNE32(u32 to);
/* jnz rel32 */
u32* JNZ32(u32 to);
/* jng rel32 */
u32* JNG32(u32 to);
/* jnge rel32 */
u32* JNGE32(u32 to);
/* jnl rel32 */
u32* JNL32(u32 to);
/* jnle rel32 */
u32* JNLE32(u32 to);

/* call func */
void CALLFunc(u32 func); // based on CALL32
/* call rel32 */
void CALL32(u32 to);
/* call r32 */
void CALL32R(int to);
/* call m32 */
void CALL32M(u32 to);

////////////////////////////////////
// misc instructions               /
////////////////////////////////////

/* cmp imm32 to r32 */
void CMP32ItoR(int to, u32 from);
/* cmp imm32 to m32 */
void CMP32ItoM(u32 to, u32 from);
/* cmp r32 to r32 */
void CMP32RtoR(int to, int from);
/* cmp m32 to r32 */
void CMP32MtoR(int to, u32 from);

/* test imm32 to r32 */
void TEST32ItoR(int to, u32 from);
/* test r32 to r32 */
void TEST32RtoR(int to, int from);

/* setl r8 */
void SETL8R(int to);
/* setb r8 */
void SETB8R(int to);

/* cbw */
void CBW();
/* cwd */
void CWD();
/* cdq */
void CDQ();

/* push r32 */
void PUSH32R(int from);
/* push m32 */
void PUSH32M(u32 from);
/* push imm32 */
void PUSH32I(u32 from);

/* pop r32 */
void POP32R(int from);

/* pushad */
void PUSHA32();
/* popad */
void POPA32();

/* ret */
void RET();

/********************/
/* FPU instructions */
/********************/

/* fild m32 to fpu reg stack */
void FILD32(u32 from);
/* fistp m32 from fpu reg stack */
void FISTP32(u32 from);
/* fld m32 to fpu reg stack */
void FLD32(u32 from);
/* fstp m32 from fpu reg stack */
void FSTP32(u32 to);

/* fldcw fpu control word from m16 */
void FLDCW(u32 from);
/* fstcw fpu control word to m16 */
void FNSTCW(u32 to);

/* fadd m32 to fpu reg stack */
void FADD32(u32 from);
/* fsub m32 to fpu reg stack */
void FSUB32(u32 from);
/* fmul m32 to fpu reg stack */
void FMUL32(u32 from);
/* fdiv m32 to fpu reg stack */
void FDIV32(u32 from);
/* fabs fpu reg stack */
void FABS();
/* fsqrt fpu reg stack */
void FSQRT();
/* fchs fpu reg stack */
void FCHS();

/********************/
/* MMX instructions */
/********************/

// r64 = mm

/* movq m64 to r64 */
void MOVQMtoR(int to, u32 from);
/* movq r64 to m64 */
void MOVQRtoM(u32 to, int from);

/* pand r64 to r64 */
void PANDRtoR(int to, int from);

/* por r64 to r64 */
void PORRtoR(int to, int from);
/* por m64 to r64 */
void PORMtoR(int to, u32 from);

/* pxor r64 to r64 */
void PXORRtoR(int to, int from);

/* psllq r64 to r64 */
void PSLLQRtoR(int to, int from);
/* psllq m64 to r64 */
void PSLLQMtoR(int to, u32 from);
/* psllq imm8 to r64 */
void PSLLQItoR(int to, u8 from);

/* psrlq r64 to r64 */
void PSRLQRtoR(int to, int from);
/* psrlq m64 to r64 */
void PSRLQMtoR(int to, u32 from);
/* psrlq imm8 to r64 */
void PSRLQItoR(int to, u8 from);

/* paddusb r64 to r64 */
void PADDUSBRtoR(int to, int from);
/* paddusb m64 to r64 */
void PADDUSBMtoR(int to, u32 from);
/* paddusw r64 to r64 */
void PADDUSWRtoR(int to, int from);
/* paddusw m64 to r64 */
void PADDUSWMtoR(int to, u32 from);

/* paddb r64 to r64 */
void PADDBRtoR(int to, int from);
/* paddb m64 to r64 */
void PADDBMtoR(int to, u32 from);
/* paddw r64 to r64 */
void PADDWRtoR(int to, int from);
/* paddw m64 to r64 */
void PADDWMtoR(int to, u32 from);
/* paddd r64 to r64 */
void PADDDRtoR(int to, int from);
/* paddd m64 to r64 */
void PADDDMtoR(int to, u32 from);

/* emms */
void EMMS();


#endif /* __IX86_H__ */
