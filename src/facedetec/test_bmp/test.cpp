

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


#include "base/rgba_bitmap.h"




int main(int argc, char** argv) {

    std::cout << "bmp ./fileNmae"<< std::endl;

    std::string path = argv[1];

    std::cout << "bmp " << path << std::endl;

    
    std::ifstream f(path); //taking file as inputstream
    std::string str;
    if(f) {
       std::stringstream ss;
       ss << f.rdbuf(); // reading data
       str = ss.str();
    }
    else
    {
       std::cout << " rgba file does not exist " << std::endl;
        return -1;
    }

  
           
  unsigned long  width = 640;
  unsigned long  height = 360;
  size_t  p_output_size = 0;
  
    
  unsigned char *  bgrBuf = rgba_to_rgb_brg( (const unsigned char*) str.c_str() , str.length(), bitmap_buffer_format_BGR , 0, width , height , &p_output_size );
  write_bmp(bgrBuf, width, height, "./arvind.bmp"  );
  free(bgrBuf) ;
  
 
  





}
