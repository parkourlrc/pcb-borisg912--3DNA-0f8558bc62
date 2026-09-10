idf_component_register(SRCS "visual.c"
                    INCLUDE_DIRS "include")

# Add dependency on Espressif's LED strip driver
set(EXTRA_COMPONENT_DIRS $ENV{IDF_PATH}/components/led_strip)
