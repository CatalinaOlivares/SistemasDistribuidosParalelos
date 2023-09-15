#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <emmintrin.h> // Para instrucciones SSE
#include <time.h>      // Para medir el tiempo con clock()

// Función para aplicar dilatación en una vecindad de 5 píxeles
int dilatacion(__m128i vecindad[4]) {
    __m128i maximo = vecindad[0];
    for (int i = 0; i < 4; i++) {
        maximo = _mm_max_epu8(maximo, vecindad[i]);
    }
    return _mm_extract_epi16(maximo, 0);
}
// Función para aplicar dilatación en una vecindad de 5 píxeles de forma secuencial
unsigned char dilatacion_secuencial(unsigned char vecindad[5]) {
    unsigned char maximo = vecindad[0];
    for (int i = 1; i < 5; i++) {
        if (vecindad[i] > maximo) {
            maximo = vecindad[i];
        }
    }
    return maximo;
}

void llenarVecindad(__m128i vecindad[5], const unsigned char *imagen, int i, int j, int ancho, int alto) {
    for (int x = -1; x <= 2; x++) {
        for (int y = -1; y <= 1; y++) {
            // Calcula las coordenadas
            int newX = i + x;
            int newY = j + y;

            // Verifica si las coordenadas están dentro de los límites de la imagen
            if (newX >= 0 && newX < ancho && newY >= 0 && newY < alto) {
                vecindad[(x + 1) * 4 + (y + 1)] = _mm_set1_epi8(imagen[newX * alto + newY]);
            } else {
                // Si está fuera de los límites, asigna un valor adecuado o lo que corresponda
                vecindad[(x + 1) * 4 + (y + 1)] = _mm_setzero_si128(); // Por ejemplo, asigna cero
            }
        }
    }
}
void guardarImagenPGM(const char *nombreArchivo, unsigned char *imagen, int ancho, int alto) {
    FILE *archivo = fopen(nombreArchivo, "wb");
    if (!archivo) {
        fprintf(stderr, "No se pudo abrir el archivo %s.\n", nombreArchivo);
        exit(1);
    }

    fprintf(archivo, "P5\n");
    fprintf(archivo, "%d %d\n", ancho, alto);
    fprintf(archivo, "255\n");

    fwrite(imagen, 1, ancho * alto, archivo);
    fclose(archivo);
}

