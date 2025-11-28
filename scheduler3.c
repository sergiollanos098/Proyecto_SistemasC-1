/* scheduler.c - Escenario 3: Prioridad Dinámica y Detección de Pérdida
   
   Reglas implementadas:
   - Adquisicion (P1) siempre al inicio.
   - Orden dinamico entre P2 (control) y P3 (transmision):
        * Zona Luminosa (0..42): P1 -> P2 -> P3 (Secuencia ideal)
        * Zona Oscura  (43..99): P1 -> P3 -> P2 (Simula cambio de prioridad)
   - Detección de "Cambio Abrupto": Si el salto de IDs es > 1 (ej: P1 a P3),
     se reporta "PERDIDA INFO".
   - IDs: P1=0, P2=1, P3=2
*/

extern int get_sensor_temp();
extern void print_msg(char* str, int len);
extern void print_int(int num);
extern void check_cooling(int temp, int* state_addr);
extern void transmit_data(int temp);
extern int get_time();
extern int get_current_pc();

// --- FUNCIONES AUXILIARES ---

int my_strlen(char* str) {
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

void report_loss(int from, int to) {
    char* msg = " | [ALERTA] PERDIDA INFO: Salto P";
    print_msg(msg, my_strlen(msg));
    print_int(from + 1);
    print_msg("->P", 3);
    print_int(to + 1);
}

void main() {
    // Variables de Lógica
    int current_temp = 0;
    int cooling_state = 0;
    int sim_time = 0;
    int last_run = -1; // Rastrea el último proceso ejecutado entre ciclos

    // Variables de Métricas (Para el reporte final)
    int start_time = 0;
    int end_time = 0;
    int total_simulation_cycles = 0; 
    int total_syscalls = 0;          
    int anomaly_count = 0;           
    int loss_count = 0; // Nueva métrica: Contador de pérdidas de info

    char* msg_inicio = "\n--- INICIO SIMULACION ESCENARIO 3 (PRIORIZACION) ---\n";
    print_msg(msg_inicio, my_strlen(msg_inicio));

    while (1) {
        sim_time += 5;
        
        // Inicio cronómetro del ciclo
        start_time = get_time();

        // ---------------------------------------------------------
        // 1. ADQUISICIÓN (P1 - ID 0)
        // ---------------------------------------------------------
        // Siempre se ejecuta al inicio para obtener el dato
        current_temp = get_sensor_temp();
        total_syscalls++; // Syscall Read

        if (current_temp == 0) break; // Fin del archivo (EOF simulado con 0)

        // Prints informativos
        print_msg("Min: ", 5);
        print_int(sim_time);
        
        int tiempo_relativo = sim_time % 100;
        if (tiempo_relativo <= 42) {
            print_msg(" [Luz] | ", 9);
        } else {
            print_msg(" [Osc] | ", 9);
        }
        total_syscalls += 2; 

        // ---------------------------------------------------------
        // 2. SCHEDULER LOGIC (Determinación de Orden)
        // ---------------------------------------------------------
        int order[3];
        int ord_len = 0;

        // P1 siempre es el primero en la lógica de secuencia
        order[0] = 0; 
        ord_len = 1;

        if (tiempo_relativo <= 42) {
            // Zona luminosa: Orden natural (P1 -> P2 -> P3)
            order[1] = 1; 
            order[2] = 2; 
            ord_len = 3;
        } else {
            // Zona oscura: Prioridad a Transmisión (P1 -> P3 -> P2)
            // Esto causará un salto de ID 0 a ID 2 -> Reporte de pérdida
            order[1] = 2; 
            order[2] = 1; 
            ord_len = 3;
        }

        // ---------------------------------------------------------
        // 3. EJECUCIÓN ORDENADA Y DETECCIÓN DE SALTOS
        // ---------------------------------------------------------
        for (int i = 0; i < ord_len; i++) {
            int pid = order[i];

            // A) Detección de cambio abrupto (Lógica del Kernel)
            if (last_run != -1 && (last_run != pid)) {
                int diff = last_run - pid;
                if (diff < 0) diff = -diff; // Valor absoluto
                
                // Si el salto es mayor a 1 (ej: de 0 a 2), asumimos pérdida de contexto
                if (diff > 1) {
                    report_loss(last_run, pid);
                    loss_count++; // Aumentamos métrica
                }
            }

            // B) Ejecución del proceso
            if (pid == 0) {
                // P1 ya se ejecutó arriba (lectura), no hacemos nada aquí
                // pero necesitamos pasar por aquí para actualizar 'last_run'
            } else if (pid == 1) {
                // P2: Control
                check_cooling(current_temp, &cooling_state);
                total_syscalls++;
            } else if (pid == 2) {
                // P3: Transmisión
                transmit_data(current_temp);
                total_syscalls++;
            }

            // Actualizamos el último proceso corrido
            last_run = pid;
        }

        // ---------------------------------------------------------
        // 4. DETECCIÓN DE ANOMALÍAS (Hardware Interrupt)
        // ---------------------------------------------------------
        if (current_temp > 105) {
            int anomaly_pc = get_current_pc();
            print_msg(" | ANOMALIA PC: ", 16);
            print_int(anomaly_pc);
            anomaly_count++;
        }

        // ---------------------------------------------------------
        // 5. CÁLCULO DE TIEMPOS (Métricas)
        // ---------------------------------------------------------
        end_time = get_time();
        int iter_cycles = end_time - start_time;
        
        // Penalización por overhead de lógica de decisión (Simulado)
        iter_cycles += 20; 

        total_simulation_cycles += iter_cycles;

        print_msg(" | T_exe: ", 10);
        print_int(iter_cycles);
        print_msg("\n", 1);

        // Delay para visualización
        for (volatile int d = 0; d < 2000000; d++);
    }

    // ---------------------------------------------------------
    // REPORTE FINAL DE MÉTRICAS (Igual a Escenario 1 y 2)
    // ---------------------------------------------------------
    
    // Cálculo aproximado de memoria usada
    int mem_occupation = 0;
    mem_occupation += sizeof(current_temp) + sizeof(cooling_state) + sizeof(sim_time);
    mem_occupation += sizeof(last_run) + (sizeof(int) * 3); // Array order
    mem_occupation += sizeof(total_simulation_cycles) * 5;  // Variables de métricas

    char* sep = "------------------------------\n";
    print_msg("\n", 1);
    print_msg(sep, my_strlen(sep));
    print_msg("   RESULTADOS ESCENARIO 3    \n", 30);
    print_msg(sep, my_strlen(sep));

    print_msg("Ciclos Totales: ", 16);
    print_int(total_simulation_cycles);
    print_msg("\n", 1);

    print_msg("Syscalls: ", 10);
    print_int(total_syscalls);
    print_msg("\n", 1);

    print_msg("Cambios Abruptos (Loss): ", 25);
    print_int(loss_count);
    print_msg("\n", 1);

    print_msg("Anomalias Temp: ", 16);
    print_int(anomaly_count);
    print_msg("\n", 1);

    print_msg("Memoria Ocupada (Bytes): ", 25);
    print_int(mem_occupation);
    print_msg("\n", 1);
    
    print_msg(sep, my_strlen(sep));

    // Syscall Exit
    asm("li a7, 93; li a0, 0; ecall");
}