/* proceso3.s */
.global transmit_data

.text

# a0 = temperatura actual
transmit_data:
    # Reservar espacio y guardar registros callee-saved (ni idea que es pero funciona xd)
    addi sp, sp, -16
    sw ra, 12(sp)
    sw s0, 8(sp)

    mv s0, a0           # Guardar temperatura en s0 

    # Imprimir temperatura como numero entero
    mv a0, s0           # Pasar temperatura como argumento
    call print_int      # Llamar a la rutina print

    # Restaurar registros y liberar stack
    lw s0, 8(sp)
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

# Convierte un entero en ASCII y lo imprime usando ecall (write).
# Este print_int es local a este archivo (no usa print_msg de io_driver.s).
print_int:
    addi sp, sp, -32     # Reservar buffer de 32 bytes en stack
    sw ra, 28(sp)        # Guardar ra
    
    addi t1, sp, 26      # t1 = puntero al final del buffer
    li t2, 10            # Divisor decimal
    mv t3, a0            # Copia del numero a convertir

convert_loop:
    rem t4, t3, t2       # t4 = ultimo digito (resto)
    div t3, t3, t2       # t3 = numero/10
    addi t4, t4, 48      # Convertir digito a ASCII
    addi t1, t1, -1      # Retroceder en buffer
    sb t4, 0(t1)         # Guardar caracter
    bnez t3, convert_loop   # Repetir hasta que el numero sea 0
    
    # Imprimir numero convertido
    li a7, 64            # sys_write
    li a0, 1             # stdout
    mv a1, t1            # a1 = inicio del string
    addi t5, sp, 26
    sub a2, t5, t1       # Longitud = (fin-inicio)
    ecall

    lw ra, 28(sp)        # Restaurar ra
    addi sp, sp, 32      # Liberar buffer
    ret
