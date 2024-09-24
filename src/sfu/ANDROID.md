Generate hello c++ jni APP with new project aicentapp, select GUI in Java only

Then make following changes


CMakeLists.txt
 

cmake_minimum_required(VERSION 3.22.1)

project("aicentapp")

# git clone git@github.com:Adappt-Intelligence-Inc/MagicAI.git

add_subdirectory(MagicAI/src/sfu)


add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
       native-lib.cpp)


target_link_libraries(${CMAKE_PROJECT_NAME}
        runWebrtc
        android
        log)



change build.gradle(Module:App)

  ndk   {
            //abiFilters "arm64-v8a"
            abiFilters "armeabi-v7a"
        }




change native-lib.cpp

#include <jni.h>
#include <string>

extern int testStart( );

extern "C" JNIEXPORT jstring

JNICALL
Java_scope_ar_aicentapp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";


    testStart( );

    return env->NewStringUTF(hello.c_str());
}