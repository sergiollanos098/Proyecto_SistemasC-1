/* scheduler.c */
// Funciones externas, en los otros .c
extern int get_sensor_temp();
extern void print_msg(char* str, int len);
extern void print_int(int num);
extern void check_cooling(int temp, int* state_addr);
extern void transmit_data(int temp);
extern int get_time();
extern int get_current_pc();

// Chicos para los otros scheduler comenten por si acaso, da flojera pero haganlo xd

void main() {
    int current_temp = 0;
    int cooling_state = 0; // Estado LED guardado en el Stack 
    
    // Metricas de rendimiento 
    int start_time = 0;
    int end_time = 0;
    int total_cycles = 0;
    int anomaly_pc = 0;
    
    // Reloj del satelite
    int sim_time = 0; 

    print_msg("\n INICIO SIMULACION ORBITA LEO \n", 30);

    while(1) {
        // El sensor toma muestras cada 5 minutos (empieza en el 5)
        sim_time += 5;
        
        // Foto de los ciclos de CPU actuales 
        start_time = get_time();
        
        // Proceso 1: Lectura
        // Leemos temperatura del .txt 
        current_temp = get_sensor_temp();
        // Si leemos 0 asumimos que el archivo termino
        if (current_temp == 0) break; 

        // Minuto actual
        print_msg("Min: ", 5);
        print_int(sim_time);
        
        int tiempo_relativo = sim_time % 100;
        // Minutos 0 a 42 -> zona de luz 
        if (tiempo_relativo <= 42) {
            print_msg(" [Luz] | ", 9);
        } else {
            // Minutos 43 a 100 -> zona oscura 
            print_msg(" [Osc] | ", 9);
        }

        // Proceso 2: Control de enfriamiento 
        // Pasamos la direccion de memoria &cooling_state para que assembly pueda modificarla 
        check_cooling(current_temp, &cooling_state);

        // Proceso 3: Transmitir dato, solo imprime el numero 
        transmit_data(current_temp);
        
        // Se rompió el limite fisico
        if (current_temp > 105) {
            // Capturamos direccion exacta de memoria PC donde ocurrio 
            anomaly_pc = get_current_pc();
            print_msg(" | ANOMALIA PC: ", 17);
            print_int(anomaly_pc);
        }

        // Fin del cronometro
        end_time = get_time();
        // Total de ciclos (end - start)
        total_cycles = end_time - start_time;

        print_msg(" | T_exe: ", 10);
        print_int(total_cycles);
        print_msg("\n", 1); 

        // Pequeño retardo para ver los datos pasar
        for(volatile int d = 0; d < 2000000; d++);
    }
    
    print_msg(" FIN DE ORBITA \n", 16);
    // Syscall Exit para cerrar QEMU limpiamente 
    asm("li a7, 93; li a0, 0; ecall");
}
