/* proceso2.s */
# modifica cooling_state usando puntero
.global check_cooling

.text
# check_cooling(int temp, int* state_addr)
# a0 = temperatura actual
# a1 = direccion de la variable "cooling_state" en el stack de main
check_cooling:
    addi sp, sp, -16
    sw ra, 12(sp)       # Guardar ra
    sw s0, 8(sp)        # Guardar s0 (temperatura)
    sw s1, 4(sp)        # Guardar s1 (direccion del estado)
    
    mv s0, a0           # s0 = temp
    mv s1, a1           # s1 = &cooling_state
    
    # Cargar el estado actual desde la direccion dada por C 
    lw t1, 0(s1)        # t1 = estado (0 = OFF, 1 = ON)
    
    # Si el estado actual es ON, evaluar si debemos apagar
    bnez t1, check_off_logic

    # Estado OFF, encender si la temperatura supera 90
    li t2, 90
    bgt s0, t2, action_turn_on
    j end

check_off_logic:
    # Estado ON, apagar si temperatura cae por debajo de 60
    li t2, 60
    blt s0, t2, action_turn_off
    j end

action_turn_on:
    li t1, 1
    sw t1, 0(s1)        # Guardar nuevo estado (1) en la direccion proporcionada
    
    # Imprimir "[ON] "
    li t3, 32; sb t3, 4(sp)   
    li t3, 93; sb t3, 3(sp)   
    li t3, 78; sb t3, 2(sp)   
    li t3, 79; sb t3, 1(sp)   
    li t3, 91; sb t3, 0(sp)   
    
    li a7, 64; li a0, 1; mv a1, sp; li a2, 5; ecall
    j end

action_turn_off:
    li t1, 0
    sw t1, 0(s1)        # Guardar nuevo estado (0) en la direccion proporcionada
    
    # Imprimir "[OF] "
    li t3, 32; sb t3, 4(sp)   
    li t3, 93; sb t3, 3(sp)   
    li t3, 70; sb t3, 2(sp)   
    li t3, 79; sb t3, 1(sp)   
    li t3, 91; sb t3, 0(sp)   
    
    li a7, 64; li a0, 1; mv a1, sp; li a2, 5; ecall

end:
    lw s1, 4(sp)        # Restaurar s1
    lw s0, 8(sp)        # Restaurar s0
    lw ra, 12(sp)       # Restaurar ra
    addi sp, sp, 16
    ret
