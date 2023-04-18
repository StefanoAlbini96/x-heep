// // Copyright lowRISC contributors.
// // Licensed under the Apache License, Version 2.0, see LICENSE for details.
// // SPDX-License-Identifier: Apache-2.0

// #ifndef OPENTITAN_SW_DEVICE_LIB_DIF_DIF_PLIC_H_
// #define OPENTITAN_SW_DEVICE_LIB_DIF_DIF_PLIC_H_

// /**
//  * @file
//  * @brief <a href="/hw/ip/rv_plic/doc/">PLIC</a> Device Interface Functions
//  *
//  * The PLIC should be largely compatible with the (currently draft) [RISC-V PLIC
//  * specification][plic], but tailored for the OpenTitan rv_plic register
//  * addresses. We intend to make the addresses compatible with the PLIC
//  * specification in the near future.
//  *
//  * [plic]: https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc
//  */

// #include <stdbool.h>
// #include <stdint.h>

// #include "mmio.h"
// #include "macros.h"

// #ifdef __cplusplus
// extern "C" {
// #endif  // __cplusplus

// /**
//  * A toggle state: enabled, or disabled.
//  *
//  * This enum may be used instead of a `bool` when describing an enabled/disabled
//  * state.
//  */
// typedef enum dif_plic_toggle {
//   /*
//    * The "enabled" state.
//    */
//   kDifPlicToggleEnabled,
//   /**
//    * The "disabled" state.
//    */
//   kDifPlicToggleDisabled,
// } dif_plic_toggle_t;

// /**
//  * Hardware instantiation parameters for PLIC.
//  *
//  * This struct describes information about the underlying hardware that is
//  * not determined until the hardware design is used as part of a top-level
//  * design.
//  */
// typedef struct dif_plic_params {
//   /**
//    * The base address for the PLIC hardware registers.
//    */
//   mmio_region_t base_addr;
// } dif_plic_params_t;

// /**
//  * A handle to PLIC.
//  *
//  * This type should be treated as opaque by users.
//  */
// typedef struct dif_plic {
//   dif_plic_params_t params;
// } dif_plic_t;

// /**
//  * The result of a PLIC operation.
//  */
// typedef enum dif_plic_result {
//   /**
//    * Indicates that the operation succeeded.
//    */
//   kDifPlicOk = 0,
//   /**
//    * Indicates some unspecified failure.
//    */
//   kDifPlicError = 1,
//   /**
//    * Indicates that some parameter passed into a function failed a
//    * precondition.
//    *
//    * When this value is returned, no hardware operations occurred.
//    */
//   kDifPlicBadArg = 2,
// } dif_plic_result_t;

// /**
//  * The lowest interrupt priority.
//  */
// extern const uint32_t kDifPlicMinPriority;

// /**
//  * The highest interrupt priority.
//  */
// extern const uint32_t kDifPlicMaxPriority;

// /**
//  * A PLIC interrupt source identifier.
//  *
//  * This corresponds to a specific interrupt, and not the device it originates
//  * from.
//  *
//  * This is an unsigned 32-bit value that is at least zero and is less than the
//  * `NumSrc` instantiation parameter of the `rv_plic` device.
//  *
//  * The value 0 corresponds to "No Interrupt".
//  */
// typedef uint32_t dif_plic_irq_id_t;

// /**
//  * A PLIC interrupt target.
//  *
//  * This corresponds to a specific system that can service an interrupt. In
//  * OpenTitan's case, that is the Ibex core. If there were multiple cores in the
//  * system, each core would have its own specific interrupt target ID.
//  *
//  * This is an unsigned 32-bit value that is at least 0 and is less than the
//  * `NumTarget` instantiation parameter of the `rv_plic` device.
//  */
// typedef uint32_t dif_plic_target_t;

