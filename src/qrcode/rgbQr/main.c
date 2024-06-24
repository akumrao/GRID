

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include <assert.h>

#include <zbar.h>

#ifdef ENABLE_NLS
#include <libintl.h>
#include <locale.h>
#define _(string) gettext(string)
#else
#define _(string) string
#endif

#define N_(string) string





#ifdef HAVE_DBUS
static const char *note_usage2 =
    N_("    --nodbus        disable dbus message\n");
#endif

static const char *warning_not_found_head = N_(
    "\n"
    "WARNING: barcode data was not detected in some image(s)\n"
    "Things to check:\n"
    "  - is the barcode type supported? Currently supported symbologies are:\n");

static const char *warning_not_found_tail = N_(
    "  - is the barcode large enough in the image?\n"
    "  - is the barcode mostly in focus?\n"
    "  - is there sufficient contrast/illumination?\n"
    "  - If the symbol is split in several barcodes, are they combined in one "
    "image?\n"
    "  - Did you enable the barcode type?\n"
    "    some EAN/UPC codes are disabled by default. To enable all, use:\n"
    "    $ zbarimg -S*.enable <files>\n"
    "    Please also notice that some variants take precedence over others.\n"
    "    Due to that, if you want, for example, ISBN-10, you should do:\n"
    "    $ zbarimg -Sisbn10.enable <files>\n"
    "\n");

static const char *xml_head =
    "<barcodes xmlns='http://zbar.sourceforge.net/2008/barcode'>\n";
static const char *xml_foot = "</barcodes>\n";

//static int notfound = 0, exit_code = 0;
//static int num_images = 0, num_symbols = 0;
static int xmllvl  = 0;
//static int polygon = 0;
//static int oneshot = 0;
//static int binary  = 0;

//char *xmlbuf	   = NULL;
unsigned xmlbuflen = 0;

static zbar_processor_t *processor = NULL;

/*
static inline int dump_error(MagickWand *wand)
{
    char *desc;
    ExceptionType severity;
    desc = MagickGetException(wand, &severity);

    if (severity >= FatalErrorException)
	exit_code = 2;
    else if (severity >= ErrorException)
	exit_code = 1;
    else
	exit_code = 0;

    static const char *sevdesc[] = { "WARNING", "ERROR", "FATAL" };
    fprintf(stderr, "%s: %s\n", sevdesc[exit_code], desc);

    MagickRelinquishMemory(desc);
    return (exit_code);
}*/

void saveFile( char *buf, int len)
{
   FILE *fp=NULL;
   
   fp = fopen( "./test.rgba" , "wb" );
   if(fp)
   fwrite(buf , 1 ,len , fp );

   fclose(fp);
    
}


int readFile(const char* filename,  char *buf, int len)
{
   FILE *fp=NULL;
   
   fp = fopen( filename , "rb" );
   
   if(fp)
   fread(buf , 1 ,len , fp );
   else 
       return -1;

   fclose(fp);
   
   return 0;
    
}


