#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#define COLS 1000
#define ROWS 1000

#define WHITE    "15 15 15 "
#define RED      "15 00 00 "
#define ORANGE   "15 05 00 "
#define YELLOW   "15 10 00 "
#define LTGREEN  "00 13 00 "
#define GREEN    "05 10 00 "
#define LTBLUE   "00 05 10 "
#define BLUE     "00 00 10 "
#define DARKTEAL "00 05 05 "
#define BROWN    "03 03 00 "
#define BLACK    "00 00 00 "

MPI_Init(&argc, &argv);

int rank, size;

MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
MPI_Status status
int room_width = 1000;
int room_height = 1000;

int rank_height = room_height*(rank+1)/size - room_height*rank/size;//each rows section to calc.

float local_array = (float **)malloc(row_width*(rank_height+2)*sizeof(float));
float full_array = (float **)malloc(row_width*(rank_height+2)*sizeof(float));



// Send bottom row down
if (rank < size-1)
{
    MPI_Send(local_array + room_width*rank_height, room_width, MPI_FLOAT, rank+1, 0, MPI_COMM_WORLD);
    MPI_Recv(local_array + room_width*(rank_height+1), room_width, MPI_FLOAT, rank+1, 1, MPI_COMM_WORLD, &status);
}
if (rank > 0)
{
    // Receive ghost row from above
    MPI_Recv(local_array, room_width, MPI_FLOAT, rank-1, 0, MPI_COMM_WORLD, &status);
    // Send top row up
    MPI_Send(local_array + room_width, room_width, MPI_FLOAT, rank-1, 1, MPI_COMM_WORLD);
}

// Copy the current slice to the old slice
copyNewToOld(local_array, full_array);
// Generate a new current slice based on the old slice
calculateNew(local_array, full_array);

disps = (int *)malloc(size*sizeof(int));
sizes = (int *)malloc(size*sizeof(int));
// Calculate displacements and sizes
disps[0] = 0;
for (i = 1; i <= size; i++)
{
    int next = (room_height*i/size) * width;
    if (i < size)
	disps[i] = next;
    sizes[i-1] = next - disps[i-1];
}

MPI_Gatherv(local_array, rank_height*room_width, MPI_FLOAT, full_array, sizes, disps, MPI_FLOAT, 0, MPI_COMM_WORLD);


if(rank ==0)
	printGridtoFile(full_array);

// copy new grid to the old grid
void copyNewToOld(float grid_a[ROWS][COLS], float grid_b[ROWS][COLS]) {
  int x, y;
  for (x = 0; x < ROWS; ++x) {
    for (y = 0; y < COLS; ++y) {
      grid_b[x][y] = grid_a[x][y];
    }
  }
}

// update the new grid based on the old grid
void calculateNew(float grid_a[ROWS][COLS], float grid_b[ROWS][COLS]) {
  int x, y;
  for (x = 1; x < ROWS - 1; ++x) {
    for (y = 1; y < COLS - 1; ++y) {
      grid_a[x][y] = 0.25 * (grid_b[x-1][y] + grid_b[x+1][y] + grid_b[x][y-1] + grid_b[x][y+1]);
    }
  }
}

// print the grid to a bitmat file
void printGridtoFile(float grid[ROWS][COLS]) {
  int x, y;
  FILE * fp;
  fp = fopen("c.pnm", "w");

  /* Print the P3 format header */
  fprintf(fp, "P3\n%d %d\n15\n", COLS, ROWS);
  for (x = 0; x < ROWS; ++x) {
    for (y = 0; y < COLS; ++y) {
      if (grid[x][y] > 250) {
	fprintf(fp, "%s ", RED );
      } else if (grid[x][y] > 180) {
	fprintf(fp, "%s ", ORANGE );
      } else if (grid[x][y] > 120) {
	fprintf(fp, "%s ", YELLOW );
      } else if (grid[x][y] > 80) {
	fprintf(fp, "%s ", LTGREEN );
      } else if (grid[x][y] > 60) {
	fprintf(fp, "%s ", GREEN );
      } else if (grid[x][y] > 50) {
	fprintf(fp, "%s ", LTBLUE );
      } else if (grid[x][y] > 40) {
	fprintf(fp, "%s ", BLUE );
      } else if (grid[x][y] > 30) {
	fprintf(fp, "%s ", DARKTEAL );
      } else if (grid[x][y] > 20) {
	fprintf(fp, "%s ", BROWN );
      } else {
	fprintf(fp, "%s ", BLACK );
      }
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

int main(int argc, char **argv) {
  int h, w, cycles, heat;
  float grid_a[ROWS][COLS];
  float grid_b[ROWS][COLS];

  if (argc != 2) {
    printf("Usage: ./program <number of timestamps>\n");
    exit(0);
  }
  cycles = atoi(argv[1]);

  // initialize two-dimensional array
  for (h = 0; h < ROWS; ++h) {
    for (w = 0; w < COLS; ++w) {
      grid_a[w][h] = 20;
    }
  }

  // initialize a heat source
  for (heat = 299; heat < 700; ++heat) {
    grid_a[0][heat] = 300;
  }

  for (cycles; cycles > 0; --cycles) {
    copyNewToOld(grid_a, grid_b);
    calculateNew(grid_a, grid_b);
  }

  printGridtoFile(grid_a);
  system("convert c.pnm c.png");
  return 0;
}
