import structs_gen

# Generates the header file with the structs for every peripheral specified by a json file

template_path = "./sw/device/lib/drivers/template.tpl"     # template used for the output
json_files = []                                             # list of all the json files
output_files = []                                           # list of all the output file names
peripheral_names = []                                       # names of the peripherals for which the structs are generated

# dma
json_files.append("./hw/ip/dma/data/dma.hjson")
output_files.append("./sw/device/lib/drivers/dma/dma_structs.h")
peripheral_names.append("dma")

# fast_intr_ctrl
json_files.append("./hw/ip/fast_intr_ctrl/data/fast_intr_ctrl.hjson")
output_files.append("./sw/device/lib/drivers/fast_intr_ctrl/fast_intr_ctrl_structs.h")
peripheral_names.append("fast_intr_ctrl")

# power_manager
json_files.append("./hw/ip/power_manager/data/power_manager.hjson")
output_files.append("./sw/device/lib/drivers/power_manager/power_manager_structs.h")
peripheral_names.append("power_manager")

# soc_ctrl
json_files.append("./hw/ip/soc_ctrl/data/soc_ctrl.hjson")
output_files.append("./sw/device/lib/drivers/soc_ctrl/soc_ctrl_structs.h")
peripheral_names.append("soc_ctrl")


#### VENDOR ####

# gpio
json_files.append("./hw/vendor/pulp_platform_gpio/gpio_regs.hjson")
output_files.append("./sw/device/lib/drivers/gpio/gpio_structs.h")
peripheral_names.append("gpio")

# i2c
json_files.append("./hw/vendor/lowrisc_opentitan/hw/ip/i2c/data/i2c.hjson")
output_files.append("./sw/device/lib/drivers/i2c/i2c_structs.h")
peripheral_names.append("i2c")

# rv_plic
json_files.append("./hw/vendor/lowrisc_opentitan/hw/ip/rv_plic/data/rv_plic.hjson")
output_files.append("./sw/device/lib/drivers/rv_plic/rv_plic_structs.h")
peripheral_names.append("rv_plic")

# rv_timer
json_files.append("./hw/vendor/lowrisc_opentitan/hw/ip/rv_timer/data/rv_timer.hjson")
output_files.append("./sw/device/lib/drivers/rv_timer/rv_timer_structs.h")
peripheral_names.append("rv_timer")

# uart
json_files.append("./hw/vendor/lowrisc_opentitan/hw/ip/uart/data/uart.hjson")
output_files.append("./sw/device/lib/drivers/uart/uart_structs.h")
peripheral_names.append("uart")

# spi_host
json_files.append("./hw/vendor/lowrisc_opentitan_spi_host/data/spi_host.hjson")
output_files.append("./sw/device/lib/drivers/spi_host/spi_host_structs.h")
peripheral_names.append("spi_host")

# spi_device
# json_files.append("./hw/vendor/lowrisc_opentitan/hw/ip/spi_device/data/spi_device.hjson")
# output_files.append("./sw/device/lib/drivers/spi_memio/spi_memio_structs.h")
peripheral_names.append("spi_device")


for i in range(len(json_files)):

    structs_gen.main([  "--template_filename", template_path,
                        "--peripheral_name", peripheral_names[i],
                        "--json_filename", json_files[i], 
                        "--output_filename", output_files[i]],)