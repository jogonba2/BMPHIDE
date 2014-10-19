#include <stdio.h>
#include <string.h>
#include <math.h>
#include <omp.h>

char* ORIGINAL_IMAGE = "Lenna.bmp";
char* DATA           = "I'm hidden :)";
char* METHOD         = "";
char  HIDE           = 0;
char  EXTRACT        = 0;
char  NTHREADS       = 2;

char enough_image_size(int num_pixels,char* text_to_hide){
	int number_bits_text = sizeof(text_to_hide)*8;
	if(num_pixels>=number_bits_text) return 1;
	return 0x00;
}

char hide_text_estego_object(FILE *fd,char *pixels_image,int offset_metadata_pixels){
	int i,j;
	int length_text = strlen(DATA);
	int *plength_text = &length_text;
	fseek(fd,6,SEEK_SET);
	fwrite(plength_text,4,1,fd);
	char mask = 0b10000000;
	omp_set_num_threads(NTHREADS);
	#pragma omp parallel for private(j) firstprivate(mask) if(length_text > 200)
	for(i=0;i<length_text;i++){
		for(j=0;j<8;j++){
			if(((DATA[i] & mask)>>(7-j)&0b00000001)==0) pixels_image[i+j] = pixels_image[i+j] & 0b11111110;
			else                        				pixels_image[i+j] = pixels_image[i+j] | 0b00000001;
			mask >>= 1;
			#pragma omp critical
			{
				fseek(fd,j+offset_metadata_pixels+i*8,SEEK_SET);
				fwrite(&pixels_image[i+j],1,1,fd);
			}
		}
	}
	fclose(fd);
    return 1;
}

char extract_text_estego_object(FILE* fd,int length_unhided,char* unhided,int offset_metadata_pixels){
	char byte_acum = 0b00000000;
	int i,z=0;
	char act_pixel;
	//omp_set_num_threads(NTHREADS);
	//#pragma omp parallel for firstprivate(act_pixel,z,byte_acum)
	for(i=0;i<((length_unhided)*8)+1;i++){
		//#pragma omp critical
		//{
		fseek(fd,offset_metadata_pixels+i,SEEK_SET);
		fread(&act_pixel,1,1,fd);
		//}	
		byte_acum  |= act_pixel & 0b00000001;
		//#pragma omp ordered if(i%8==0)
		//{
		if(i%8==0 && i!=0){ 
			unhided[z++] = byte_acum; 
			byte_acum = 0; 
		}
		//}
		if((i+1)%8!=0){ byte_acum <<= 1; }
	}
	return 1;
}

void show_summary(int num_pixels,int heigth,int width,int length_data){
	fprintf(stdout,"\n------------------------ Summary ----------------------------\n\n[!] Num pixels: %d \n[!] Heigth: %d\n[!] Width: %d\n[!] Length of hidded data: %d\n\n-------------------------------------------------------------",num_pixels,heigth,width,length_data);
}

void usage(){
	fprintf(stdout,"usage: bmphide in_image method [data_to_hide] num_threads\n\t in_image: image required to hide/extract\n\t method: --hide,--extract\n\t data_to_hide: optional if --hide is setted\n\t num_threads: specifies num threads\n");
}

void header(){
	fprintf(stdout,"\n\n*************************  BMPHIDE  *************************\n*************************************************************\n\n********************* Initialising  *************************\n");
}

int main(int argc,char* argv[])
{
	if(!(argc==4 || argc==5)){ usage(); return -1; }
	header();
	/** Set with args **/
	ORIGINAL_IMAGE = argv[1];
	METHOD         = argv[2];
	if(!strcmp(METHOD,"--hide") || !strcmp(METHOD,"--extract")){
		if(!strcmp(METHOD,"--hide")){
			HIDE = 1;
			if(argc!=5){ fprintf(stderr,"[-] Data or num_threads are not specified \n"); return 0; }
			DATA = argv[3];
		}
		else                         EXTRACT = 1;	
	}
	else{ fprintf(stderr,"[-] Method is not valid\n"); return 0; }
	if(argc==5) NTHREADS = atoi(argv[4]);
	else        NTHREADS = atoi(argv[3]);
	/** GET METADATA BMP **/
	int offset_metadata_pixels,num_pixels;
	FILE *fd;
	fd = fopen(ORIGINAL_IMAGE,"rb+");
    if (fd == NULL) { fprintf(stderr,"[-] Error opening file to read %s\n", ORIGINAL_IMAGE); return -1; }
	/** Check BM header offset 0:2 **/
	char type[3];type[2]='\0';
	fscanf(fd,"%c%c",&type[0],&type[1]);
	if(strcmp(type, "BM") != 0) { fprintf(stderr,"[-] Expected bmp \n"); return -1; }
	/** Extract offset to first pixel from offset 0 of metadata structure**/
	fseek(fd,10,SEEK_SET);
	fread(&offset_metadata_pixels,sizeof(int),1,fd);
	/** Extract horizontal pixels **/
	int width = 0;
	fseek(fd,18,SEEK_SET);
	fread(&width,sizeof(int),1,fd);
	/** Extract vertical pixels **/
	int heigth = 0;
	fread(&heigth,sizeof(int),1,fd);
	/** Extract total number of pixels **/
	num_pixels = width * heigth;
	/** Extract length of hidden data **/
	int length_data; 
	fseek(fd,6,SEEK_SET);
	fread(&length_data,4,1,fd);
	/** Show Summary **/
	show_summary(num_pixels,heigth,width,length_data);
	
	/** Read pixels from image **/
	char image_pixels[num_pixels];
	int i;
	omp_set_num_threads(NTHREADS);
	#pragma omp parallel for if (length_data>200)
	for(i=offset_metadata_pixels;i<offset_metadata_pixels+(length_data*8);i++){
		fseek(fd,i,SEEK_SET);	
		fread(&image_pixels[i-offset_metadata_pixels],1,1,fd);
	}
	char hidded[length_data];
	
	
	/** Manage calls **/
	fprintf(stdout,"\n\n------------------------ Results ----------------------------\n");
    if((HIDE && EXTRACT) || (!HIDE && !EXTRACT)){ fprintf(stderr,"\n[-]Not valid options, check flags\n"); return -1; }
    if(HIDE && !EXTRACT){ 
		if(enough_image_size(num_pixels,DATA)){
			if(hide_text_estego_object(fd,image_pixels,offset_metadata_pixels)) fprintf(stdout,"\n[+] Data: %s hidded correctly\n",DATA);
			else fprintf(stderr,"\n[-] Is not possible to hide %s \n",DATA);
		}
		else fprintf(stderr,"\n[-] Image is not large enough to store data\n");
	}
	else{ 
		if(extract_text_estego_object(fd,length_data,hidded,offset_metadata_pixels)){
			 hidded[length_data] = '\0'; 
			 fprintf(stdout,"\n[+] Data extracted correctly: %s \n",hidded);
		}
		else fprintf(stderr,"n[-] Image is not capable to store data\n");
	}
    fclose(fd);
    return 0;
}
