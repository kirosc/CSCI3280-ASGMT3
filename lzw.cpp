/*
* CSCI3280 Introduction to Multimedia Systems *
* --- Declaration --- *
* I declare that the assignment here submitted is original except for source
* material explicitly acknowledged. I also acknowledge that I am aware of
* University policy and regulations on honesty in academic work, and of the
* disciplinary guidelines and procedures applicable to breaches of such policy
* and regulations, as contained in the website
* http://www.cuhk.edu.hk/policy/academichonesty/ *
* Assignment 3
* Name :
* Student ID :
* Email Addr :
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unordered_map>
#include <iostream>

#define CODE_SIZE  12
#define TRUE 1
#define FALSE 0

using namespace std;

/* function prototypes */
unsigned int read_code(FILE*, unsigned int); 
void write_code(FILE*, unsigned int, unsigned int); 
void writefileheader(FILE *,char**,int);
void readfileheader(FILE *,char**,int *);
void compress(FILE*, FILE*);
void decompress(FILE*, FILE*);
unordered_map<string, unsigned int> initialize_comp_dict();
unordered_map<unsigned int, string> initialize_decomp_dict();

int main(int argc, char **argv)
{
    int printusage = 0;
    int	no_of_file;
    char **input_file_names;    
	char *output_file_names;
    FILE *lzw_file;

    if (argc >= 3)
    {
		if ( strcmp(argv[1],"-c") == 0)
		{		
			/* compression */
			lzw_file = fopen(argv[2] ,"wb");
        
			/* write the file header */
			input_file_names = argv + 3;
			no_of_file = argc - 3;
			writefileheader(lzw_file,input_file_names,no_of_file);
        	        	
			/* ADD CODES HERE */
            // Compress files one by one
            for (int i = 3; argv[i] != nullptr; ++i) {
                printf("Adding : %s\n", argv[i]);
                FILE *text_file;
                text_file = fopen(argv[i] ,"rb");
                compress(text_file, lzw_file);
                fclose(text_file);
            }

            write_code(lzw_file, 0, 8);
            fclose(lzw_file);
		} else
		if ( strcmp(argv[1],"-d") == 0)
		{	
			/* decompress */
			lzw_file = fopen(argv[2] ,"rb");
			
			/* read the file header */
			no_of_file = 0;			
			readfileheader(lzw_file,&output_file_names,&no_of_file);
			
			/* ADD CODES HERE */

            FILE *output_file;
            string str = string(output_file_names);
            unsigned int end_pos = str.find("\n\n");

            // Delimit file names by '\n'
            str = str.substr(0, end_pos + 1);
            strcpy(output_file_names, str.c_str());
            output_file_names = strtok(output_file_names, "\n");

            // Decompress file one by one
            while(output_file_names != nullptr) {
                printf("Deflating: %s\n", output_file_names);
                output_file = fopen(output_file_names ,"wb");
                decompress(lzw_file, output_file);
                output_file_names = strtok(nullptr, "\n");
            }

			fclose(lzw_file);
			free(output_file_names);
		}else
			printusage = 1;
    }else
		printusage = 1;

	if (printusage)
		printf("Usage: %s -<c/d> <lzw filename> <list of files>\n",argv[0]);
 	
	return 0;
}

/*****************************************************************
 *
 * writefileheader() -  write the lzw file header to support multiple files
 *
 ****************************************************************/
void writefileheader(FILE *lzw_file,char** input_file_names,int no_of_files)
{
	int i;
	/* write the file header */
	for ( i = 0 ; i < no_of_files; i++) 
	{
		fprintf(lzw_file,"%s\n",input_file_names[i]);	
			
	}
	fputc('\n',lzw_file);

}

/*****************************************************************
 *
 * readfileheader() - read the fileheader from the lzw file
 *
 ****************************************************************/
void readfileheader(FILE *lzw_file,char** output_filenames,int * no_of_files)
{
	int noofchar;
	char c,lastc;

	noofchar = 0;
	lastc = 0;
	*no_of_files=0;
	/* find where is the end of double newline */
	while((c = fgetc(lzw_file)) != EOF)
	{
		noofchar++;
		if (c =='\n')
		{
			if (lastc == c )
				/* found double newline */
				break;
			(*no_of_files)++;
		}
		lastc = c;
	}

	if (c == EOF)
	{
		/* problem .... file may have corrupted*/
		*no_of_files = 0;
		return;
	
	}
	/* allocate memeory for the filenames */
	*output_filenames = (char *) malloc(sizeof(char)*noofchar);
	/* roll back to start */
	fseek(lzw_file,0,SEEK_SET);

	fread((*output_filenames),1,(size_t)noofchar,lzw_file);
	
	return;
}

