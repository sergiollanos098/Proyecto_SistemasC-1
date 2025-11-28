/* scheduler.c - Escenario 1 (FIFO) con Métricas Integradas */

extern int get_sensor_temp();
extern void print_msg(char* str, int len);
extern void print_int(int num);
extern void check_cooling(int temp, int* state_addr);
extern void transmit_data(int temp);
extern int get_time();
extern int get_current_pc();

// --- FUNCIÓN AUXILIAR (Importada de scheduler2.c) ---
int my_strlen(char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

void main() {
    // Variables funcionales
    int current_temp = 0;
    int cooling_state = 0; 
    int sim_time = 0; 
    
    // Variables de Métricas (Igual que scheduler2.c)
    int start_time = 0;
    int end_time = 0;
    int total_simulation_cycles = 0; // Acumulador total
    int total_syscalls = 0;          // Contador de llamadas al sistema
    int anomaly_count = 0;           // Contador de anomalías/interrupciones
    int anomaly_pc = 0;

    char* msg_inicio = "\n INICIO SIMULACION ORBITA LEO (ESCENARIO 1) \n";
    print_msg(msg_inicio, my_strlen(msg_inicio));

    while(1) {
        // El sensor toma muestras cada 5 minutos
        sim_time += 5;
        
        // Foto inicio ciclo
        start_time = get_time();
        
        // 1. LECTURA (Adquisición)
        current_temp = get_sensor_temp();
        total_syscalls++; // Syscall de lectura

        // Si leemos 0 asumimos que el archivo termino
        if (current_temp == 0) break; 

        // Prints informativos (Cuentan como syscalls de I/O)
        char* str_min = "Min: ";
        print_msg(str_min, my_strlen(str_min));
        print_int(sim_time);
        
        int tiempo_relativo = sim_time % 100;
        if (tiempo_relativo <= 42) {
            char* str_luz = " [Luz] | ";
            print_msg(str_luz, my_strlen(str_luz));
        } else {
            char* str_osc = " [Osc] | ";
            print_msg(str_osc, my_strlen(str_osc));
        }
        total_syscalls += 2; // Prints de tiempo y estado luz

        // 2. CONTROL DE ENFRIAMIENTO (Lógica FIFO original: Leer -> Enfriar)
        check_cooling(current_temp, &cooling_state);
        // Asumimos que check_cooling implica interacción con hardware/memoria externa
        // (Nota: en scheduler2 se contaba condicionalmente, aquí lo contamos general)
        total_syscalls++; 

        // 3. TRANSMISIÓN (Lógica FIFO original: Enfriar -> Transmitir)
        transmit_data(current_temp);
        total_syscalls++; 
        
        // Detección de Anomalía Crítica
        if (current_temp > 105) {
            anomaly_pc = get_current_pc();
            char* msg_anom = " | ANOMALIA PC: ";
            print_msg(msg_anom, my_strlen(msg_anom));
            print_int(anomaly_pc);
            
            anomaly_count++; // Aumentamos métrica de anomalía
        }

        // Fin del cronometro
        end_time = get_time();
        
        // Calculo de ciclos
        int iter_cycles = end_time - start_time;
        total_simulation_cycles += iter_cycles; // Acumulado global

        char* msg_exe = " | T_exe: ";
        print_msg(msg_exe, my_strlen(msg_exe));
        print_int(iter_cycles);
        print_msg("\n", 1); 
        total_syscalls += 2; // Print final de linea

        // Pequeño retardo
        for(volatile int d = 0; d < 2000000; d++);
    }
    
    // Cálculo memoria (Adaptado a las variables usadas aquí)
    int mem_occupation = 0;
    mem_occupation += sizeof(current_temp);
    mem_occupation += sizeof(cooling_state);
    mem_occupation += sizeof(sim_time);
    mem_occupation += sizeof(total_simulation_cycles);
    mem_occupation += sizeof(total_syscalls);
    mem_occupation += sizeof(anomaly_count);
    // Nota: Escenario 1 ocupa MENOS memoria porque no usa structs de Process

    // --- REPORTE FINAL (Formato idéntico a scheduler2.c) ---
    char* msg_res = "\n--- RESULTADOS ESCENARIO 1 ---\n";
    print_msg(msg_res, my_strlen(msg_res));
    
    char* msg_ciclos = "Ciclos Totales: ";
    print_msg(msg_ciclos, my_strlen(msg_ciclos));
    print_int(total_simulation_cycles);
    print_msg("\n", 1);

    char* msg_sys = "Syscalls: ";
    print_msg(msg_sys, my_strlen(msg_sys));
    print_int(total_syscalls);
    print_msg("\n", 1);

    char* msg_int = "Interrupts (Anomalias): ";
    print_msg(msg_int, my_strlen(msg_int));
    print_int(anomaly_count);
    print_msg("\n", 1);

    char* msg_mem = "Mem. Occupation (Bytes): ";
    print_msg(msg_mem, my_strlen(msg_mem));
    print_int(mem_occupation);
    print_msg("\n", 1);

    char* msg_line = "------------------------------\n";
    print_msg(msg_line, my_strlen(msg_line));

    // Syscall Exit para cerrar QEMU limpiamente 
    asm("li a7, 93; li a0, 0; ecall");
}