int main(int argc, char *argv[]) {
    char *imagen_entrada = NULL;
    char *imagen_salida1 = NULL;
    char *imagen_salida2 = NULL;
    int ancho_imagen = 0;

    int opt;
    while ((opt = getopt(argc, argv, "i:s:p:N:")) != -1) {
        switch (opt) {
            case 'i':
                imagen_entrada = optarg;
                break;
            case 's':
                imagen_salida1 = optarg;
                break;
            case 'p':
                imagen_salida2 = optarg;
                break;
            case 'N':
                ancho_imagen = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Uso incorrecto. Uso: ./dilation -i imagen_entrada.pgm -s imagen_salida1.pgm -p imagen_salida2.pgm -N ancho_imagen\n");
                return 1;
        }
    }

    if (imagen_entrada == NULL || imagen_salida1 == NULL || imagen_salida2 == NULL || ancho_imagen == 0) {
        fprintf(stderr, "Faltan argumentos. Uso: ./dilation -i imagen_entrada.pgm -s imagen_salida1.pgm -p imagen_salida2.pgm -N ancho_imagen\n");
        return 1;
    }

    FILE *archivo;
    char formato[3];
    int ancho, alto, valorMaximo;

    // Abre el archivo PGM en modo binario
    archivo = fopen(imagen_entrada, "rb");
    if (!archivo) {
        fprintf(stderr, "No se pudo abrir el archivo.\n");
        return 1;
    }

    // Lee el formato (debe ser "P5")
    fscanf(archivo, "%2s", formato);
    if (strcmp(formato, "P5") != 0) {
        fprintf(stderr, "El archivo no está en formato P5 (PGM en escala de grises).\n");
        fclose(archivo);
        return 1;
    }

    // Lee el ancho, alto y valor máximo
    fscanf(archivo, "%d %d %d", &ancho, &alto, &valorMaximo);

    // Muestra el formato y la información
    printf("Formato: %s\n", formato);
    printf("Ancho: %d\n", ancho);
    printf("Alto: %d\n", alto);
    printf("Valor Maximo: %d\n", valorMaximo);

    // Lee los píxeles de la imagen directamente en el arreglo "imagen"
    unsigned char *imagen = (unsigned char *)malloc(ancho * alto);
    fread(imagen, 1, ancho * alto, archivo);

    // Cerramos el archivo
    fclose(archivo);

    // Medir el tiempo de inicio para la primera parte
    clock_t inicio_secuencial = clock();

    // Proceso de dilatación de forma secuencial
unsigned char *imagen_dilatada_secuencial = (unsigned char *)malloc(ancho * alto);

for (int i = 1; i < ancho - 1; i++) {
    for (int j = 1; j < alto - 1; j++) {
        unsigned char vecindad_secuencial[5];
        // Llena la vecindad con los píxeles circundantes de forma secuencial
        vecindad_secuencial[0] = imagen[(i - 1) * alto + (j - 1)];
        vecindad_secuencial[1] = imagen[(i - 1) * alto + j];
        vecindad_secuencial[2] = imagen[i * alto + (j - 1)];
        vecindad_secuencial[3] = imagen[i * alto + j];
        vecindad_secuencial[4] = imagen[(i + 1) * alto + (j + 1)]; // Agregar el píxel central

        // Aplica la dilatación de forma secuencial y guarda el resultado
        imagen_dilatada_secuencial[i * alto + j] = dilatacion_secuencial(vecindad_secuencial);
    }
}

    guardarImagenPGM(imagen_salida1, imagen_dilatada_secuencial, ancho, alto);
    //liberar memoria
    free(imagen_dilatada_secuencial);
    // Medir el tiempo de finalización para la primera parte
    clock_t fin_secuencial = clock();
    // Calcular el tiempo transcurrido en segundos para la primera parte
    double tiempo_transcurrido_secuencial = (double)(fin_secuencial - inicio_secuencial) / CLOCKS_PER_SEC;

    // Imprimir los tiempo transcurrido
    printf("Tiempo transcurrido secuencial: %f segundos\n", tiempo_transcurrido_secuencial);





 // Parte paralela
    unsigned char *imagen_dilatada_paralela = (unsigned char *)malloc(ancho * alto);

    

    __m128i vecindad[5]; // Declara vecindad una vez fuera del bucle anidado

    // Crear una matriz para almacenar las vecindades
    __m128i **vecindades = malloc((ancho - 2) * sizeof(__m128i *));
    for (int i = 0; i < ancho - 2; i++) {
        vecindades[i] = malloc((alto - 2) * sizeof(__m128i));
    }

    // Llenar las vecindades de manera eficiente antes del bucle principal
    for (int i = 1; i < ancho - 1; i++) {
        for (int j = 1; j < alto - 1; j++) {
            llenarVecindad(vecindad, imagen, i, j, ancho, alto);
            // Almacenar la vecindad en la matriz de vecindades
            vecindades[i - 1][j - 1] = vecindad[0]; // Puedes elegir cualquier elemento de la vecindad
        }
    }
// Medir el tiempo de inicio para la parte paralela
    clock_t inicio_paralela = clock();
    // Bucle principal utilizando la matriz de vecindades
    for (int i = 1; i < ancho - 1; i++) {
        for (int j = 1; j < alto - 1; j++) {
            // Obtener la vecindad desde la matriz de vecindades
            __m128i *vecindad_actual = &vecindades[i - 1][j - 1];

            // Calcula resultado con la vecindad llena
            __m128i resultado = _mm_max_epu8(_mm_max_epu8(vecindad_actual[0], vecindad_actual[1]),
                                              _mm_max_epu8(vecindad_actual[2], _mm_max_epu8(vecindad_actual[3], vecindad_actual[4])));

            // Guarda el resultado en la imagen dilatada en paralelo
            imagen_dilatada_paralela[i * alto + j] = _mm_extract_epi16(resultado, 0);
        }
    }
    clock_t fin_paralela = clock();

    // Calcular el tiempo transcurrido en segundos para la segunda parte
    double tiempo_transcurrido_paralela = (double)(fin_paralela - inicio_paralela) / CLOCKS_PER_SEC;
    guardarImagenPGM(imagen_salida2, imagen_dilatada_paralela, ancho, alto);
    // Imprimir los tiempo transcurrido
    printf("Tiempo transcurrido paralelamente: %f segundos\n", tiempo_transcurrido_paralela);
    free(imagen_dilatada_paralela);
    // Liberar la memoria de la matriz de vecindades
    for (int i = 0; i < ancho - 2; i++) {
        free(vecindades[i]);
    }
    free(vecindades);
    // Liberar la memoria
    free(imagen);

    return 0;
}