/* proceso1.s */
# Lee del archivo a traves del driver para obtener los datos.
.global get_sensor_temp
# Funcion externa 
.global read_int 
.text

# Retorna: a0 = temperatura leida
get_sensor_temp:
    # Guardamos ra en el stack porque esta función llama a otra
    addi sp, sp, -16
    sw ra, 12(sp)
    
    # Llamamos al driver (io_driver.s)
    call read_int
    
    # Recuperamos ra y retornamos
    lw ra, 12(sp)
    addi sp, sp, 16
    ret
