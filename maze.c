#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char mapa[31][31];
int rgb[100][3], farbaM[100][100], filter[100];
int n, m, k = 1;

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

void write_head(FILE *f, int width, int height)
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

  fwrite(&bfh, sizeof(struct BitmapFileHeader), 1, f);
  fwrite(&bih, sizeof(struct BitmapInfoHeader), 1, f);
}

void write_pixel(FILE *f, unsigned char r, unsigned char g, unsigned char b)
{
  fwrite(&r, 1, 1, f);
  fwrite(&g, 1, 1, f);
  fwrite(&b, 1, 1, f);
}

void kresli_plan_mapy(char *nazov_suboru)
{
  FILE *f = fopen(nazov_suboru, "wb");
  int w = 40 * m, h = 40 * n;
  write_head(f, w, h);

  int x, y;
  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
    {
      int i = (h-y)/40, j = x/40;
      switch (mapa[i][j])
      {
        case 'T': write_pixel(f, 0, 0, 255); break;
        case 'X': write_pixel(f, 255, 0, 0); break;
        case '#': write_pixel(f, 0, 0, 0); break;
        case '.': write_pixel(f, 255, 255, 255); break;
        default: write_pixel(f, 128, 128, 128); break;
      }
    }
  fclose(f);
}

void kresli_miestnosti(char *nazov_suboru)
{
  FILE *f = fopen(nazov_suboru, "wb");
  int w = 40 * m, h = 40 * n;
  write_head(f, w, h);

  int x, y;
  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
    {
      int i = (h-y)/40, j = x/40;
      int c = farbaM[i][j];
      switch (mapa[i][j])
      {
        case 'T': write_pixel(f, 0, 0, 255); break;
        case 'X': write_pixel(f, 255, 0, 0); break;
        case '#': write_pixel(f, 0, 0, 0); break;
        case '.': write_pixel(f, rgb[c][0], rgb[c][1], rgb[c][2]); break;
        default: write_pixel(f, 128, 128, 128); break;
      }
    }
  fclose(f);
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

    for (i = 0; i < n; i++)
      for (j = 0; j < m; j++)
        if (farbaM[i][j] == q)
        {
          if(mapa[i+1][j] >= 'A' && mapa[i+1][j] <= 'Z')
            bod[mapa[i+1][j] - 'A'] = 1;
          if(mapa[i-1][j] >= 'A' && mapa[i-1][j] <= 'Z')
            bod[mapa[i-1][j] - 'A'] = 1;
          if(mapa[i][j+1] >= 'A' && mapa[i][j+1] <= 'Z')
            bod[mapa[i][j+1] - 'A'] = 1;
          if(mapa[i][j-1] >= 'A' && mapa[i][j-1] <= 'Z')
            bod[mapa[i][j-1] - 'A'] = 1;
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

void kresli_mapu (char *nazov_suboru, int k)
{
  prechod ('T', k);
  FILE *f = fopen(nazov_suboru, "wb");
  int w = 40 * m, h = 40 * n;
  write_head(f, w, h);

  int x, y;
  for (y = 0; y < h; y++)
    for (x = 0; x < w; x++)
    {
      int i = (h-y)/40, j = x/40;
      switch (mapa[i][j])
      {
        case 'T': write_pixel(f, 0, 0, 255); break;
        case 'X': write_pixel(f, 255, 0, 0); break;
        case '#': write_pixel(f, 0, 0, 0); break;
        case '.':
          if(filter[farbaM[i][j]])
            write_pixel(f, 64, 255, 64);
          else
            write_pixel(f, 255, 255, 255); break;
        default: write_pixel(f, 128, 128, 128); break;
      }
    }
}

int hladaj(int r, int s, int c)
{
  if( r < 0 || r >= n || s < 0 || s >= m || mapa[r][s] != '.' || farbaM[r][s])
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
  m = 0; n = 0;
  while((mapa[n][m] = getchar()) > 0)
  {
    if ((predosly == mapa[n][m]) && (predosly == '\n'))
    {
        m = dlzka;
        return 1;
    }
    predosly = mapa[n][m];
    if (mapa[n][m] == '\n')
    {
          n++;
          dlzka = m;
          m = 0;
          continue;
    }
    m++;
  }
  if (n++>0 && m>0) return 1;
  return 0;
}

void inicializacia()
{
  int i, j;
  for (i=0; i<100; i++)
  {
      rgb[i][0] = rand() % 256;
      rgb[i][1] = rand() % 256;
      rgb[i][2] = rand() % 256;
  }
  for (i = 0; i < n; i++)
  {
    for (j=0; j < m; j++)
      if(hladaj(i,j,k))
        k++;
  }
  for (i = 0; i < n; i++)
  {
    for (j=0; j < m; j++)
      printf("%c", mapa[i][j]);
    printf("\n");
  }
  printf("\n");
}

void vykresli_farby()
{
  int i,j;
  for(i=0; i<n; i++)
  {
    for(j=0;j<m;j++)
      printf("%3d", farbaM[i][j]);
    printf("\n");
  }
  printf("\n");
}

int main(void)
{
  while (nacitaj_mapu() == 1)
  {
  inicializacia();
  kresli_plan_mapy("plan_mapy.bmp");
  kresli_miestnosti("miestnosti.bmp");
  vypis_susednosti();
  kresli_mapu ("mapa_final.bmp", 2);
  }
  return 0;
}


