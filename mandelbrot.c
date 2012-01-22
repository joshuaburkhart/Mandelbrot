#include <iostream>
using std::cout;
using std::endl;

#include <string>
using std::string;

#include "mpi.h"

structure complex
{
  float real;
  float imag;
};

int cal_pixel(complex c);
int dislpay(int row, int color[]);

int main(int argc, char *argv[])
{
  MPI_Init(&argc,&argv);

  int nprocs;
  int myid;

  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Status s;

  if (myid == 0) { //master
    slave_count = 0;
    unproc_row_num=0;
    for (int i = 1; i < nprocs; i++) {
      MPI_Send(unproc_row_num,1,MPI_INT,i,data_tag,MPI_COMM_WORLD);
      busy_slave_count++;
      unproc_row_num++;
    }
    do {
      MPI_Recv(slave_id, proc_row_num, color,3,MPI_INT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&s);
      busy_slave_count--;
      if(unproc_row_num < disp_height) { //deal unprocessed row to slave
        MPI_Send(unproc_row_num,1,MPI_INT,slave_id,data_tag, MPI_COMM_WORLD);
        unproc_row_num++;
        busy_slave_count++;
      }else{ //deal death to slave
        MPI_Send(-1,1,MPI_INT,slave_id,death_tag, MPI_COMM_WORLD);
      }
      output(proc_row_num, color);
    } while (slave_count > 0);
  }
  else { //slave
    int unproc_row_num;
    MPI_Recv(unproc_row_num,1,MPI_INT,master,source_tag,MPI_COMM_WORLD,&s);
    while(source_tag == data_tag) {
      c.imag = imag_min + ((float) unproc_row_num * scale_imag);
      for (col_num = 0; col_num < disp_width; col_num++) {
        c.real = real_min + ((float) col_num * scale_real);
        color[col_num] = cal_pixel(c);
      }
      MPI_Send(my_id,unproc_row_num,color,3,MPI_INT,0,data_tag,MPI_COMM_WORLD);
      MPI_Recv(unproc_row_num,1,MPI_INT,0,source_tag,MPI_COMM_WORLD,&s);
    }
  return 0;
  }
}

int cal_pixel(complex c)
{
  int count, max_iter;
  complex z;
  float temp, lengthsq;
  max_iter = 256;
  z.real = 0;
  z. imag = 0;
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
    
int output(int row, color[])
{
  ofstream file_handle;
  file_handle.open("output.csv");
  file_handle << row+"";
  for (int i=0,i<color.length,i++){
    file_handle << ","+color[i];
  }
  file_handle << "\n";
  file_handle.close();
  return 0;
}

