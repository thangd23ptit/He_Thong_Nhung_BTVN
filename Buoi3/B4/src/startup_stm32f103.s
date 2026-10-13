/*
 * startup_stm32f103.s
 * Bare-metal startup code for STM32F103 Cortex-M3
 */

.syntax unified
.cpu cortex-m3
.thumb

.global g_pfnVectors
.global Reset_Handler
.global Default_Handler

.extern main

/* ============================================================
 * Interrupt vector table
 * ============================================================ */
.section .isr_vector, "a", %progbits
.type g_pfnVectors, %object

g_pfnVectors:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* STM32F103 medium-density IRQ0..IRQ59 */
    .rept 11                        /* IRQ0..IRQ10 */
    .word Default_Handler
    .endr
    .word DMA1_Channel1_IRQHandler  /* IRQ11 = DMA1 Channel1 (ADC1) */
    .rept 48                        /* IRQ12..IRQ59 */
    .word Default_Handler
    .endr

.size g_pfnVectors, .-g_pfnVectors

/* ============================================================
 * Reset handler
 * ============================================================ */
.section .text.Reset_Handler, "ax", %progbits
.type Reset_Handler, %function

Reset_Handler:
    /* Copy .data from Flash to RAM */
    ldr r0, =_sdata
    ldr r1, =_edata
    ldr r2, =_sidata

.Ldata_copy:
    cmp r0, r1
    bcs .Lbss_init

    ldr r3, [r2]
    str r3, [r0]

    adds r0, r0, #4
    adds r2, r2, #4

    b .Ldata_copy

    /* Clear .bss */
.Lbss_init:
    ldr r0, =_sbss
    ldr r1, =_ebss
    movs r2, #0

.Lbss_clear:
    cmp r0, r1
    bcs .Lcall_main

    str r2, [r0]
    adds r0, r0, #4

    b .Lbss_clear

    /* Call main() */
.Lcall_main:
    bl main

    /* main() should not return */
.Lhang:
    b .Lhang

.size Reset_Handler, .-Reset_Handler

/* ============================================================
 * Default interrupt handler
 * ============================================================ */
.section .text.Default_Handler, "ax", %progbits
.type Default_Handler, %function

Default_Handler:
    b .

.size Default_Handler, .-Default_Handler

/* Weak aliases */
.weak NMI_Handler
.thumb_set NMI_Handler, Default_Handler

.weak HardFault_Handler
.thumb_set HardFault_Handler, Default_Handler

.weak MemManage_Handler
.thumb_set MemManage_Handler, Default_Handler

.weak BusFault_Handler
.thumb_set BusFault_Handler, Default_Handler

.weak UsageFault_Handler
.thumb_set UsageFault_Handler, Default_Handler

.weak SVC_Handler
.thumb_set SVC_Handler, Default_Handler

.weak DebugMon_Handler
.thumb_set DebugMon_Handler, Default_Handler

.weak PendSV_Handler
.thumb_set PendSV_Handler, Default_Handler

.weak SysTick_Handler
.thumb_set SysTick_Handler, Default_Handler

.weak DMA1_Channel1_IRQHandler
.thumb_set DMA1_Channel1_IRQHandler, Default_Handler

.section .note.GNU-stack, "", %progbits