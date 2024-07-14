// xailient
#include <xa-sdk/xa_object_detection_sdk.hpp>
// note that we do not need this library in the final release, this is only a helper library to read images from files
#include <xa-sdk/xa_image_file_reader.hpp>
// std
#include <iostream>


/**
 * @brief Logging callback example
 *
 * @param msg
 */
static void
logCallbackFunction(xailient::sdk::LogType logType, const char* msg)
{
    std::cout << "[" << xailient::sdk::toString(logType) << "]: " << msg << "\n";
}

/**
 * @brief Process the inference output as needed, for now we print it only
 *
 * @param output
 */
static void
processOutput(const xailient::sdk::BizcontrollerOutput& output)
{
    for (const auto& obj : output.detectedObjects) {
        std::cout << "Detected Object ID: " << obj.id << std::endl;
        std::cout << "Label: " << xailient::sdk::toString(obj.label) << std::endl;
        std::cout << "Confidence: " << obj.confidence << std::endl;
        std::cout << "BoundingBox: (" << obj.bbox.xmin << ", " << obj.bbox.ymin << ", "
                  << obj.bbox.xmax << ", " << obj.bbox.ymax << ")" << std::endl;
        std::cout << "-----------------------------------" << std::endl;
    }
}

int
main(void)
{
    // initialize the SDK
    xailient::sdk::SdkInitData sdkInitData{
        .logFunction = logCallbackFunction
    };
    if (xailient::sdk::xa_sdk_initialize(sdkInitData) != xailient::sdk::ErrorCode::XA_ERR_NONE) {
        std::cerr << "Error initializing the object detection sdk" << std::endl;
        return -1;
    }

    // configure the sdk, update as needed
    auto config = xailient::sdk::xa_sdk_get_config();
    xailient::sdk::xa_sdk_configure(config);

    // we read the image from a file, note that this is temporary code, this should be read from the camera
    xailient::sdk::RGBImage rgbImage;
    if (!xailient::sdk::read_rgb_from_file("./image.jpg", rgbImage)) {
        std::cerr << "Error reading the image from the file image.jpg" << std::endl;
        return -2;
    }

    // convert to the input image type, which is the one we will be feeding to the sdk
    xailient::sdk::Image inputImage{
        .buff = rgbImage.buffer.get(),
        .width = rgbImage.width,
        .height = rgbImage.height
    };

    // run inference
    xailient::sdk::BizcontrollerOutput inferenceOutput;
    if (xailient::sdk::xa_sdk_process_image(inputImage, inferenceOutput) != xailient::sdk::ErrorCode::XA_ERR_NONE) {
        std::cerr << "Error processing the image" << std::endl;
        return -3;
    }

    // process the output
    processOutput(inferenceOutput);

    xailient::sdk::xa_sdk_uninitialize();
    return 0;
}
