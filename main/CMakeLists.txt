idf_component_register(SRCS 
                        "macero.c"
                        "ble.c"
                    INCLUDE_DIRS 
                        "." 
                        "../components/ssd1306"

                    REQUIRES
                        driver)
