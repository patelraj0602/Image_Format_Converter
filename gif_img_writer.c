// This function writes the GIF using the RGB values extracted from TIFF image
#include<stdio.h>
#include<stdlib.h>
#include"declarations.h"

// Writes the HEADER (Size = 6 bytes) to the gif 
void header(FILE *gif)
{
    struct header_gif
    {
        char signature[3]; // Always "GIF"
        char version[3];   // GIF version - Can be "87a" or "89a". The latter is newest version used to make animated gifs
    };
    struct header_gif header = {"GIF", "87a"}; // We are writing "87a" gifs because of greater compatibility and non-animated conversion    
    fwrite(&header, 6, 1, gif);
}

// This function writes the LOGICAL SCREEN DESCRIPTOR (Size = 7 bytes) to the gif
void logical_screen_descriptor(FILE *gif, short width, short height){

    struct logical_screen_descriptor{
        unsigned char canvas_width[2];
        unsigned char canvas_height[2];
        unsigned char packed_field; 
        unsigned char background_color_index;
        unsigned char pixel_aspect_ratio; 
    } gif_lcd;

    // Writing width and height in little endian format
    gif_lcd.canvas_width[0] = width;        // canvas_dimensions = image_dimensions, if the gif contains only 1 image
    gif_lcd.canvas_width[1] = width >> 8;

    gif_lcd.canvas_height[0] = height;
    gif_lcd.canvas_height[1] = height >> 8;

    // packed field = global color table flag (1bit) + color resolution (3bits) + sort flag(1bit) + size of global color table (3bits)
    gif_lcd.packed_field = 128*(1) + 16*(1) + 8*(0) + 1*(7);

    gif_lcd.background_color_index = 0x00; // color index of the pixels of canvas not overlayed by image. No use for us since canvas_size = image_size in our case 
    gif_lcd.pixel_aspect_ratio = 0x00;     // used to represent image captured from analog televisions, no longer in use today

    fwrite(&gif_lcd, 7, 1, gif);
}

// Writes the Global Color Table (Size = varies) and converts the RGB_array to array of indexed values according to Color table
unsigned char * global_color_table(FILE *gif, unsigned char **array, short width, short height)
{
    unsigned char color_table[256][3];
    unsigned char *indexed_pixels; // pointer to the array containg the indexed pixels
    indexed_pixels = (unsigned char*)malloc(sizeof(unsigned char)*(width*height)); // allocating memory dynamically for array of indexed values of pixels
    int pixel_count = 0;

    int count = 0;
    // first color in table = Color of first pixel
    color_table[count][0] = *(*(array + 0) + 0);
    color_table[count][1] = *(*(array + 0) + 1);
    color_table[count][2] = *(*(array + 0) + 2);       
    int temp = 0;

    for(int i = 0; i < height; i++){
        for(int j = 0; j < width*3 ; j += 3){
            // This loop iterates through the color table, checks if the color is present in the table and if its not present then adds it to the table
            for(int k = 0; k <= count; k++){
                if(*(*(array + i) + j) == color_table[k][0] && *(*(array + i) + j + 1) == color_table[k][1] && *(*(array + i) + j + 2) == color_table[k][2]){    
                    temp++;
                    break;
                }
            }       
            if(temp == 0){
                count++;
                color_table[count][0] = *(*(array + i) + j);
                color_table[count][1] = *(*(array + i) + j + 1);
                color_table[count][2] = *(*(array + i) + j + 2);
            }
            temp = 0;
            
            // This loop indexes the pixels according to the color table
            for(int k = 0; k <= count; k++){
                if(*(*(array + i) + j) == color_table[k][0] && *(*(array + i) + j + 1) == color_table[k][1] && *(*(array + i) + j + 2) == color_table[k][2]){
                    indexed_pixels[pixel_count] = k;
                    break;
                }
            }
            pixel_count++;
        }
    }
    // Adding extra colors in case the image contains only less colors because the aize of color table is fixed 4, 8, 16,....
    for(int i = count; i < 256; i++)
    {
        count++;
        color_table[count][0] = 0x00;
        color_table[count][1] = 0x00;
        color_table[count][2] = 0x00;
    }

    for(int i = 0; i <= count; i++)
    {
        fwrite(&color_table[i], 3, 1, gif);
    }
    
    return indexed_pixels;
}