// /**
//  * An interrupt trigger type.
//  */
// typedef enum dif_plic_irq_trigger {
//   /**
//    * Trigger on an edge (when the signal changes from low to high).
//    */
//   kDifPlicIrqTriggerEdge,
//   /**
//    * Trigger on a level (when the signal remains high).
//    */
//   kDifPlicIrqTriggerLevel,
// } dif_plic_irq_trigger_t;

// /**
//  * Enum for describing all the different types of interrupt
//  * sources that the rv_plic handles
// */
// typedef enum irq_sources
// {
//   IRQ_UART_SRC,   // from 1 to 8
//   IRQ_GPIO_SRC,   // from 9 to 32 
//   IRQ_I2C_SRC,    // from 33 to 48
//   IRQ_SPI_SRC,    // line 49
//   IRQ_BAD = -1    // default failure case
// } irq_sources_t;


// dif_plic_result_t plic_structInit(void);
// dif_plic_result_t plic_irq_set_priority(dif_plic_irq_id_t irq, uint32_t priority);
// dif_plic_result_t plic_irq_set_enabled(dif_plic_irq_id_t irq,
//                                        dif_plic_target_t target,
//                                        dif_plic_toggle_t state);
// dif_plic_result_t plic_irq_claim(dif_plic_target_t target,
//                                  dif_plic_irq_id_t *claim_data); 
// dif_plic_result_t plic_irq_complete(
//     dif_plic_target_t target,
//     const dif_plic_irq_id_t *complete_data);

// /**
//  * Creates a new handle for PLIC.
//  *
//  * This function does not actuate the hardware.
//  *
//  * @param params Hardware instantiation parameters.
//  * @param[out] plic Out param for the initialized handle.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_init(dif_plic_params_t params, dif_plic_t *plic);

// /**
//  * Returns whether a particular interrupt is currently pending.
//  *
//  * @param plic A PLIC handle.
//  * @param irq An interrupt type.
//  * @param[out] is_pending Out-param for whether the interrupt is pending.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_is_pending(const dif_plic_t *plic,
//                                           dif_plic_irq_id_t irq,
//                                           bool *is_pending);

// /**
//  * Checks whether a particular interrupt is currently enabled or disabled.
//  *
//  * @param plic A PLIC handle.
//  * @param irq An interrupt type.
//  * @param target An interrupt target.
//  * @param[out] state Out-param toggle state of the interrupt.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_get_enabled(const dif_plic_t *plic,
//                                            dif_plic_irq_id_t irq,
//                                            dif_plic_target_t target,
//                                            dif_plic_toggle_t *state);

// /**
//  * Sets whether a particular interrupt is currently enabled or disabled.
//  *
//  * This operation does not affect IRQ generation in `target`, which
//  * must be configured in the corresponding peripheral itself.
//  *
//  * @param plic A PLIC handle.
//  * @param irq An interrupt type.
//  * @param target An interrupt target.
//  * @param state The new toggle state for the interrupt.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_set_enabled(const dif_plic_t *plic,
//                                            dif_plic_irq_id_t irq,
//                                            dif_plic_target_t target,
//                                            dif_plic_toggle_t state);

// /**
//  * Sets the IRQ trigger type.
//  *
//  * Sets the behaviour of the Interrupt Gateway for a particular IRQ to be edge
//  * or level triggered.
//  *
//  * @param plic A PLIC handle.
//  * @param irq An interrupt type.
//  * @param trigger A trigger type.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_set_trigger(const dif_plic_t *plic,
//                                            dif_plic_irq_id_t irq,
//                                            dif_plic_irq_trigger_t trigger);

// /**
//  * Sets IRQ source priority (0-3).
//  *
//  * In order for the PLIC to set a Claim/Complete register and assert the
//  * external interrupt line to the target (Ibex, ...), the priority of the IRQ
//  * source must be higher than the threshold for this source.
//  *
//  * @param plic A PLIC handle.
//  * @param irq An interrupt type.
//  * @param priority Priority to set.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_set_priority(const dif_plic_t *plic,
//                                             dif_plic_irq_id_t irq,
//                                             uint32_t priority);

