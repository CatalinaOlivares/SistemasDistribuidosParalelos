#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <emmintrin.h> // Para instrucciones SSE
#include <time.h>      // Para medir el tiempo con clock()

// Función para aplicar dilatación en una vecindad de 4 vecinos
int dilatacion(__m128i vecindad[3]) {
    __m128i maximo = vecindad[0];
    for (int i = 0; i < 3; i++) {
        maximo = _mm_max_epu8(maximo, vecindad[i]);
    }
    return _mm_extract_epi16(maximo, 0);
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
            __m128i vecindad[3];
            // Llena la vecindad con los píxeles circundantes
            for (int x = -1; x <= 1; x++) {
                vecindad[x + 1] = _mm_set1_epi8(imagen[(i + x) * alto + (j - 1)]);
            }
            // Aplica la dilatación y guarda el resultado en la imagen dilatada
            imagen_dilatada_secuencial[i * alto + j] = dilatacion(vecindad);
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
    
    //Proceso de dilatación de forma paralela con un solo máximo
    unsigned char *imagen_dilatada_paralela = (unsigned char *)malloc(ancho * alto);
    // Medir el tiempo de inicio para la segunda parte
    clock_t inicio_paralela = clock();
    for (int i = 1; i < ancho - 1; i++) {
        for (int j = 1; j < alto - 1; j++) {
            __m128i vecindad[4]; // Para los 4 vecinos
            // Llena la vecindad con los píxeles circundantes
            for (int x = -1; x <= 2; x++) {
                vecindad[x + 1] = _mm_set1_epi8(imagen[(i + x) * alto + (j - 1)]);
            }

            // Aplica la dilatación a los 4 vecinos en un solo paso
            __m128i resultado = _mm_max_epu8(_mm_max_epu8(vecindad[0], vecindad[1]), _mm_max_epu8(vecindad[2], vecindad[3]));

            // Guarda el resultado en la imagen dilatada
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
    // Liberar la memoria
    free(imagen);

    return 0;
}