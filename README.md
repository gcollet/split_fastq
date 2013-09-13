split_fastq
===========

This small piece of code splits a fastq file in n fastq files

Because fastq files may be very large files (like 22 GB in my case), we need to cut them in pieces of less than 4 GB in order to store them on a FAT32 formatted disk.
This is why I created this code.

But cutting 22 GB of data is not so simple if you want to do it quickly.
I did it in C, with the mmap functions that let you map an entire file in virtual memory.
split_fastq finds the cut positions, and then simply copies memory and lets the system do the writing on disk.

Some performance evaluations :
5' 30'' to split a 22 Go file in two 11 Go files.
2' 47'' to split a 11 Go file in two 5,7 Go files.
2' 27'' to split a 11 Go file in four 2,8 Go files.
2' 10'' to split a 11 Go file in six 1,9 Go files.

Usage: ./split_fastq <fastq_file> <nb_files>

Guillaume Collet.