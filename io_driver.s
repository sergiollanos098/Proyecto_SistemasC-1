/* io_driver.s */
# I/O y telemtria
.global read_int
.global print_msg
.global get_time
.global get_current_pc
.global print_int      

.text

# print_msg(char* str, int len)
# Imprime una cadena de longitud fija usando ecall.
# a0 = file descriptor (1 = stdout)
# a1 = puntero a string
# a2 = longitud
# a7 = 64 (sys_write)
print_msg:
    li a7, 64 # Codigo de write
    mv a2, a1 # Longitud
    mv a1, a0 # Puntero a la cadena
    li a0, 1 # FileDescriptor = stdout
    ecall
    ret

# read_int() -> int
# Lee caracteres desde stdin hasta formar un entero.
# Ignora caracteres no numericos excepto '\n' (10) o '\r' (13) que terminan lectura.
# Devuelve el numero en a0.
read_int:
    addi sp, sp, -16 # Reserva espacio y guarda registros
    sw ra, 12(sp)
    sw s0, 8(sp)
    li s0, 0  # Acumulador del numero
    li t1, 10  # Base decimal
read_loop:
    # sys_read: lee 1 byte desde stdin en (sp+4)
    li a7, 63            # Codigo read
    li a0, 0             # FD = stdin
    addi a1, sp, 4       # Dirección de almacenamiento
    li a2, 1             # Leer 1 byte
    ecall

    beqz a0, end_read    # Si no se leyo nada, terminar

    lb t2, 4(sp)         # t2 = caracter leido

    # Si es '\n' terminamos
    li t3, 10
    beq t2, t3, end_read

    # Si es '\r' lo ignoramos
    li t3, 13
    beq t2, t3, read_loop

    # Convertir ASCII a numero
    addi t2, t2, -48     # '0' → 0

    bltz t2, read_loop   # No es digito
    li t3, 9
    bgt t2, t3, read_loop  # No esta entre 0–9

    # s0 = s0 * 10 + t2
    mul s0, s0, t1
    add s0, s0, t2
    j read_loop
end_read:
    mv a0, s0            # Retorno en a0
    lw s0, 8(sp)
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

# Telemetria

# Devuelve en a0 el contador de ciclos de reloj (rdcycle)
get_time:
    rdcycle a0
    ret

# Devuelve el PC actual usando AUIPC
get_current_pc:
    auipc a0, 0
    ret

# Convierte un entero en texto decimal y lo imprime con print_msg
# Usa un buffer temporal en stack
print_int:
    addi sp, sp, -32     # Reserva buffer en stack
    sw ra, 28(sp)

    # Caso especial: numero 0
    bnez a0, work_print
    li t0, 48            # '0'
    sb t0, 16(sp)
    li a1, 1             # Longitud = 1
    addi a0, sp, 16      # Direccion del buffer
    call print_msg
    j end_print

work_print:
    addi t1, sp, 26      # t1 = puntero al final del buffer
    li t2, 10            # Divisor decimal
    mv t3, a0            # Copia del numero

convert_loop:
    rem t4, t3, t2       # Resto = ultimo digito
    div t3, t3, t2       # Numero = numero/10
    addi t4, t4, 48      # Digito -> ASCII
    addi t1, t1, -1      # Avanzamos hacia atras en el buffer
    sb t4, 0(t1)         # Guardamos digito
    bnez t3, convert_loop

    # Imprimir cadena resultante
    mv a0, t1            # Inicio de la cadena
    addi t5, sp, 26
    sub a1, t5, t1       # Longitud = fin-inicio
    call print_msg

end_print:
    lw ra, 28(sp)
    addi sp, sp, 32
    ret
