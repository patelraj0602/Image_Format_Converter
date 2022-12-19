#include<stdio.h>

//function declarations for reading tiff image
unsigned char **open_tiff_file(FILE*,short *,short *);

//function declarations for writing gif image
void header(FILE *);
void logical_screen_descriptor(FILE *, short, short);
unsigned char * global_color_table(FILE *, unsigned char **, short, short);
void image_descriptor(FILE *, short, short);
void image_data(FILE *, unsigned char *, short, short);

// by Prabal
//tiff structures
struct TIFF_header
{
    char byte_order[2];
    unsigned short int version;
    unsigned int offset; 
};

struct TIFF_ifd
{
    short int num_dir;
};

struct TIFF_dir_entry
{
    short int tag;
    short int type;
    int length;
    long int value;
};

struct TIFF_RGB
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
};

// by Shashank and Shridhar
// gif structures
struct header_gif
{
    char signature[3]; 
    char version[3];
};

struct logical_screen_descriptor
{
    unsigned char canvas_width[2];
    unsigned char canvas_height[2];
    unsigned char packed_field;
    unsigned char background_color_index;
    unsigned char pixel_aspect_ratio;
};

struct image_descriptor
{
    unsigned char image_separator;
    unsigned char image_left[2];
    unsigned char image_top[2];
    unsigned char image_width[2];
    unsigned char image_height[2];
    unsigned char packed_field;
};
