/* asm_add.s — Thumb-2 Assembly Functions
 * Deepanshu — Module 2: ISA & Registers
 *
 * ARM AAPCS calling convention:
 *   R0 = first argument  → return value
 *   R1 = second argument
 *   BX LR = return to caller (Thumb mode)
 *
 * .syntax unified = Thumb-2 mode (mix 16-bit + 32-bit instructions)
 * .thumb          = Cortex-M always in Thumb state
 * .global         = visible from C code (linker can find it)
 */

    .syntax unified
    .thumb
    .text

/* ─────────────────────────────────────────────────────────
 * uint32_t asm_add(uint32_t a, uint32_t b)
 *
 * R0 = a  (first arg — AAPCS)
 * R1 = b  (second arg — AAPCS)
 * ADD R0, R0, R1   → R0 = a + b
 * BX  LR           → return R0 to caller
 * ─────────────────────────────────────────────────────────
 */
    .global asm_add
    .type   asm_add, %function
asm_add:
    ADD     R0, R0, R1      @ R0 = a + b  (Thumb-2 32-bit ADD)
    BX      LR              @ Return — LR = caller's return address
    .size   asm_add, .-asm_add


/* ─────────────────────────────────────────────────────────
 * uint32_t asm_mul(uint32_t a, uint32_t b)
 *
 * Thumb-2 MUL instruction — 32-bit result (lower 32 bits)
 * R0 = a * b
 * ─────────────────────────────────────────────────────────
 */
    .global asm_mul
    .type   asm_mul, %function
asm_mul:
    MUL     R0, R0, R1      @ R0 = a * b  (Thumb-2 MUL)
    BX      LR
    .size   asm_mul, .-asm_mul


/* ─────────────────────────────────────────────────────────
 * uint32_t asm_and(uint32_t a, uint32_t b)
 *
 * Bitwise AND — logical instruction demo
 * ─────────────────────────────────────────────────────────
 */
    .global asm_and
    .type   asm_and, %function
asm_and:
    AND     R0, R0, R1      @ R0 = a & b  (bitwise AND)
    BX      LR
    .size   asm_and, .-asm_and
