/* CUSTOMIZE BELOW HERE  #######################################################

./configure --pkg-config-flags="--static" --libdir=/usr/local/lib --disable-shared --enable-debug=2 --disable-optimizations --enable-static --enable-gpl --enable-pthreads --enable-nonfree --enable-x11grab --enable-libx264 --enable-filters --enable-runtime-cpudetect --disable-lzma --disable-htmlpages
ffmpeg -video_size 1920x1080 -framerate 30 -f x11grab -i :1.0 output.mp4

 *  
 */






#include <bits/stdc++.h>
#include "Screencapture.h"


using namespace std;

/* driver function to run the application */
int main()
{
	Screencapture screen_record;

	screen_record.openCapture();
	screen_record.init_outputfile();
	screen_record.captureVideoFrames();

	cout<<"\nprogram executed successfully\n";

	return 0;
}
