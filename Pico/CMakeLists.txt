cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)

project(dma_capture)

add_executable(dma_capture
        dma_capture.c
        )


pico_sdk_init()

pico_generate_pio_header(dma_capture ${CMAKE_CURRENT_LIST_DIR}/resistor_dac.pio)

target_link_libraries(dma_capture
		pico_stdlib
		hardware_adc
		hardware_dma
		# For the dummy output:
		hardware_pio
		pico_multicore
		)

pico_enable_stdio_usb(dma_capture 1)
pico_enable_stdio_uart(dma_capture 0)
pico_add_extra_outputs(dma_capture)


