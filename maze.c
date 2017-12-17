#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//Global variables and arrays that aren't possible to avoid, without overcomplicating the code
char map[31][31];
int rgb[100][3], map_tile_color[100][100], filter[100];
int row_amount, column_amount, k = 1;

//Structures to help create complex header of BMP file
//Not so important to understand from the code, as it is all based by BMP specifications and definitions
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

//Creating BMP header based on wiki definitions using previously defined structures
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

//Function to write one pixel of the specific color to the file
void write_pixel(FILE *file, unsigned char red, unsigned char green, unsigned char blue)
{
  fwrite(&red, 1, 1, file);
  fwrite(&green, 1, 1, file);
  fwrite(&blue, 1, 1, file);
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
      int column_number = (height-column)/40, row_number = row/40;
	  
      switch (map[column_number][row_number])
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

//Function to draw actual rooms, based on input into BMP file
void draw_rooms(char *file_name)
{
  FILE *file = fopen(file_name, "wb");
  int width = 40 * column_amount, height = 40 * row_amount;
  write_head(file, width, height);

  int row, column;
  for (column = 0; column < height; column++)
    for (row = 0; row < width; row++)
    {
      int column_number = (height-column)/40, row_number = row/40;
      int color = map_tile_color[column_number][row_number];
	  
      switch (map[column_number][row_number])
      {
        case 'T': write_pixel(file, 0, 0, 255); break;
        case 'X': write_pixel(file, 255, 0, 0); break;
        case '#': write_pixel(file, 0, 0, 0); break;
        case '.': write_pixel(file, rgb[color][0], rgb[color][1], rgb[color][2]); break;
        default: write_pixel(file, 128, 128, 128); break;
      }
    }
  fclose(file);
}
//Set of LinkedList definitions, and functions to work with them
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
  printf ("%color: ", z->meno);
  while(p != NULL)
  {
    printf(" %color", p->ciel);
    p = p->dalsi;
  }
}

struct Zoznam *z[26]; //Global declaration of the list of structures

void find_adjacency()
{
  int count, count_r, column_number, row_number;
  
  for(count=0; count<26; count++)
    z[count] = zoznam_vytvor('A' + count);

  int actual_color;
  for (actual_color = 1; actual_color < k; actual_color++)
  {
    int bod[26];
    for(count=0; count<26; count++)
      bod[count] = 0;

    for (column_number = 0; column_number < row_amount; column_number++)
      for (row_number = 0; row_number < column_amount; row_number++)
        if (map_tile_color[column_number][row_number] == actual_color)
        {
          if(map[column_number+1][row_number] >= 'A' && map[column_number+1][row_number] <= 'Z')
            bod[map[column_number+1][row_number] - 'A'] = 1;
		
          if(map[column_number-1][row_number] >= 'A' && map[column_number-1][row_number] <= 'Z')
            bod[map[column_number-1][row_number] - 'A'] = 1;
		
          if(map[column_number][row_number+1] >= 'A' && map[column_number][row_number+1] <= 'Z')
            bod[map[column_number][row_number+1] - 'A'] = 1;
		
          if(map[column_number][row_number-1] >= 'A' && map[column_number][row_number-1] <= 'Z')
            bod[map[column_number][row_number-1] - 'A'] = 1;
        }
		
    for (count=0; count<26; count++)
      for (count_r=0; count_r<26; count_r++)
        if (count != count_r && bod[count] && bod[count_r])
          zoznam_vloz(z[count], 'A' + count_r, actual_color);
  }

  for(count=0; count<26; count++)
  {
    zoznam_vypis(z[count]);
    printf("\n");
  }
}

void possible_distance (int start_position, int distance)
{
  if (distance == 0)
    return;

  struct Prvok *p = z[start_position - 'A']->prvy;
  while (p)
  {
    filter[p->farba] = 1;
    possible_distance(p->ciel, distance-1);
    p = p->dalsi;
  }
}

void draw_final_map (char *file_name, int distance)
{
  possible_distance ('T', distance);
  FILE *file = fopen(file_name, "wb");
  int width = 40 * column_amount, height = 40 * row_amount;
  write_head(file, width, height);

  int row, column;
  for (column = 0; column < height; column++)
    for (row = 0; row < width; row++)
    {
      int column_number = (height-column)/40, row_number = row/40;
	  
      switch (map[column_number][row_number])
      {
        case 'T': write_pixel(file, 0, 0, 255); break;
        case 'X': write_pixel(file, 255, 0, 0); break;
        case '#': write_pixel(file, 0, 0, 0); break;
        case '.':
          if(filter[map_tile_color[column_number][row_number]])
            write_pixel(file, 64, 255, 64);
          else
            write_pixel(file, 255, 255, 255); break;
        default: write_pixel(file, 128, 128, 128); break;
      }
    }
}

int search(int column, int row, int color)
{
  if( row < 0 || row >= row_amount || column < 0 || column >= column_amount || map[row][column] != '.' || map_tile_color[row][column])
    return 0;

  map_tile_color[row][column] = color;
  search(row+1, column, color);
  search(row-1, column, color);
  search(row, column+1, color);
  search(row, column-1, color);
  return 1;
}


int load_plan()
{
  int length;
  char previous = 0;
  column_amount = 0; row_amount = 0;
  while((map[row_amount][column_amount] = getchar()) > 0)
  {
    if ((previous == map[row_amount][column_amount]) && (previous == '\n'))
    {
        column_amount = length;
        return 1;
    }
    previous = map[row_amount][column_amount];
    if (map[row_amount][column_amount] == '\n')
    {
          row_amount++;
          length = column_amount;
          column_amount = 0;
          continue;
    }
    column_amount++;
  }
  if (row_amount++>0 && column_amount>0) return 1;
  return 0;
}

void random_color(){
	return rand() % 256;
}

void initialize()
{
  int count, column_number, row_number;
  for (count=0; count<100; count++)
  {
      rgb[count][0] = random_color;
      rgb[count][1] = random_color;
      rgb[count][2] = random_color;
  }
  for (column_number = 0; column_number < row_amount; column_number++)
  {
    for (row_number=0; row_number < column_amount; row_number++)
      if(search(column_number,row_number,distance))
        distance++;
  }
  for (column_number = 0; column_number < row_amount; column_number++)
  {
    for (row_number=0; row_number < column_amount; row_number++)
      printf("%d", map[column_number][row_number]);
    printf("\n");
  }
  printf("\n");
}

void draw_colors()
{
  int column_number,row_number;
  for(column_number=0; column_number<row_amount; column_number++)
  {
    for(row_number=0;row_number<column_amount;row_number++)
      printf("%3d", map_tile_color[column_number][row_number]);
    printf("\n");
  }
  printf("\n");
}

int main(void)
{
  while (load_plan() == 1)
  {
  initialize();
  draw_map_base("plan_mapy.bmp");
  draw_rooms("miestnosti.bmp");
  find_adjacency();
  draw_final_map ("mapa_final.bmp", 2);
  }
  return 0;
}


