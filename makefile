# Nombre de tu programa
PROGRAMA = dilation

# Opciones de compilación
CFLAGS = -Wall -Wextra -O2 -lm

# Archivos fuente
FUENTE = dilation.c

# Regla por defecto: compila y ejecuta el comando para bike.pgm
all: $(PROGRAMA)  test_lines test_bike

# Compila el programa
$(PROGRAMA): $(FUENTE)
	gcc $(CFLAGS) -o $(PROGRAMA) $(FUENTE)

# Ejecuta el comando con bike.pgm
test_bike: $(PROGRAMA)
	./$(PROGRAMA) -i bike.pgm -s imagen_salida1_bike.pgm -p imagen_salida2_bike.pgm -N 512

# Ejecuta el comando con lines.pgm
test_lines: $(PROGRAMA)
	./$(PROGRAMA) -i lines.pgm -s imagen_salida1_lines.pgm -p imagen_salida2_lines.pgm -N 4000

# Limpia los archivos generados
clean:
	rm -f $(PROGRAMA) imagen_salida1_bike.pgm imagen_salida2_bike.pgm imagen_salida1_lines.pgm imagen_salida2_lines.pgm
