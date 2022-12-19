// by Anurag and Jainav
// this program extracts the RBG values from the tiff image
#include<stdio.h>
#include<stdlib.h>
#include"declarations.h"

unsigned char **open_tiff_file(FILE *fp,short int *width_ptr,short int *height_ptr)
{
    struct TIFF_header header;
    struct TIFF_ifd ifd;
    struct TIFF_dir_entry dir_entry;
    struct TIFF_RGB colour;
    
    fread(header.byte_order, 2, 1, fp);
    fread(&header.version, 6, 1, fp);

    // Taking the file pointer to the first IFD location.
    fseek(fp, header.offset, SEEK_SET);
    fread(&ifd.num_dir, 2, 1, fp); 
    
    int k=0;
    while(k<ifd.num_dir) // loops over all the tags in the ifd to find image width and height
    {
        fread(&dir_entry.tag, 12, 1, fp);
        if(dir_entry.tag == 256)         // 256 is the tag for image width
            *width_ptr = dir_entry.value;
        if(dir_entry.tag == 257)         // 257 is the tag for image height
            *height_ptr = dir_entry.value;
        k++;
    }

    /* to allocate memory to array of rbg values for sample tiff image */
    unsigned char **colours_rbg;
    colours_rbg = (unsigned char**)malloc(sizeof(unsigned char*)*(*height_ptr));
    for(int i=0;i<*height_ptr;i++){
        colours_rbg[i] = (unsigned char*)malloc(sizeof(unsigned char)*(*width_ptr*3));
    }

    /* to get rbg values of the image and store it in an array */
    fseek(fp, 8, SEEK_SET);
    int j=0, i=0;
    while(i<*height_ptr){
        j = 0;
        while(j<*width_ptr*3){
            // printf("%d %d",i,j);
            fread(&colour.red, 3, 1, fp);
            colours_rbg[i][j] = colour.red;
            colours_rbg[i][j+1] = colour.green;
            colours_rbg[i][j+2] = colour.blue;
            // printf(" RGB values is : %d %d %d\n",colour.red,colour.green,colour.blue);
            j += 3;
        }
        i++;
    }
    fclose(fp);

    return colours_rbg;
}

