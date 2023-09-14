# Nombre de tu programa
PROGRAMA = dilation

# Opciones de compilación
CFLAGS = -Wall -Wextra -O2 -lm

# Archivos fuente
FUENTE = dilation.c

# Regla por defecto: compila y ejecuta el comando
all: $(PROGRAMA)
	./$(PROGRAMA) -i bike.pgm -s imagen_salida1.pgm -p imagen_salida2.pgm -N 512

# Compila el programa
$(PROGRAMA): $(FUENTE)
	gcc $(CFLAGS) -o $(PROGRAMA) $(FUENTE)

# Limpia los archivos generados
clean:
	rm -f $(PROGRAMA)

.PHONY: all clean
