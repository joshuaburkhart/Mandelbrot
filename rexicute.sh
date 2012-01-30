mpirun -np $1 ./mandelbrot -x $2 -y $3 -s $4 -w $5 -h $6
R --no-save << EOT
A = scan(file="output.csv",sep=",")
A = matrix(A,nrows=$6,ncol=$5)
A = A[,ncol(A):1]
image(A)
quit("no")
EOT
evince "./Rplots.pdf"
