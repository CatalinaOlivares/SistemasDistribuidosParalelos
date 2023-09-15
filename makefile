# Nombre de tu programa
PROGRAMA = dilation

# Opciones de compilación
CFLAGS = -Wall -Wextra -O2 -lm

# Archivos fuente
FUENTE = dilation.c

# Regla por defecto: compila y ejecuta el comando para bike.pgm
all: $(PROGRAMA)  

# Compila el programa
$(PROGRAMA): $(FUENTE)
	gcc $(CFLAGS) -o $(PROGRAMA) $(FUENTE)


# Limpia los archivos generados
clean:
	rm -f $(PROGRAMA) imagen_salida1_bike.pgm imagen_salida2_bike.pgm imagen_salida1_lines.pgm imagen_salida2_lines.pgm
