#pragma once

/**
 * @brief Defines different register types in the ISA.
 */
namespace Revo::Register {

/* clang-format off */

/**
 * @brief General purpose registers
 */
enum class GPR : u8 {
    r0 = 0, r1, r2, r3, r4, r5, r6, r7,
    r8, r9, r10, r11, r12, r13, r14, r15,
    r16, r17, r18, r19, r20, r21, r22, r23,
    r24, r25, r26, r27, r28, r29, r30, r31
};

/**
 * @brief Floating point registers
 */
enum class FPR : u8 {
    f0 = 0, f1, f2, f3, f4, f5, f6, f7,
    f8, f9, f10, f11, f12, f13, f14, f15,
    f16, f17, f18, f19, f20, f21, f22, f23,
    f24, f25, f26, f27, f28, f29, f30, f31
};

/**
 * @brief Special purpose registers
 * 
 * @todo Read operations on Time Base Registers use different numbers
 * (TBL 268, TBU 269) and need to be implemented using some other mechanism
 * in the specification to be correct.
 */
enum class SPR : u16 {
    // --- General User Model UISA ---
    XER  = 1, ///< Integer Exception Register
    LR   = 8, ///< Link Register
    CTR  = 9, ///< Count Register

    // --- Decrementer ---
    // all alone :(
    DEC  = 22, ///< Decrementer

    // --- Save-Restore Registers ---
    SRR0 = 26, ///< Save-Restore Register 0
    SRR1 = 27, ///< Save-Restore Register 1

    // --- Standard System Control & Exceptions ---
    DSISR = 18,  ///< Data Storage Interrupt Status Register
    DAR   = 19,  ///< Data Address Register
    SDR1  = 25,  ///< Storage Description Register 1
    SPRG0 = 272, ///< SPR General 0
    SPRG1 = 273, ///< SPR General 1
    SPRG2 = 274, ///< SPR General 2
    SPRG3 = 275, ///< SPR General 3
    EAR   = 282, ///< External Access Register
    PVR   = 287, ///< Processor Version Register

    // --- Time Base Registers ---
    TBL = 284, ///< Time Base Lower (Write)
    TBU = 285, ///< Time Base Upper (Write)

    // --- Block Address Translations Registers ---
    IBAT0U = 528, ///< Instruction BAT 0 Upper
    IBAT0L = 529, ///< Instruction BAT 0 Lower
    IBAT1U = 530, ///< Instruction BAT 1 Upper
    IBAT1L = 531, ///< Instruction BAT 1 Lower
    IBAT2U = 532, ///< Instruction BAT 2 Upper
    IBAT2L = 533, ///< Instruction BAT 2 Lower
    IBAT3U = 534, ///< Instruction BAT 3 Upper
    IBAT3L = 535, ///< Instruction BAT 3 Lower
    IBAT4U = 560, ///< Instruction BAT 4 Upper
    IBAT4L = 561, ///< Instruction BAT 4 Lower
    IBAT5U = 562, ///< Instruction BAT 5 Upper
    IBAT5L = 563, ///< Instruction BAT 5 Lower
    IBAT6U = 564, ///< Instruction BAT 6 Upper
    IBAT6L = 565, ///< Instruction BAT 6 Lower
    IBAT7U = 566, ///< Instruction BAT 7 Upper
    IBAT7L = 567, ///< Instruction BAT 7 Lower
    DBAT0U = 536, ///< Data BAT 0 Upper
    DBAT0L = 537, ///< Data BAT 0 Lower
    DBAT1U = 538, ///< Data BAT 1 Upper
    DBAT1L = 539, ///< Data BAT 1 Lower
    DBAT2U = 540, ///< Data BAT 2 Upper
    DBAT2L = 541, ///< Data BAT 2 Lower
    DBAT3U = 542, ///< Data BAT 3 Upper
    DBAT3L = 543, ///< Data BAT 3 Lower
    DBAT4U = 568, ///< Data BAT 4 Upper
    DBAT4L = 569, ///< Data BAT 4 Lower
    DBAT5U = 570, ///< Data BAT 5 Upper
    DBAT5L = 571, ///< Data BAT 5 Lower
    DBAT6U = 572, ///< Data BAT 6 Upper
    DBAT6L = 573, ///< Data BAT 6 Lower
    DBAT7U = 574, ///< Data BAT 7 Upper
    DBAT7L = 575, ///< Data BAT 7 Lower

    // --- Graphics Quantization Registers ---
    GQR0 = 912, ///< Graphics Quantization Register 0
    GQR1 = 913, ///< Graphics Quantization Register 1
    GQR2 = 914, ///< Graphics Quantization Register 2
    GQR3 = 915, ///< Graphics Quantization Register 3
    GQR4 = 916, ///< Graphics Quantization Register 4
    GQR5 = 917, ///< Graphics Quantization Register 5
    GQR6 = 918, ///< Graphics Quantization Register 6
    GQR7 = 919, ///< Graphics Quantization Register 7

    // --- Hardware Implementation Dependent Registers ---
    HID2 = 920,  ///< Hardware Implementation Dependent Register 2
    HID0 = 1008, ///< Hardware Implementation Dependent Register 0
    HID1 = 1009, ///< Hardware Implementation Dependent Register 1
    HID4 = 1011, ///< Hardware Implementation Dependent Register 4

    // --- Write Pipe Address Register ---
    // all alone too, maybe him and DEC can be buddies? :)
    WPAR = 921, ///< Write Pipe Address Register

    // --- Direct Memory Access Registers ---
    DMAU = 922, ///< DMA Upper
    DMAL = 923, ///< DMA Lower

    // --- Breakpoint Registers ---
    IABR = 1010, ///< Instruction Address Breakpoint Register
    DABR = 1013, ///< Data Address Breakpoint Register

    // --- Cache Control Registers ---
    L2CR = 1017, ///< L2 Cache Control Register
    ICTC = 1019, ///< Instruction Cache Throttling Control Register

    // --- Thermal Management Registers ---
    THRM1 = 1020, ///< Thermal Management Register 1
    THRM2 = 1021, ///< Thermal Management Register 2
    THRM3 = 1022, ///< Thermal Management Register 3

    // --- Performance Monitor Registers ---
    MMCR0 = 952, ///< Monitor Mode Control Register 0 
    MMCR1 = 956, ///< Monitor Mode Control Register 1

    // --- Performance Monitor Counter Registers ---
    PMC1 = 953, ///< Performance Monitor Counter Register 1
    PMC2 = 954, ///< Performance Monitor Counter Register 2
    SIA  = 955, ///< Sampled Instruction Address Register
    PMC3 = 957, ///< Performance Monitor Counter Register 3
    PMC4 = 958, ///< Performance Monitor Counter Register 4

    // --- Performance Monitor Registers ---
    UMMCR0 = 936, ///< User Monitor Mode Control Register 0
    UPMC1  = 937, ///< User Performance Monitor Counter Register 1
    UPMC2  = 938, ///< User Performance Monitor Counter Register 2
    USIA   = 939, ///< User Sampled Instruction Address Register
    UMMCR1 = 940, ///< User Monitor Mode Control Register 1
    UPMC3  = 941, ///< User Performance Monitor Counter Register 3
    UPMC4  = 942, ///< User Performance Monitor Counter Register 4
};

/**
 * @brief Condition register fields cr0-cr7
 */
enum class CR : u8 {
    cr0 = 0, cr1, cr2, cr3, cr4, cr5, cr6, cr7
};

/* clang-format on */

} // namespace Revo::Register
