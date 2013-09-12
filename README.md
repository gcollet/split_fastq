split_fastq
===========

This small piece of code splits a fastq file in n fastq files

Because fastq files may be very large files (like 24 GB in my case), we need to cut them in pieces of less than 4 GB in order to store them on a FAT32 formatted disk.
This is why I created this code.

But cutting 24 GB of data is not so simple if you want to do it quickly, I did it in C, with the mmap functions that let you map an entire file in virtual memory.
Then I find the cut positions, and then I simply copy memory and let the system do the writing on disk.

Cutting 24 GB in two files took 5 minutes on my laptop.

Feel free to use this code ;-)

Guillaume Collet.