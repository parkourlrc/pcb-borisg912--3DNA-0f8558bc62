idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES driver audio bases gyro i2c input powermgmt visual hardware personalization
)