// /**
//  * Sets the target priority threshold.
//  *
//  * Sets the target priority threshold. PLIC will only interrupt a target when
//  * IRQ source priority is set higher than the priority threshold for the
//  * corresponding target.
//  *
//  * @param plic A PLIC handle.
//  * @param target Target to set the IRQ priority threshold for.
//  * @param threshold IRQ priority threshold to be set.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_target_set_threshold(const dif_plic_t *plic,
//                                                 dif_plic_target_t target,
//                                                 uint32_t threshold);

// /**
//  * Claims an IRQ and gets the information about the source.
//  *
//  * Claims an IRQ and returns the IRQ related data to the caller. This function
//  * reads a target specific Claim/Complete register. #dif_plic_irq_complete must
//  * be called in order to allow another interrupt with the same source id to be
//  * delivered. This usually would be done once the interrupt has been serviced.
//  *
//  * Another IRQ can be claimed before a prior IRQ is completed. In this way, this
//  * functionality is compatible with nested interrupt handling. The restriction
//  * is that you must Complete a Claimed IRQ before you will be able to claim an
//  * IRQ with the same ID. This allows a pair of Claim/Complete calls to be
//  * overlapped with another pair -- and there is no requirement that the
//  * interrupts should be Completed in the reverse order of when they were
//  * Claimed.
//  *
//  * @see #dif_plic_irq_complete
//  *
//  * @param plic A PLIC handle.
//  * @param target Target that claimed the IRQ.
//  * @param[out] claim_data Data that describes the origin of the IRQ.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_claim(const dif_plic_t *plic,
//                                      dif_plic_target_t target,
//                                      dif_plic_irq_id_t *claim_data);

// /**
//  * Completes the claimed IRQ.
//  *
//  * Finishes servicing of the claimed IRQ by writing the IRQ source ID back to a
//  * target specific Claim/Complete register. This function must be called after
//  * #dif_plic_irq_claim, when the caller is prepared to service another IRQ with
//  * the same source ID. If a source ID is never Completed, then when future
//  * interrupts are Claimed, they will never have the source ID of the uncompleted
//  * IRQ.
//  *
//  * @see #dif_plic_irq_claim
//  *
//  * @param plic A PLIC handle.
//  * @param target Target that claimed the IRQ.
//  * @param[out] complete_data Previously claimed IRQ data that is used to signal
//  * PLIC of the IRQ servicing completion.
//  * @return The result of the operation.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_irq_complete(const dif_plic_t *plic,
//                                         dif_plic_target_t target,
//                                         const dif_plic_irq_id_t *complete_data);

// /**
//  * Forces the software interrupt for a particular target.
//  *
//  * This function causes an interrupt to the `target` HART to be serviced as if
//  * hardware had asserted it.
//  *
//  * This function allows to synchronise between the HARTs, which otherwise
//  * would not be possible due to HART being only able to access own CSRs.
//  * NOTE: this is not an issue on Ibex, as it has only one HART.
//  *
//  * An interrupt handler is expected to call `dif_plic_software_irq_acknowledge`
//  * when the interrupt has been handled.
//  *
//  * @param plic PLIC state data.
//  * @param target Target HART.
//  * @return `dif_plic_result_t`.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_software_irq_force(const dif_plic_t *plic,
//                                               dif_plic_target_t target);

// /**
//  * Acknowledges the software interrupt for a particular target.
//  *
//  * This function indicates to the hardware that the software interrupt has been
//  * successfully serviced. It is expected to be called from a software interrupt
//  * handler.
//  *
//  * @param plic PLIC state data.
//  * @param target Target HART.
//  * @return `dif_plic_result_t`.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_software_irq_acknowledge(const dif_plic_t *plic,
//                                                     dif_plic_target_t target);

