#include "base/thread.h"

class T31Video:public base::Thread
{
    public:

    void run();

    int T31Init();
    int T31Exit();

    int XAInit();  //XA_SDK
    int XAExit();
    int XAProcess( uint8_t* buffer_containing_raw_rgb_data , int w, int h  );


    virtual ~T31Video();



};