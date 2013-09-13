/*
 The MIT License (MIT)
 
 Copyright (c) 2013 Guillaume Collet
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>

int main(int argc, char *argv[])
{
	// Read arguments
	if (argc != 3) {
		std::cout << "\nUsage: ./split_fastq <fastq_file> <nb_parts>\n\n";
		return 1;
	}
	std::string fastq_file_name = argv[1];
	int nb_parts = atoi(argv[2]);
	std::string prefix = fastq_file_name.substr(0, fastq_file_name.rfind("."));
	std::string ext = fastq_file_name.substr(fastq_file_name.rfind(".") + 1);
	
	////////////////////////////////////////////////////////////////////////////
	// Open and map the input file
	//
	
	// Open input file
	int fdin = open (fastq_file_name.c_str(), O_RDONLY);
	if (fdin  < 0){
		std::cerr << "can't open " << fastq_file_name << " for reading -> exit\n";
		exit(1);
	}
	
	// find size of input file
	struct stat statbuf;
	if (fstat (fdin, &statbuf) < 0){
		std::cerr << "fstat error on " << fastq_file_name << " -> exit\n";
		exit(1);
	}
	
	// mmap the input file
	char * src = (char *) mmap (0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0);
	if (src == NULL){
		std::cerr << "mmap error on " << fastq_file_name << " -> exit\n";
		exit(1);
	}
	
	////////////////////////////////////////////////////////////////////////////
	// Calculate the index of the beginning of each sub-file and its size
	//
	size_t sub_index [nb_parts];
	size_t sub_size [nb_parts];
	
	sub_index[0] = 0;	
	size_t default_size = statbuf.st_size / nb_parts;
	for (int i = 1; i < nb_parts; i++) {
		sub_index[i] = default_size;
		while (sub_index[i] < statbuf.st_size) {
			if (src[sub_index[i]] == '@' && src[sub_index[i] - 1] == '\n') {
				break;
			}
			sub_index[i]++;
		}
		sub_size [i - 1] = sub_index[i] - sub_index[i - 1];
		default_size += statbuf.st_size / nb_parts;
	}
	sub_size [nb_parts - 1] = statbuf.st_size - sub_index[nb_parts - 1];

	////////////////////////////////////////////////////////////////////////////
	// Open, map, and copy the output files
	//
	for (int i = 0; i < nb_parts; i++) {
		std::stringstream output_file_name;
		output_file_name << prefix << "_" << i << "." << ext;
		
		// open/create the output file
		int fdout = open (output_file_name.str().c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
		if (fdout < 0){
			std::cerr << "can't create file number " << output_file_name.str() << " for writing-> exit\n";
			exit(1);
		}
		
		// go to the location corresponding to the last byte
		if (lseek (fdout, sub_size[i] - 1, SEEK_SET) == -1) {
			std::cerr << "lseek error on " << output_file_name.str() << "-> exit\n";
			exit(1);
		}
		
		// write a dummy byte at the last location
		if (write (fdout, "", 1) != 1){
			std::cerr << "write error on " << output_file_name.str() << "-> exit\n";
			exit(1);
		}
		
		// mmap the output file
		char * dst = (char *) mmap (0, sub_size[i], PROT_READ | PROT_WRITE, MAP_SHARED, fdout, 0);
		if (dst == NULL) {
			std::cerr << "mmap error on " << output_file_name.str() << "-> exit\n";
			exit(1);
		}
		
		// copy the bunch of data
		memcpy (dst, src + sub_index[i], sub_size[i]);
		
		// unmap output file
		if (munmap(dst, sub_size[i])){
			std::cerr << "munmap error on " << output_file_name.str() << " -> exit\n";
		}
		
		// close output file
		close (fdout);
	}
	
	// unmap input file
	if (munmap(src, statbuf.st_size)){
		std::cerr << "munmap error on " << fastq_file_name << " -> exit\n";
	}
	
	close (fdin);
	
    return 0;
}
