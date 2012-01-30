mpirun -np $1 ./mandelbrot -x $2 -y $3 -s $4 -w $5 -h $6
R --no-save << EOT
A = scan(file="output.csv",sep=",")
dim(A) = c($5,$6)
image(A)
quit("no")
EOT
evince "./Rplots.pdf"
