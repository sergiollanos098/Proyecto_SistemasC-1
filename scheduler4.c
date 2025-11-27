/* scheduler.c - Escenario 4: Syscalls y Guardado de Contexto (PCB) */
extern int get_sensor_temp();               // Proceso 1
extern void print_msg(char* str, int len);  // Driver
extern void print_int(int num);             // Driver
extern void check_cooling(int temp, int* state_addr); // Proceso 2
extern void transmit_data(int temp);        // Proceso 3
extern int get_time();                      // rdcycle
extern int get_current_pc();                

int my_strlen(char* str) {
    int len = 0;
    while (str[len] != '\0') len++; 
    return len;
}

// Estructura PCB (Process Control Block)
typedef struct {
    int id;         // Identificador 
    int pc;         
    int sp;         // Stack Pointer, donde estan sus variables
    int status;     // Estado: 0=Listo, 1=Corriendo, 2=Pausado
    int priority;   
} PCB;

PCB pcb_table[3]; 

// Funciones de Context Switch

// Simula congelar un proceso en el tiempo.
void save_context(int process_idx) {
    // 1. Preguntamos al CPU: en que linea de codigo estoy?
    int current_pc = get_current_pc(); 
    
    // 2. Guardamos ese numero en la tabla del proceso correspondiente
    pcb_table[process_idx].pc = current_pc;
    
    // 3. Marcamos el proceso como "Interrumpido" (Status 2)
    pcb_table[process_idx].status = 2; 
    
    // Print
    char* msg = " [OS] Context Switch -> Guardando PC: ";
    print_msg(msg, my_strlen(msg));
    print_int(current_pc);
    print_msg("\n", 1);
}

// Simula descongelar un proceso para que siga donde se quedó.
void restore_context(int process_idx) {
    // 1. Print del PC que vamos a recuperar 
    char* msg = " [OS] Recuperando el PC: ";
    print_msg(msg, my_strlen(msg));
    print_int(pcb_table[process_idx].pc); // Leemos de la tabla
    
    // 2. Avisamos que volvemos a ese punto
    print_msg(" -> Reanudando proceso...\n", 23);
    
    // 3. Marcamos el proceso como "Corriendo" (Status 1)
    pcb_table[process_idx].status = 1; 
}

// PROGRAMA PRINCIPAL (SCHEDULER)
void main() {
    // Variables estado del satelite
    int current_temp = 0;
    int cooling_state = 0; 
    int prev_cooling_state = 0;
    int sim_time = 0; 
    
    // Variables para metricas 
    int total_cycles = 0;
    int total_syscalls = 0;
    int interrupts = 0;
    int cycle_start, cycle_end;

    // Inicializamos la Tabla PCB 
    pcb_table[0] = (PCB){1, 0, 0, 0, 1}; // P1
    pcb_table[1] = (PCB){2, 0, 0, 0, 2}; // P2
    pcb_table[2] = (PCB){3, 0, 0, 0, 1}; // P3

    char* header = "\n ESCENARIO 4: SYSCALLS & CONTEXT SWITCH \n";
    print_msg(header, my_strlen(header));

    // Simula la vida del satélite
    while(1) {
        sim_time += 5; // Avanzamos 5 minutos
        cycle_start = get_time(); // Iniciamos cronometro de esta iteracion

        // P1: Leer sensor de temperatura
        char* msg_p1 = "Min: ";
        print_msg(msg_p1, my_strlen(msg_p1));
        print_int(sim_time);
        print_msg(" | Ejecutando P1... ", 19);

        // Llamamos a Assembly para leer el sensor
        current_temp = get_sensor_temp();
        total_syscalls++; // Contamos 1 lectura

        if (current_temp == 0) break; // Si el archivo termina, salimos

        // Gestion de Flags e interrupciones
        // Simulamos la bandera de comunicación cada 10 minutos
        int communication_flag = (sim_time % 10 == 0); 

        // Si hay Flag activado o si hay peligro de temperatura
        if (communication_flag || (current_temp > 90 && cooling_state == 0)) {
            
            // A. Generar syscall (interrupcion)
            char* msg_int = "\n [!] INTERRUPCION SYSCALL GENERADA \n";
            print_msg(msg_int, my_strlen(msg_int));
            total_syscalls++; 
            interrupts++; // +1 Interrupcion detectada

            // B. Guardar contexto (Pausa P1) 
            save_context(0); // Guardamos el estado de P1 (ID 0)

            // C. Ejecutar proceso prioritario (P2 - Control) 
            // Aquí el OS le da el control a P2 para manejar el evento
            prev_cooling_state = cooling_state;
            check_cooling(current_temp, &cooling_state);
            
            // Si P2 tuvo que actuar (cambiar LED), cuenta como syscall
            if (prev_cooling_state != cooling_state) {
                total_syscalls++; 
            }

            // D. Restaurar contexto (reanudar P1) 
            restore_context(0); // Leemos la tabla y volvemos a P1
        }

        // P3: Transmitir datos al centro de control
        // Si no hubo interrupción (o despues de ella), transmitimos
        transmit_data(current_temp);
        total_syscalls++;

        // Anomalias criticas
        if (current_temp > 105) {
            int pc_anom = get_current_pc();
            char* msg_err = " | Falla Critica PC: ";
            print_msg(msg_err, my_strlen(msg_err));
            print_int(pc_anom);
            interrupts++;
        }

        // Metricas de tiempo
        cycle_end = get_time();
        int iter_cycles = cycle_end - cycle_start;
        total_cycles += iter_cycles; // Acumulamos al total global

        char* msg_exe = " | T_exe: ";
        print_msg(msg_exe, my_strlen(msg_exe));
        print_int(iter_cycles);
        print_msg("\n", 1);
        
        // Retardo para ver la simulación pasar lento
        for(volatile int d = 0; d < 2000000; d++);
    }

    // Reporte
    // Estimamos cuánta memoria RAM gastamos (Tabla PCB + Variables)
    int mem_occupation = sizeof(pcb_table) + sizeof(current_temp) * 10; 

    char* msg_res = "\n--- RESULTADOS ESCENARIO 4 ---\n";
    print_msg(msg_res, my_strlen(msg_res));

    // Imprimimos T_exe Total
    char* l1 = "T_exe: ";
    print_msg(l1, my_strlen(l1)); print_int(total_cycles); print_msg("\n", 1);

    // Imprimimos Syscalls
    char* l2 = "Syscalls: ";
    print_msg(l2, my_strlen(l2)); print_int(total_syscalls); print_msg("\n", 1);

    // Imprimimos Interrupciones (Context Switch)
    char* l3 = "Interrupts (Context Switch): ";
    print_msg(l3, my_strlen(l3)); print_int(interrupts); print_msg("\n", 1);

    // Imprimimos Memoria Ocupada
    char* l4 = "Mem. Occupation: ";
    print_msg(l4, my_strlen(l4)); print_int(mem_occupation); print_msg("\n", 1);
    
    asm("li a7, 93; li a0, 0; ecall");
}