#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char map[31][31];
int rgb[100][3], farbaM[100][100], filter[100];
int row_amount, column_amount, k = 1;

#pragma pack(push, 1)

struct BitmapFileHeader {
  unsigned short bfType;
  unsigned long bfSize;
  unsigned short bfReserved1;
  unsigned short bfReserved2;
  unsigned long bfOffBits;
};

#pragma pack(pop)

struct BitmapInfoHeader {
  unsigned long biSize;
  long int biWidth;
  long int biHeight;
  unsigned short biPlanes;
  unsigned short biBitCount;
  unsigned long biCompression;
  unsigned long biSizeImage;
  long int biXPelsPerMeter;
  long int biYPelsPerMeter;
  unsigned long biClrUsed;
  unsigned long biClrImportant;
};

void write_head(FILE *file, int width, int height)
{
  if (width % 4 != 0 || height % 4 != 0)
  {
    printf("Chyba: Vyska a sirka nie su delitelne 4.\n");
    return;
  }
  struct BitmapInfoHeader bih;
  bih.biSize = sizeof(struct BitmapInfoHeader);
  bih.biWidth = width;
  bih.biHeight = height;
  bih.biSizeImage = bih.biWidth * bih.biHeight * 3;
  bih.biPlanes = 1;
  bih.biBitCount = 24;
  bih.biCompression = 0;
  bih.biXPelsPerMeter = 0;
  bih.biYPelsPerMeter = 0;
  bih.biClrUsed = 0;
  bih.biClrImportant = 0;

  struct BitmapFileHeader bfh;
  bfh.bfType = 0x4D42;
  bfh.bfSize = sizeof(struct BitmapFileHeader) + sizeof(struct BitmapInfoHeader) + bih.biSizeImage;
  bfh.bfReserved1 = 0;
  bfh.bfReserved2 = 0;
  bfh.bfOffBits = sizeof(struct BitmapFileHeader) + bih.biSize;

  fwrite(&bfh, sizeof(struct BitmapFileHeader), 1, file);
  fwrite(&bih, sizeof(struct BitmapInfoHeader), 1, file);
}

void write_pixel(FILE *file, unsigned char r, unsigned char g, unsigned char b)
{
  fwrite(&r, 1, 1, file);
  fwrite(&g, 1, 1, file);
  fwrite(&b, 1, 1, file);
}

void draw_map_base(char *file_name)
{
  FILE *file = fopen(file_name, "wb");
  int width = 40 * column_amount, height = 40 * row_amount;
  write_head(file, width, height);

  int row, column;
  for (column = 0; column < height; column++)
    for (row = 0; row < width; row++)
    {
      int i = (height-column)/40, j = row/40;
      switch (map[i][j])
      {
        case 'T': write_pixel(file, 0, 0, 255); break;
        case 'X': write_pixel(file, 255, 0, 0); break;
        case '#': write_pixel(file, 0, 0, 0); break;
        case '.': write_pixel(file, 255, 255, 255); break;
        default: write_pixel(file, 128, 128, 128); break;
      }
    }
  fclose(file);
}

void kresli_miestnosti(char *file_name)
{
  FILE *file = fopen(file_name, "wb");
  int width = 40 * column_amount, height = 40 * row_amount;
  write_head(file, width, height);

  int row, column;
  for (column = 0; column < height; column++)
    for (row = 0; row < width; row++)
    {
      int i = (height-column)/40, j = row/40;
      int c = farbaM[i][j];
      switch (map[i][j])
      {
        case 'T': write_pixel(file, 0, 0, 255); break;
        case 'X': write_pixel(file, 255, 0, 0); break;
        case '#': write_pixel(file, 0, 0, 0); break;
        case '.': write_pixel(file, rgb[c][0], rgb[c][1], rgb[c][2]); break;
        default: write_pixel(file, 128, 128, 128); break;
      }
    }
  fclose(file);
}

struct Prvok
{
  char ciel;
  struct Prvok *dalsi;
  int farba;
};

struct Zoznam
{
  struct Prvok *prvy;
  char meno;
};

struct Zoznam *zoznam_vytvor(char znak)
{
  struct Zoznam *z =(struct Zoznam *)calloc(1,sizeof(struct Zoznam));
  z->meno = znak;
  return z;
};

struct Prvok *prvok_vytvor(char ciel, struct Prvok *dalsi, int farba)
{
  struct Prvok *p =(struct Prvok *)calloc(1,sizeof(struct Prvok));
  p->ciel = ciel;
  p->farba = farba;
  p->dalsi = dalsi;
  return p;
};

void zoznam_vloz(struct Zoznam *z, char ciel, int farba)
{
  z->prvy = prvok_vytvor(ciel, z->prvy, farba);
}

