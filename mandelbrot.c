
#include <iostream>
#include <fstream>
#include <string>
#include <mpi.h>
#include <stdlib.h>
using namespace std;

#define MASTER_P 0

struct complx {
  float real;
  float imag;
};

int cal_pixel(struct complx c);
int output(int **grid, int rows, int cols);

int main(int argc, char *argv[])
{
  MPI_Init(&argc,&argv);

  int display_height=atoi(*(argv+1));
  int display_width=atoi(*(argv+1));
  int real_min=-2;
  int real_max=2;
  int imag_min=-2;
  int imag_max=2;
  
  int nprocs,myid;
  const int KILL_TAG = display_width + 1;

  float scale_real=(float)(imag_max-imag_min)/display_width;
  float scale_imag=(float)(real_max-real_min)/display_height;

  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  if(nprocs > display_height+1){
    if(myid==0)
      printf("illegal slave count\nnprocs - 1 (%i) must be no larger than display_height (%i)\naborting...\n",nprocs - 1,display_height);
    return MPI_Finalize();
  }
  MPI_Status s;
  int *color;
  color = (int *) malloc(display_width * sizeof(int));

  if (myid == 0) { //master
    int **grid;
    grid = (int **) malloc(display_height * sizeof(int*));
    for(int i=0;i<display_height;i++){
      *(grid+i) = (int *) malloc(display_width * sizeof(int));
    }
    int busy_slave_count = 0;
    int next_row_num = 0; //will be sent as tag
    for (int i = 1; i < nprocs; i++) { //give each slave a starting row
      MPI_Send(NULL,0,MPI_INT,i,next_row_num,MPI_COMM_WORLD);
      busy_slave_count++;
      next_row_num++;
    }
    do {
      MPI_Recv(color,display_width,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
      busy_slave_count--;
      if(next_row_num < display_height) { //send row to slave
        MPI_Send(NULL,0,MPI_INT,s.MPI_SOURCE,next_row_num,MPI_COMM_WORLD);
        next_row_num++;
        busy_slave_count++;
      }else{ //kill slave
        MPI_Send(NULL,0,MPI_INT,s.MPI_SOURCE,KILL_TAG,MPI_COMM_WORLD);
      }
      memcpy(*(grid+s.MPI_TAG),color,display_width*sizeof(int));
    }while(busy_slave_count > 0);
    output(grid,display_height,display_width);
    free(grid);
  }
  else{ //slave
    MPI_Recv(NULL,0,MPI_INT,MASTER_P,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
    while(s.MPI_TAG != KILL_TAG) {
      complx c;
      c.imag = imag_min + ((float) s.MPI_TAG * scale_imag);
      for (int col_num = 0; col_num < display_width; col_num++) {
        c.real = real_min + ((float) col_num * scale_real);
        *(color+col_num) = cal_pixel(c);
      }
      MPI_Send(color,display_width,MPI_INT,MASTER_P,s.MPI_TAG,MPI_COMM_WORLD);
      MPI_Recv(NULL,0,MPI_INT,MASTER_P,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
    }
  }
free(color);
return MPI_Finalize();
}

int cal_pixel(struct complx c)
{
  int count, max_iter;
  complx z;
  float temp, lengthsq;
  max_iter = 256;
  z.real = 0;
  z.imag = 0;
  count = 0;
  do  {
    temp = z.real * z.real - z.imag * z.imag + c.real;
    z.imag = 2 * z.real * z.imag + c.imag;
    z.real = temp;
    lengthsq = z.real * z.real + z.imag * z.imag;
    count++;
  }while((lengthsq < 4.0) && (count < max_iter));
return count;
}
    
int output(int **grid, int rows, int cols)
{
  ofstream file_handle;
  file_handle.open("output.csv");
  for(int row=0;row<rows;row++){
    for(int col=0;col<cols;col++){
      file_handle << *(*(grid+row)+col);
      if(col==cols-1){
        file_handle << "\n";
      }
      else{
        file_handle << ",";
      }
    }
  }
  file_handle.close();
  return 0;
}