// /**
//  * Returns software interrupt pending state for a particular target.
//  *
//  * @param plic PLIC state data.
//  * @param target Target HART.
//  * @param[out] is_pending Flag indicating whether the interrupt is pending.
//  * @return `dif_plic_result_t`.
//  */
// DIF_WARN_UNUSED_RESULT
// dif_plic_result_t dif_plic_software_irq_is_pending(const dif_plic_t *plic,
//                                                    dif_plic_target_t target,
//                                                    bool *is_pending);

// #ifdef __cplusplus
// }  // extern "C"
// #endif  // __cplusplus

// #endif  // OPENTITAN_SW_DEVICE_LIB_DIF_DIF_PLIC_H_



// void plic_prio_test(int irq, int prio);

// void plic_prio_test(int irq, int prio)
// {
//   uint32_t *reg_ptr = &rv_plic.rv_plic_ms.PRIO0;  // address of the first one
//   uint32_t *goal_ptr = &rv_plic.rv_plic_ms.PRIO1; // testing

//   uint8_t offset = (irq * sizeof(rv_plic.rv_plic_ms.PRIO0));  // byte distance from the base address
//   uint8_t dist_diff = goal_ptr - reg_ptr;

//   ptrdiff_t prio_reg_size = sizeof(rv_plic.rv_plic_ms.PRIO0);

//   int prio0_test = 42;
//   rv_plic.rv_plic_ms.PRIO0.w = prio0_test;
//   // ptrdiff_t *final_add = reg_ptr + offset;
//   ptrdiff_t *final_add = reg_ptr + (offset / prio_reg_size);
//   *final_add = prio;                          // write into final register

//   printf("----------------------\n");
//   printf("Size reg struct:\t%d\n", sizeof(rv_plic.rv_plic_ms.PRIO0));
//   printf("PRIO TEST:\nTest reg: PRIO%d\nTest value: %d\n", irq, prio);
//   printf("Base address:\t%p\n", reg_ptr);
//   printf("Goal address:\t%p\n", goal_ptr);
//   printf("Offset:\t\t%d  |  %p\n", offset, offset);
//   printf("Distance:\t%d  |  %p\n", dist_diff, dist_diff);

//   printf("Final addr:\t%p\n", reg_ptr+(offset/prio_reg_size));
//   printf("Final addr:\t%p\n", final_add);
//   printf("Final addr:\t%p\n", reg_ptr+dist_diff);
//   printf("\n\n");
//   printf("Write PRIO0:\t%d\nWrite PRIO%d:\t%d\n", prio0_test, irq, prio);
//   printf("Read PRIO0:\t%d\nRead PRIO%d:\t%d\n", *(reg_ptr), irq, rv_plic.rv_plic_ms.PRIO1.w);
//   printf("\n");
//   printf("Read bit:\t%d\n", rv_plic.rv_plic_ms.PRIO1.b.PRIO1);
//   printf("Read word:\t%d\n", rv_plic.rv_plic_ms.PRIO1.b._reserved);
//   printf("%d\n", *(reg_ptr + offset));
//   printf("----------------------\n\n");
// }


// ////////////////////////////////////////////////////////////////////////////////////////////
// void test_foo(int32_t value);
// void test_foo(int32_t value){

//   ptrdiff_t offset = plic_priority_reg_offset(0); // PRIO0 register
//   mmio_region_write32(rv_plic.rv_plic_base_addr, offset, value);
//   rv_plic.rv_plic_ms.PRIO0.w = mmio_region_read32(rv_plic.rv_plic_base_addr, offset);

//   printf("\n\n------------------------------------------\n");
//   printf("PRIO0 reg:\t\t%d\n", rv_plic.rv_plic_ms.PRIO0.w);
//   printf("PRIO0 bit:\t\t%d\n", rv_plic.rv_plic_ms.PRIO0.b.PRIO0);
//   printf("PRIO0 reserved:\t\t%d\n", rv_plic.rv_plic_ms.PRIO0.b._reserved);
//   printf("------------------------------------------\n");
// }
// ////////////////////////////////////////////////////////////////////////////////////////////