// Writes the IMAGE DESCRIPTOR (Size = 10 bytes) to the gif
void image_descriptor(FILE *gif, short width, short height)
{
    struct image_descriptor
    {
        unsigned char image_separator;
        unsigned char image_left[2];
        unsigned char image_top[2];
        unsigned char image_width[2];
        unsigned char image_height[2];
        unsigned char packed_field;
    } id;

    id.image_separator = 0x2C;  // Every image descriptor begins with 2C
    id.image_left[0] = 0;
    id.image_left[1] = 0;       // Position where image should begin on canvas
    id.image_top[0] = 0;
    id.image_top[1] = 0;        // Position where image should begin on canvas

    // image dimensions in little endian form
    id.image_width[0] = width; 
    id.image_width[1] = width >> 8;

    id.image_height[0] = height;
    id.image_height[1] = height >> 8;

    id.packed_field = 0;        // Describes the details about local color table. It is zero because we are not using local color table

    fwrite(&id, 10, 1, gif);
}

/* Structure of Image Data = 
    {
        Minimum Code Size
        {
            Size Of Sub-block = x
            {
                Clear Code
                Index of pixel 1
                Index of pixel 2
                .
                .
                Index of pixel x - 1
            }
            Size of Sub-block = y
            {
                Clear Code
                Index of pixel (x - 1) + 1
                .
                .
                Index of pixel x + y - 2
            }
            .
            .
            Size of sub-block = 1
            End OF Information Code
            Size of sub-block = 0
            End of GIF
        }
    }
*/
// Writes the IMAGE DATA (Size = depends on the image) to the gif
void image_data(FILE *gif, unsigned char *indexed_pixels, short width, short height)
{
    /* The Minimum Code Size is the number of bits used per pixel.
       However after taking into account the 2 extra indexes (Clear code and EOI code), the code size used becomes MinCodeSize + 1
       So, here min_code_size = 07 means that 1 byte (7 + 1 bits) is used to denote each pixel 
    */
    char min_code_size = 0x07;                 
    
    /* First byte of each sub-block tells us how many bytes of actual data follow. 
       Its 101 (0x65) because each sub-block contains indexes for 100 pixels + 1 clear code
    */    
    char no_bytes_in_each_subblock = 0x65;

    char clear_code = 0x80;       // Value of clear code depends on our Min Code Size. For min_code_size = 7, clear code is 128 (0x80)
    char EOI_code = 0x81;         // End of Information code also depends on Min Code Size. For 7, its value is 129 (0x81)
    char end_of_gif = 0x3B;       // Every GIF ends with 0x3B

    fwrite(&min_code_size, 1, 1, gif);

    register unsigned int count = 0;                     // Register int because count will be used a large number of times
    for(int i = 0; i < ((width*height) / 100); i++)      // each subblock denotes 100 pixels so no_subblocks = (width*height) / 100  
    {
        fwrite(&no_bytes_in_each_subblock, 1, 1, gif);
        fwrite(&clear_code, 1, 1, gif);

        // Loop for writing values of next 100 pixels
        for(int j = 0; j < 100; j++)
        {
            fwrite(indexed_pixels + count, 1, 1, gif);
            count++;
        }
    }

    // Writing the EOI code which denotes the end of image
        
    no_bytes_in_each_subblock = (width*height) - ((width*height)/100)*100 + 1 + 1;
    fwrite(&no_bytes_in_each_subblock, 1, 1, gif);
    fwrite(&clear_code, 1, 1, gif);
    for(int i = 0; i < (no_bytes_in_each_subblock - 2); i++)
    {
        fwrite(indexed_pixels + count, 1, 1, gif);
        count++;
    }
    fwrite(&EOI_code, 1, 1, gif);

    // This tells the gif reader that no more sub-blocks follow since number of bytes in next subblock = 0
    no_bytes_in_each_subblock = 0;
    fwrite(&no_bytes_in_each_subblock, 1, 1, gif);

    // End of the GIF. Every GIF ends with 0x3B
    fwrite(&end_of_gif, 1, 1, gif);
}

/* END OF PROGRAM */