// texture_gen.cpp

#include <stdio.h>
#include <stdlib.h>

#define WIDTH 32

void savePPM(const char* filename, unsigned char* data, int width, int height);

int main()
{
	unsigned char* data = new unsigned char[WIDTH*3];

	for(uint i=0 ; i < WIDTH ; i++)
	{
		unsigned char value = (unsigned char)((WIDTH-1-i)*255 / (WIDTH-1));
		data[i*3+0] = value;
		data[i*3+1] = value;
		data[i*3+2] = value;
	}

	savePPM("fading.ppm", data, WIDTH, 1);

	delete [] data;

	return 0;
}

void savePPM(const char* filename, unsigned char* data, int width, int height)
{
	// Ouverture du fichier
	FILE* f = fopen(filename, "wb");
	if(!f)
	{
		fprintf(stderr, "Erreur : impossible de creer %s\n", filename);
		return;
	}

	// Ecriture du header
	fprintf(f, "P6\n%d %d\n255\n", width, height);

	// Ecriture des datas
	for(int y=0 ; y < height ; y++)
		fwrite(&data[y*width*3], 1, width*3, f);

	// Fermeture du fichier
	fclose(f);
}
