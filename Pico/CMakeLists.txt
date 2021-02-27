cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
# We also need PICO EXTRAS
include(pico_extras_import.cmake)
project(dma_capture)

add_executable(dma_capture
        dma_capture.c
        )


pico_sdk_init()

target_link_libraries(dma_capture
		pico_stdlib
		hardware_adc
		hardware_dma
		# For the dummy output:
		pico_multicore
		pico_audio_pwm
		)

pico_enable_stdio_usb(dma_capture 1)
pico_enable_stdio_uart(dma_capture 0)
pico_add_extra_outputs(dma_capture)