static int scan_image(unsigned char *blob, int width, int height)
{

    int found	       = 0;
    unsigned seq = 0;
    //MagickWand *images = NewMagickWand();

    // default is a measly 72dpi for pdf
   // MagickSetResolution(images, 900, 900);

    //if (!MagickReadImage(images, filename) && dump_error(images))
	//return (-1);

   //  n = MagickGetNumberImages(images);
   // for (seq = 0; seq < n; seq++) {
	//if (exit_code == 3)
	 //   return (-1);

    {
    	++seq;	
	//if (!MagickSetImageIndex(images, seq) && dump_error(images))
	 //   return (-1);

	zbar_image_t *zimage = zbar_image_create();
	assert(zimage);
	zbar_image_set_format(zimage, zbar_fourcc('R', 'G', 'B', '3'));


	zbar_image_set_size(zimage, width, height);

	// extract grayscale image pixels
	// FIXME color!! ...preserve most color w/422P
	// (but only if it's a color image)
	size_t bloblen	    = width * height*3;
        
	//unsigned char *blob = malloc(bloblen);
	zbar_image_set_data(zimage, blob, bloblen, zbar_image_free_data);

	//if (!MagickGetImagePixels(images, 0, 0, width, height, "RGB", CharPixel,  blob))
            
       
        
         //saveFile(blob, bloblen );

	if (xmllvl == 1) {
	    xmllvl++;
	   // printf("<source href='%s'>\n", filename);
	}

	zbar_process_image(processor, zimage);

	// output result data
	const zbar_symbol_t *sym = zbar_image_first_symbol(zimage);
	for (; sym; sym = zbar_symbol_next(sym)) {
	    zbar_symbol_type_t typ = zbar_symbol_get_type(sym);
	    unsigned len	   = zbar_symbol_get_data_length(sym);
	    if (typ == ZBAR_PARTIAL)
		continue;
	    else if (xmllvl <= 0) {
		if (!xmllvl)
		    printf("%s:", zbar_get_symbol_name(typ));

		if (len &&
		    fwrite(zbar_symbol_get_data(sym), len, 1, stdout) != 1) {
		    
		    return (-1);
		}
	    } 
//            else
//            {
//		if (xmllvl < 3) {
//		    xmllvl++;
//		    printf("<index num='%u'>\n", seq);
//		}
//		zbar_symbol_xml(sym, &xmlbuf, &xmlbuflen);
//		if (fwrite(xmlbuf, xmlbuflen, 1, stdout) != 1) {
//		    exit_code = 1;
//		    return (-1);
//		}
//	    }
	    found++;
	   // num_symbols++;

	   
	    printf("\n");
	   
	}

	fflush(stdout);

	zbar_image_destroy(zimage);

//	num_images++;
//	if (zbar_processor_is_visible(processor)) {
//	    int rc = zbar_processor_user_wait(processor, -1);
//	    if (rc < 0 || rc == 'q' || rc == 'Q')
//		exit_code = 3;
//	}
    }

    if (xmllvl > 1) {
	xmllvl--;
	printf("</source>\n");
    }



    //DestroyMagickWand(images);
    return found;
}


//static inline int parse_config(const char *cfgstr, const char *arg)
//{
//    if (!cfgstr || !cfgstr[0])
//	return (usage(1, "ERROR: need argument for option: ", arg));
//
//    if (zbar_processor_parse_config(processor, cfgstr))
//	return (usage(1, "ERROR: invalid configuration setting: ", cfgstr));
//
//    if (!strcmp(cfgstr, "binary"))
//	binary = 1;
//
//    return (0);
//}

int main(int argc, const char *argv[])
{
    // option pre-scan

    int i=0;

    int  display = 0;



   // InitializeMagick("zbarimg");

    processor = zbar_processor_create(0);
    assert(processor);



    if (zbar_processor_init(processor, NULL, display)) {
	zbar_processor_error_spew(processor, 0);
	return (1);
    }

    
    int width  = 100;
    int height =100;
        
    zbar_increase_verbosity();

    if( argc > 1)
    {
        for (i=1; i < argc; i++)
        {
            size_t bloblen	    = width * height*3;
            unsigned char *blob = malloc(bloblen);
            
            if(readFile(argv[i], blob, bloblen ) )                        
              return (-1);

            if (scan_image(blob, width, height ))
            {  
                printf( "found QR \n" );
            }
            else
            {
                 fprintf(stderr, "%s", _(warning_not_found_tail));
            }
            
           // free(blob);
        }
    }

    else 
    {
        size_t bloblen	    = width * height*3;
	unsigned char *blob = malloc(bloblen);
        
        if(readFile("./test.rgb", blob, bloblen ) )                        
        return (-1);
        
        
        scan_image( blob, width, height);
        
       // free(blob);
         
    }

   

   

    zbar_processor_destroy(processor);
    //DestroyMagick();
    return (0);
}
