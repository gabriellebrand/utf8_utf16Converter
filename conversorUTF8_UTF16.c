
#import <stdio.h>
#import <stdlib.h>
/*
0xxxxxxx
110x xxxx 10xx xxxx
1110 xxxx 10xx xxxx 10xx xxxx
1111 0xxx 10xx xxxx 10xx xxxx 10xx xxxx
*/
//bitmask 1 bytes: 0111 1111 -> >> 7 -> 0000 0000 (0x00)
//bitmask 2 bytes: 1100 0000 -> >> 5 -> 0000 0110 (0x06)
//bitmask 3 bytes: 1110 0000 -> >> 4 -> 0000 1110 (0x0e)
//bitmask 4 bytes: 1111 0000 -> >> 3 -> 0001 1110 (0x1e)
// 01110000


//funcao 1: verificar a quantidade de bytes do simbolo

int quantidadeBytes (unsigned char byte) {

	unsigned char aux = byte >> 7;
	// byte|0x7f;
	//TODO: mudar isso aqui para comparativo de >
	if (aux == 0x00){
		return 1;
	}
	aux = byte >> 3;
	if (aux == 0x1e){
		return 4;
	}
	aux = aux >> 1;
	if (aux == 0x0e){
		return 3;
	}
	aux = aux >> 1;
	if (aux == 0x06){
		return 2;
	}
	else return 0;
}

void dump (void *p, int n) {
  unsigned char *p1 = p;
  while (n--) {
    printf("%02x", *p1);
    p1++;
  }

}

int arraychar_int (unsigned char *array, int tam) {
	int saida = 0, shift = 3, i = tam-1;
	unsigned int *unicode;
	unicode = (unsigned int*)malloc(sizeof(unsigned int)*tam);
	printf("tam: %d", tam);
	for (i = tam; i>=0; i--){
		printf("\narray: ");
		unicode[i] = array[i];
		dump(&array[i], sizeof(array[i]));
		unicode[i] = unicode[i] >> shift;
		shift--;
		saida = saida | unicode[i];
		printf("\nsaida: ");
 		dump(&saida, sizeof(saida));
	}
	free(unicode);
	return saida;
}
unsigned char * utf8_UNICODE_2bytes (unsigned char *array) {
	unsigned char *unicode;
	unicode = (unsigned char*)malloc(sizeof(char)*2);
	
	array[0] = array[0] & 0x1f; //retira os bits de marcacao
	unicode[0] = array[0] >> 2; // primeiro byte do unicode
	unicode[1] = (array[0] << 6) | (array[1] & 0x3F); //segundo byte do unicode
		

	return unicode;
}

//decodifica simbolo em utf8 para UNICODE
unsigned char * utf8_UNICODE(unsigned char *str, int tam) {
	unsigned char *unicode; //vetor de bytes
	
	if (tam == 1) {
		unicode = str;
		
		printf("SIZE: %lu", sizeof(char));
		printf("\ndump unicode: ");
		dump(unicode, sizeof(unsigned char)); 
	}
	else if (tam == 2) {
		unicode = (unsigned char*)malloc(sizeof(unsigned char)*2);

		str[0] = str[0] & 0x1f; //retira os bits 110 de marcacao
		str[1] = (str[1] & 0x3f); //retira os bits 10 de marcacao
		unicode[0] = str[0] >> 2; // primeiro byte do unicode
		//une os bits unicode do primeiro byte utf8 com os bits unicode do segundo byte utf8
		unicode[1] = (str[0] << 6) | str[1]; //segundo byte do unicode

		printf("SIZE: %lu", sizeof(unicode)); //!! problema: sizeof(unicode) = 8bytes
		printf("\ndump unicode: ");
		dump(unicode, sizeof(unsigned char)*2); 
	}
	else if (tam == 3) {
		unicode = (unsigned char*)malloc(sizeof(unsigned char)*2);

		str[0] = str[0] & 0x0f; //retira os bits 1110 de marcacao
		str[1] = str[1] & 0x3f; //retira os bits 10 de marcacao
		str[2] = str[2] & 0x3f; //retira os bits 10 de marcacao
		//une os bits unicode do primeiro byte utf8 com os bits unicode do segundo byte utf8
		unicode[0] = (str[0] << 4) | (str[1] >> 2); //primeiro byte do unicode
		//une os bits unicode restantes do segundo byte utf8 com os bits unicode do terceiro byte utf8
		unicode[1] = (str[1] << 6) | str[2]; //segundo byte do unicode

		printf("\ndump unicode: ");
		dump(unicode, sizeof(unsigned char)*2); 
	}
	else if (tam == 4) {
		unicode = (unsigned char*)malloc(sizeof(unsigned char)*3);

		str[0] = str[0] & 0x07; //retira os bits 1111 de marcacao
		str[1] = str[1] & 0x3f; //retira os bits 10 de marcacao
		str[2] = str[2] & 0x3f; //retira os bits 10 de marcacao
		str[3] = str[3] & 0x3f; //retira os bits 10 de marcacao

		//bits unicode do 1o byte + bits unicode da esquerda 2o byte
		unicode[0] = (str[0] << 2) | (str[1] >> 4);
		//bits unicode restantes do 2o byte + bits unicode do 3o byte
		unicode[1] = (str[1] << 4) | (str[2] >> 2);
		//bits unicode restantes do 3o byte + bits unicode do 4o byte
		unicode[2] = (str[2] << 6) | str[3];

		dump(unicode, sizeof(unsigned char)*3); 

	}
	else {
		printf("nao codificado \n");
		return NULL;
	}

	return unicode;
}

int utf8_16(FILE *arq_entrada, FILE *arq_saida) {
	char *vetByte, char_utf8 = 'a';
	unsigned int saida;
	unsigned char * unicode;
	int i, tam = 0;

	while(char_utf8 != EOF){

 		char_utf8 = fgetc(arq_entrada);
 		printf("\n\n --- %c ", char_utf8);
 		
 		tam = quantidadeBytes (char_utf8);
 		vetByte = (char*)malloc(sizeof(char)*tam);
 		vetByte[0] = char_utf8;
 		printf("\nTAM: %d dump utf8: ", tam);
 		dump(&char_utf8, sizeof(char));

 		for (i = 1; i < tam; i++) {
 			char_utf8 = fgetc(arq_entrada);
 			vetByte[i] = char_utf8;
 			dump(&char_utf8, sizeof(char));
 		}

 		printf("\n str %s \n", vetByte);

 		unicode = utf8_UNICODE((unsigned char*)vetByte, tam);
 		saida = arraychar_int(unicode, tam);
 		printf("\nOUTPUT: ");
 		dump(&saida, sizeof(saida));
 		//printf(" unicode: %s \n", unicode);

 		free(vetByte);
 		//free(unicode); //!!problema: nao to conseguindo dar free

	}

	return 1;
}

int main () {
	FILE *f;

	f = fopen("utf8long.txt", "rb");
	utf8_16(f, f);

	return 0;
}