void zoznam_vypis(struct Zoznam *z)
{
  struct Prvok *p = z->prvy;
  printf ("%c: ", z->meno);
  while(p != NULL)
  {
    printf(" %c", p->ciel);
    p = p->dalsi;
  }
}

struct Zoznam *z[26];

void vypis_susednosti()
{
  int i, j;
  for(i=0; i<26; i++)
    z[i] = zoznam_vytvor('A' + i);
  int q;
  for (q = 1; q < k; q++)
  {
    int bod[26];
    for(i=0; i<26; i++)
      bod[i] = 0;

    for (i = 0; i < row_amount; i++)
      for (j = 0; j < column_amount; j++)
        if (farbaM[i][j] == q)
        {
          if(map[i+1][j] >= 'A' && map[i+1][j] <= 'Z')
            bod[map[i+1][j] - 'A'] = 1;
          if(map[i-1][j] >= 'A' && map[i-1][j] <= 'Z')
            bod[map[i-1][j] - 'A'] = 1;
          if(map[i][j+1] >= 'A' && map[i][j+1] <= 'Z')
            bod[map[i][j+1] - 'A'] = 1;
          if(map[i][j-1] >= 'A' && map[i][j-1] <= 'Z')
            bod[map[i][j-1] - 'A'] = 1;
        }
    for (i=0; i<26; i++)
      for (j=0; j<26; j++)
        if (i != j && bod[i] && bod[j])
          zoznam_vloz(z[i], 'A' + j, q);
  }

  for(i=0; i<26; i++)
  {
    zoznam_vypis(z[i]);
    printf("\n");
  }
}

void prechod (int vb, int pocet)
{
  if (pocet == 0)
    return;
  struct Prvok *p = z[vb - 'A']->prvy;
  while (p)
  {
    filter[p->farba] = 1;
    prechod(p->ciel, pocet-1);
    p = p->dalsi;
  }
}

void kresli_mapu (char *file_name, int k)
{
  prechod ('T', k);
  FILE *file = fopen(file_name, "wb");
  int width = 40 * column_amount, height = 40 * row_amount;
  write_head(file, width, height);

  int row, column;
  for (column = 0; column < height; column++)
    for (row = 0; row < width; row++)
    {
      int i = (height-column)/40, j = row/40;
      switch (map[i][j])
      {
        case 'T': write_pixel(file, 0, 0, 255); break;
        case 'X': write_pixel(file, 255, 0, 0); break;
        case '#': write_pixel(file, 0, 0, 0); break;
        case '.':
          if(filter[farbaM[i][j]])
            write_pixel(file, 64, 255, 64);
          else
            write_pixel(file, 255, 255, 255); break;
        default: write_pixel(file, 128, 128, 128); break;
      }
    }
}

int hladaj(int r, int s, int c)
{
  if( r < 0 || r >= row_amount || s < 0 || s >= column_amount || map[r][s] != '.' || farbaM[r][s])
    return 0;
  farbaM[r][s] = c;
  hladaj(r+1, s, c);
  hladaj(r-1, s, c);
  hladaj(r, s+1, c);
  hladaj(r, s-1, c);
  return 1;
}


int nacitaj_mapu()
{
  int dlzka;
  char predosly = 0;
  column_amount = 0; row_amount = 0;
  while((map[row_amount][column_amount] = getchar()) > 0)
  {
    if ((predosly == map[row_amount][column_amount]) && (predosly == '\n'))
    {
        column_amount = dlzka;
        return 1;
    }
    predosly = map[row_amount][column_amount];
    if (map[row_amount][column_amount] == '\n')
    {
          row_amount++;
          dlzka = column_amount;
          column_amount = 0;
          continue;
    }
    column_amount++;
  }
  if (row_amount++>0 && column_amount>0) return 1;
  return 0;
}

void initialize()
{
  int i, j;
  for (i=0; i<100; i++)
  {
      rgb[i][0] = rand() % 256;
      rgb[i][1] = rand() % 256;
      rgb[i][2] = rand() % 256;
  }
  for (i = 0; i < row_amount; i++)
  {
    for (j=0; j < column_amount; j++)
      if(hladaj(i,j,k))
        k++;
  }
  for (i = 0; i < row_amount; i++)
  {
    for (j=0; j < column_amount; j++)
      printf("%c", map[i][j]);
    printf("\n");
  }
  printf("\n");
}

void vykresli_farby()
{
  int i,j;
  for(i=0; i<row_amount; i++)
  {
    for(j=0;j<column_amount;j++)
      printf("%3d", farbaM[i][j]);
    printf("\n");
  }
  printf("\n");
}

int main(void)
{
  while (nacitaj_mapu() == 1)
  {
  initialize();
  draw_map_base("plan_mapy.bmp");
  kresli_miestnosti("miestnosti.bmp");
  vypis_susednosti();
  kresli_mapu ("mapa_final.bmp", 2);
  }
  return 0;
}


