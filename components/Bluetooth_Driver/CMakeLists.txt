idf_component_register(
    SRCS "Bluetooth_WYP.cpp" "ble_gatt.cpp"
    INCLUDE_DIRS "include"
    REQUIRES 
        driver 
        bt 
        nvs_flash  
        log
)
