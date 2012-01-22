
#include <iostream>
#include <fstream>
#include <string>
#include "mpi.h"
using namespace std;

#define DATA_TAG 0
#define KILL_TAG 1
#define MASTER_P 0

struct complx {
  float real;
  float imag;
};

struct result {
  int slave_id;
  int row_num;
  int color[]; //flexible array ... will mpi handle this?
};

int cal_pixel(struct complx c);
int output(int row, int color[]);

int main(int argc, char *argv[])
{
  MPI_Init(&argc,&argv);

  int display_height=500;
  int display_width=500;
  int real_min=-2;
  int real_max=2;
  int imag_min=-2;
  int imag_max=2;

  //setting up slave result MPI datatype
  int r_member_count=3;
  int r_member_lengths[3] = {1,1,display_width};
  MPI_Aint r_member_offsets[3] = {0,sizeof(int),sizeof(int) * (1 + display_width)};
  MPI_Datatype r_member_types[3] = {MPI_INT,MPI_INT,MPI_INT};
  int mpi_result_datatype;
  MPI_Type_struct(r_member_count,r_member_lengths,r_member_offsets,r_member_types, &mpi_result_datatype);
  MPI_Type_commit(&mpi_result_datatype);

  int nprocs;
  int myid;

  int scale_real=(imag_max-imag_min)/display_width;
  int scale_imag=(real_max-real_min)/display_height;

  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Status s;

  if (myid == 0) { //master
    int busy_slave_count = 0;
    int unproc_row_num = 0;
    for (int i = 1; i < nprocs; i++) { //give each slave a starting row
      MPI_Send(&unproc_row_num,1,MPI_INT,i,DATA_TAG,MPI_COMM_WORLD);
      busy_slave_count++;
      unproc_row_num++;
    }
    do {
      struct result r;
      MPI_Recv(&r,1,mpi_result_datatype,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
      busy_slave_count--;
      if(unproc_row_num < display_height) { //send row to slave
        MPI_Send(&unproc_row_num,1,MPI_INT,r.slave_id,DATA_TAG, MPI_COMM_WORLD);
        unproc_row_num++;
        busy_slave_count++;
      }else{ //kill slave
        MPI_Send(&unproc_row_num,1,MPI_INT,r.slave_id,KILL_TAG, MPI_COMM_WORLD);
      }
      output(r.row_num, r.color);
    } while (busy_slave_count > 0);
  }
  else { //slave
    struct result r;
    int unproc_row_num;
    MPI_Recv(&unproc_row_num,1,MPI_INT,MASTER_P,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
    while(s.MPI_TAG == DATA_TAG) {
      struct complx c;
      c.imag = imag_min + ((float) unproc_row_num * scale_imag);
      for (int col_num = 0; col_num < display_width; col_num++) {
        c.real = real_min + ((float) col_num * scale_real);
        r.color[col_num] = cal_pixel(c);
      }
      r.slave_id = myid;
      r.row_num = unproc_row_num;
      MPI_Send(&r,1,mpi_result_datatype,0,DATA_TAG,MPI_COMM_WORLD);
      MPI_Recv(&unproc_row_num,1,MPI_INT,MASTER_P,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
    }
  return 0;
  }
}

int cal_pixel(struct complx c)
{
  int count, max_iter;
  struct complx z;
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
  } while ((lengthsq < 4.0) && (count < max_iter));
return count;
}
    
int output(int row, int color[])
{
  int length = sizeof(color) / sizeof(int);
  ofstream file_handle;
  file_handle.open("output.csv");
  file_handle << row+"";
  for (int i=0;i<length;i++){
    file_handle << ","+color[i];
  }
  file_handle << "\n";
  file_handle.close();
  return 0;
}