/*****************************************************************
 *
 * read_code() - reads a specific-size code from the code file
 *
 ****************************************************************/
unsigned int read_code(FILE *input, unsigned int code_size)
{
    unsigned int return_value;
    static int input_bit_count = 0;
    static unsigned long input_bit_buffer = 0L;

    /* The code file is treated as an input bit-stream. Each     */
    /*   character read is stored in input_bit_buffer, which     */
    /*   is 32-bit wide.                                         */

    /* input_bit_count stores the no. of bits left in the buffer */

    while (input_bit_count <= 24) {
        input_bit_buffer |= (unsigned long) getc(input) << (24-input_bit_count);
        input_bit_count += 8;
    }

    return_value = input_bit_buffer >> (32 - code_size);
    input_bit_buffer <<= code_size;
    input_bit_count -= code_size;

    return(return_value);
}


/*****************************************************************
 *
 * write_code() - write a code (of specific length) to the file 
 *
 ****************************************************************/
void write_code(FILE *output, unsigned int code, unsigned int code_size)
{
    static int output_bit_count = 0;
    static unsigned long output_bit_buffer = 0L;

    /* Each output code is first stored in output_bit_buffer,    */
    /*   which is 32-bit wide. Content in output_bit_buffer is   */
    /*   written to the output file in bytes.                    */

    /* output_bit_count stores the no. of bits left              */    

    output_bit_buffer |= (unsigned long) code << (32-code_size-output_bit_count);
    output_bit_count += code_size;

    while (output_bit_count >= 8) {
        putc(output_bit_buffer >> 24, output);
        output_bit_buffer <<= 8;
        output_bit_count -= 8;
    }


    /* only < 8 bits left in the buffer                          */    

}

/*****************************************************************
 *
 * compress() - compress the source file and output the coded text
 *
 ****************************************************************/
void compress(FILE *input, FILE *output)
{
	/* ADD CODES HERE */
	char c;
    string p, c_str;
    unsigned int code;
    static unordered_map<string, unsigned int> comp_dict = initialize_comp_dict();

    while((c = fgetc(input)) != EOF) {
        if (comp_dict.find(p + c) != comp_dict.end()) {
            // FOUND
            code = comp_dict[p + c];
            p += c;
        } else {
            write_code(output, code, CODE_SIZE);
            comp_dict[p + c] = comp_dict.size();
            p = c;
            code = comp_dict[p];
            // Dictionary is full
            if (comp_dict.size() == 4095) {
                comp_dict = initialize_comp_dict();
            }
        }
    }
    write_code(output, comp_dict[p], CODE_SIZE);
    // EOF
    write_code(output, 4095, CODE_SIZE);
}


/*****************************************************************
 *
 * decompress() - decompress a compressed file to the orig. file
 *
 ****************************************************************/
void decompress(FILE *input, FILE *output)
{
	/* ADD CODES HERE */
    unsigned int pw, cw;
    string key_string, c, str;
    static unordered_map<unsigned int, string> decomp_dict = initialize_decomp_dict();

    pw = read_code(input, CODE_SIZE);
    key_string = decomp_dict[pw];
    fputs(key_string.c_str(), output);

    while ((cw = read_code(input, CODE_SIZE)) != 4095) {
        if (decomp_dict.find(cw) != decomp_dict.end()) {
            // FOUND
            str = decomp_dict[cw];
            c = str[0];
        } else {
            str = decomp_dict[pw];
            c = str[0];
            str += c;
        }
        fputs(str.c_str(), output);
        decomp_dict[decomp_dict.size()] = decomp_dict[pw] + c;
        pw = cw;

        // Dictionary is full
        if (decomp_dict.size() == 4095) {
            decomp_dict = initialize_decomp_dict();
            pw = read_code(input, CODE_SIZE);
            key_string = decomp_dict[pw];
            fputs(key_string.c_str(), output);
        }
    }
}

// Initialize the first 256 ASCII codes for compression
unordered_map<string, unsigned int> initialize_comp_dict() {
    unordered_map<string, unsigned int> dict;
    for (int i = 0; i < 256; ++i) {
        dict[string(1, (char) i)] = i;
    }

    return dict;
}

// Initialize the first 256 ASCII codes for decompression
unordered_map<unsigned int, string> initialize_decomp_dict() {
    unordered_map<unsigned int, string> dict;
    for (int i = 0; i < 256; ++i) {
        dict[i] = string(1, (char) i);
    }

    return dict;
}
