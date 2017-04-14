
#import <stdio.h>
#import <stdlib.h>

//verifica a quantidade de bytes do simbolo
int quantidadeBytes (unsigned char byte) {
	if (byte <= 0x7f){
		return 1;
	}
	if (byte >= 0xf0){
		return 4;
	}
	if (byte >= 0xe0){
		return 3;
	}
	if (byte >= 0xc0){
		return 2;
	}
	else return 0;
}

//junta todos os "bytes" na posicao correta em um unico unicode de 32bits 
unsigned int univet_unicode(unsigned int* uni_vet, int n) {
	unsigned int unicode = 0, i;
	for (i = 0;i < n; i++) {   
		uni_vet[i] =  uni_vet[i] << 8*(n-1-i);
		unicode = unicode | uni_vet[i];
	}

	return unicode;
}

//decodifica simbolo em utf8 para UNICODE
unsigned int utf8_UNICODE(unsigned char *str, int tam) {
	unsigned int uni_vet[3] = {0,0,0}; //vetor de int que fará expansão do vetor de char
	unsigned int unicode;

	if (tam == 1) {
		uni_vet[0] = *str;
		unicode = univet_unicode(uni_vet, 1);
	}
	else if (tam == 2) {
		str[0] = str[0] & 0x1f; //retira os bits 110 de marcacao
		str[1] = (str[1] & 0x3f); //retira os bits 10 de marcacao

		uni_vet[0] = str[0] >> 2; // primeiro byte do unicode
		//une os bits unicode do primeiro byte utf8 com os bits unicode do segundo byte utf8
		uni_vet[1] = (str[0] << 6) | str[1]; //segundo byte do unicode

		unicode = univet_unicode(uni_vet, 2);
	}
	else if (tam == 3) {
		str[0] = str[0] & 0x0f; //retira os bits 1110 de marcacao
		str[1] = str[1] & 0x3f; //retira os bits 10 de marcacao
		str[2] = str[2] & 0x3f; //retira os bits 10 de marcacao

		//une os bits unicode do primeiro byte utf8 com os bits unicode do segundo byte utf8
		uni_vet[0] = (str[0] << 4) | (str[1] >> 2); //primeiro byte do unicode
		//une os bits unicode restantes do segundo byte utf8 com os bits unicode do terceiro byte utf8
		uni_vet[1] = (str[1] << 6) | str[2]; //segundo byte do unicode

		unicode = univet_unicode(uni_vet, 2);
	}
	else if (tam == 4) {
		//unicode = (unsigned char*)malloc(sizeof(unsigned char)*3);

		str[0] = str[0] & 0x07; //retira os bits 1111 de marcacao
		str[1] = str[1] & 0x3f; //retira os bits 10 de marcacao
		str[2] = str[2] & 0x3f; //retira os bits 10 de marcacao
		str[3] = str[3] & 0x3f; //retira os bits 10 de marcacao

		//bits unicode do 1o byte + bits unicode da esquerda 2o byte
		uni_vet[0] = (str[0] << 2) | (str[1] >> 4);
	
		//bits unicode restantes do 2o byte + bits unicode do 3o byte
		uni_vet[1] = (str[1] << 4) | (str[2] >> 2);
		
		//bits unicode restantes do 3o byte + bits unicode do 4o byte
		uni_vet[2] = (str[2] << 6) | str[3];

		unicode = univet_unicode(uni_vet, 3);
		
	}
	else {
		printf("nao codificado \n");
		return -1;
	}
	
	printf("\nunicode: ");
	printf(" 0x%08x ", unicode);

	return unicode;
}

//transforma o unicode em 1 ou 2 code units do tipo int
void unicode_utf16 (unsigned int *unicode, unsigned int *code_unit1, unsigned int *code_unit2) {

	if (*unicode >= 0x10000) {
		*unicode = *unicode - 0x10000;
		*code_unit1 = 0xd800 + (*unicode >> 10);
		*code_unit2 = (*unicode & 0x3ff) + 0xdc00;
	}
	else {
		*code_unit1 = *unicode;
	}
}

//escreve no arquivo na ordem Big Endian (apenas os 16 bits)
void writeUTF16_BE (void *p, FILE *arq_saida) {
	int i = 2;
	unsigned char *p1 = p;
	printf("dump BE code unit ");
	while (i--){
		//printf("%d\n", i);
		printf("%02x ", p1[i]);
		fputc(p1[i], arq_saida);
	}
}

int utf8_16(FILE *arq_entrada, FILE *arq_saida) {
	unsigned char vetByte[4] = {0,0,0,0};
	char char_utf8 = 'a';
	unsigned int unicode = 0, code_unit1 = 0, code_unit2 = 0;
	unsigned char BOM[2] = {0xfe,0xff}; //BIG ENDIAN FILE
	int i, tam = 0;

	if (arq_entrada == NULL) {
		printf("Erro na abertura do arquivo de entrada.\n");
		return -1;
	}

	if(arq_saida == NULL){
		printf("Erro na abertura do arquivo de saida.\n");
		return -1;
	}

	fwrite(BOM,2,1,arq_saida);

	char_utf8 = fgetc(arq_entrada);

	while(char_utf8 != EOF){
 		printf("\n\n --- %c ", char_utf8);
 		
 		tam = quantidadeBytes(char_utf8);

 		vetByte[0] = (unsigned char)char_utf8;
 		printf("\nutf8 0x%02x", vetByte[0]);

 		for (i = 1; i < tam; i++) {
 			char_utf8 = fgetc(arq_entrada);
 			vetByte[i] = char_utf8;
 			printf("%02x ", vetByte[i]);
 		}

 		unicode = utf8_UNICODE(vetByte, tam);
 		unicode_utf16(&unicode, &code_unit1, &code_unit2);

 		printf("\n code unit1: 0x%08x ", code_unit1);
 		
 		writeUTF16_BE(&code_unit1, arq_saida);
 		
 		if (code_unit1 >> 10 == 0x36){
 			printf("\n code unit2: 0x%08x ", code_unit2);
 			writeUTF16_BE(&code_unit2, arq_saida);
 		}

 		char_utf8 = fgetc(arq_entrada);
	}

	return 0;
}


int main () {
	FILE *f_entrada, *f_saida;

	f_entrada = fopen("utf8_demo.txt", "rb");
	f_saida = fopen("utf16_long_saida.txt", "wb");

	utf8_16(f_entrada, f_saida);

	return 0;
}
