
The world of smart connected devices is dynamic and develops more sophisticated products to enhance our safety, convenience and entertainment. From surveillance to automotive safety systems and from mobile devices to autonomous drones, intelligent image processing and deep learning are fundamental challenges. New generation smart devices are enriched with AI, which is set to elevate embedded imaging solutions as they are essential to the success of many Internet of Things (IoT) applications.

What is Image Quality?

A quality image does not mean a sharp, bright image with high contrast. Image quality (IQ) is a critical issue that encompasses much more than brightness and sharpness. Several factors contribute significantly to the quality of the image a camera delivers. A large amount of math and processing is involved when the camera takes the light from your sensor and transforms it into a good-looking picture. Fortunately, with the use of dedicated Image signal processors (ISPs), digital cameras and smartphones are handling this in an energy-efficient way.

Why do we need camera IQ tuning?

A camera must be able to take quality images in a variety of lighting conditions, including indoors, in strong sunlight, and in darkness. Video footage and images can easily lose colors, critical details and gain image noise in dim light. Many elements make up a whole camera system and work together to get the final image. Image quality defines how well a camera system performs when reproducing an object/scene. Various characteristics of the camera system like sensor, firmware and lens contribute different elements to the overall quality of an image.

We need IQ tuning to achieve the best image/video quality from the camera system, as there are:

Lens distortion, sensor defects, noise, color response
Variations in mechanical, optical systems and electrical characteristics
Measurement criteria including automatic exposure (AE) with brightness and saturation statistics, automatic focus (AF) with contrast statistics, and automatic white balance (AWB) with color statistics
Subjectivity due to individual image quality preferences
Unprocessed images do not accurately depict an actual scene. This is where an ISP’s job begins to obtain the highest image quality possible.

What is Image Signal Processing?

A lens module is paired with an image sensor and an ISP in every camera system.

ISP in a camera system processes the RAW image from the image sensor to a final image.
For attaining the best image quality, the ISP parameters have to be configured iteratively for various lighting conditions and scenarios.
Due to the large number of ISP parameters that must be optimized, the tuning procedure can take up too many weeks to months.

Figure 1: Typical ISP pipeline for digital camera. (Source: PathPartner)

Using “Bayer’s transformation” to get colorful images

The Bayer filter, or Color Filter Array (CFA), acts as a display, allowing light photons of a certain color into each pixel on the sensor.  These sensors capture red, green and blue colors at each photosite.

Only blue photons will travel through the blue filter, similarly, only red and green photons will be allowed to pass through the red and green filter.        

The values from these photosites are then intelligently merged to create full-color pixels using a technique known as “demosaicing,” and then further processed using an ISP pipeline method to obtain high quality.


Figure 2: Converting light information into an ISP processed image. (Source: PathPartner)

As a result, with the help of an image sensor, light received through the optical lens is converted into an electronic form. And this information is directed to an ISP where it is reconstructed as a picture. The ISP does wonders to improve image quality, even when the lighting isn’t optimal for photo taking.           

What are imaging algorithms?

Imaging algorithms are a suite of custom algorithms developed for various use cases to enhance images under varied conditions and in real time; such as:

3A algorithms (AWB, AF, AE)
Low light enhancement
High dynamic range (HDR) algorithms
Software image pipeline
Stereo camera vision algorithms
Array camera software pipeline for bokeh and refocus applications
Depth map generation for ToF camera sensors
Spatial and temporal noise removal
Multi-frame & single-frame based super-resolution
Electronic image stabilization (EIS)
A set of 3A Algorithms (autoexposure, auto balance and autofocus) controls the camera exposure, white balance and focus.


Figure 3: Example of Untuned vs tuned Auto White Balance (AWB) (Source: PathPartner)

Low light enhancement – Enhance image quality in extremely low light conditions:

Automatic color enhancement
Sensor independent implementation
Supports both Bayer and YUV format
2D Noise reduction without detail loss  

Figure 4: Without low light enhancement (L) and with low light enhancement (R) ((Source: PathPartner)

High dynamic range (HDR) images/videos – Resolve issues with dynamic range that are common with low-cost cameras. It improves the pixels and enables the viewer to see a more realistic image/video.

Image registration and de-ghosting algorithms
Ported easily to custom hardware solutions with camera driver access and HAL
Efficient multi -exposure image fusion methods retaining best information from each of the images

Figure 5: Representing a large luminance variation from dark values to bright values. ((Source: PathPartner)

Performing image quality tuning for a surveillance camera system

The camera’s ability to capture an image of a moving vehicle at a very high speed in day/night mode and various frame rates directly impacts on the image quality. To get the best image quality, the image pipeline must be fine-tuned by configuring each ISP block to respond appropriately to dynamic lighting situations, ensuring that cameras generate acceptable images/videos under all lighting conditions.

For this example, key requirements might include:

Capture high-quality images in day-mode with a variety of lighting conditions ranging from full sun to overcast sky
Capture high-quality images in night lighting conditions with the subject illuminated by integrated infrared LED illuminators
A frame rate > (greater) 25 frames / sec at a full sensor resolution of 5 MP
A step-by-step process of tuning each block of ISP is important to avoid unnecessary iterations. This is mainly because if one of the blocks in ISP is not calibrated or tuned correctly, the subsequent blocks suffer and eventually produce bad image quality. By analyzing the hardware capabilities and limitations based on specifications, the tuning process is carried as below:

3A tuning
Objective image quality tuning
Subjective evaluation
Image quality evaluation and competitive benchmarking
IQ tuning needs to be performed for each of the ISP blocks showing in the figures below for day mode and night mode settings:


Figure 6: Day mode settings. (Source: PathPartner)


Figure 7 – Night mode settings. (Source: PathPartner)

Objective measurements

The tuning process involves calculating different parameters of the camera (Ex: dark current, sensor RGB color space, noise model, AWB reference values, distortion model, etc.) to derive initial settings for ISP and 3A modules. These camera parameters are calculated from the images of standard test charts taken at specific and controlled lighting conditions.

Capturing these images usually requires a lab setup with a uniform light source, test charts and measuring equipment such as lux meters and chroma meters. The number of images to capture usually depends on the configuration of dynamic regions created for each of the ISP blocks based on lux levels/exposure time/sensor gains/color temperatures.     

Subjective evaluation


Once the initial tuning is completed, most of the objective quality requirements are typically met, but a subjective image quality evaluation is done to further fine-tune to produce the preferred results. Fine-tuning involves the following steps:

Capturing and analyzing real-life scenes to find any quality issues and gaps in quality expectations.
Modifying the ISP and 3A parameters to fix any quality issues and accommodating preferences or adding calibration data to handle specific scene types.
This process is carried out in several iterations to verify the fixes and make sure no new artifacts are introduced, making sure IQ preference is met.

It is important to measure the image quality during the tuning process to decide the extent of ISP parameter optimization. This IQ testing is carried out in two phases:

Objective IQ Testing

After initial tuning, objective IQ testing is carried out and KPIs were measured, with images of standard test charts like X-rite MCC, ISO, eSFR, etc. taken in different lighting conditions simulated in imaging lab ranging from 2000K to 7500K and 1lux to 10000lux. The test cases and IQ pass/fail criteria are aligned with the intended use case of the camera. 

Subjective IQ Testing:

Various scenarios like indoor, outdoor at various times of the day, landscape, high dynamic range scenes, low lights, mixed lighting conditions, etc. are typically carried out. The captured images might be used for computer vision-based applications to automatically detect a vehicle number plate, type and color.

Various factors are analyzed and fixed for factors like:

Natural colorfulness
Noise level & texture details (aliasing, moiré patterns, sharpening artifacts, text quality in documents, etc.)
Auto white balance (AWB) decision (Different time of the day, dusk and night)
Brightness (AE) and tonal range
Sharpness and resolution of number plates
IQ Benchmark Test

This process involves comparing and benchmarking the image quality of a tuned device under tests with other standard consumer devices. IQ benchmarking for both objective and subjective criteria helps to improve and make the image quality of the device under test significantly comparable to standard consumer products.         

Conclusion

Tuning an image pipeline is a complex task for a particular camera system. Many artifacts degrade the quality of an image. To address these artifacts a camera must embed an image signal processing process needed to generate high-quality digital images. Hence, it is important to understand the purpose of each ISP block and how each stage of the pipeline can be optimized for a particular use case.





SENSOR OUTPUT HEIGHT : 1080
ISP OUTPUT FPS : 30 / 1
SENSOR OUTPUT RAW PATTERN : RGGB
ISP Top Value : 0xb5742249
ISP Runing Mode : Day
ISP Custom Mode : Disable
ISP WDR Mode : Disable
SENSOR Integration Time : 1410 lines
SENSOR Max Integration Time : 1410 lines
SENSOR analog gain : 38
MAX SENSOR analog gain : 205
SENSOR digital gain : 0
MAX SENSOR digital gain : 0
ISP digital gain : 1
MAX ISP digital gain : 64
ISP Tgain DB : 1
ISP EV value: 3314
ISP EV value log2: 766413
ISP EV value us: 33145
ISP EV min int: 1
ISP EV min again: 1024
ISP WB weighted rgain: 431
ISP WB weighted bgain: 564
ISP WB color temperature: 4964
ISP AWB Start rgain 261: bgain 266
Saturation : 128
Saturation : 128
Sharpness : 128
Contrast : 128
Brightness : 128
Antiflicker : 0
Mirror: Disable, Flip: Disable
Antiflicker nodes: 4: step : 354. 709. 1063. 1418.
debug : ch0 done 7915,ip done 7916,0,0,0,0,0,0
debug1 : 0,0,900





Why do image sensors not come with integrated Image Signal Processors?
Image Signal Processors (ISPs) help convert RAW image data into a high-quality output with noise reduction, gamma correction, auto exposure, etc. So, why don’t manufacturers like Sony or OmniVision integrate an ISP in their image sensors? Get expert insights!
by Prabu KumarJanuary 23, 2023
Share3
Why do image sensors not come with integrated Image Signal Processors
An ISP is a key component in an embedded camera system since a sensor provides the output only in the RAW format. An ISP (Image Signal Processor) is a dedicated processor that converts this RAW image data into a high quality workable output through various processes such as noise reduction, lens shading correction, gamma correction, auto exposure and auto white balance etc.

That said, a logical question that might arise is why don’t image sensors come integrated with an ISP. Why don’t sensor manufacturers such as Sony, ON Semiconductor, or OmniVision consider adding value to their sensor products by integrating a dedicated ISP?

Well, it got me curious too.

So, in this article I attempt to break down the reasons for sensor manufacturers not offering a combination of a sensor and ISP as an integrated solution.

A look back before we start
Before we start looking at the reasons, a bit of history might help.

When we say that sensors don’t come with ISPs, was it always the case? Not really.

Sensor manufacturers used to offer ISPs as well in the past. The last known image sensor that continues to have an integrated ISP and is still selling in volumes is OmniVision’s OV5640, which is a 1/4 inch 5MP camera. However, as some of you may be already aware, OmniVision has decided not to launch image sensors with integrated ISP beyond 5MP anymore.

Reasons for image sensors not offered with an ISP anymore
Image sensors do not come integrated with ISPs anymore because of predominantly 2 reasons:

Growth of microprocessors with built-in ISP
Product developers wanting to choose the ISP based on tuning requirements
Let us look at each of these in detail.

Growth of microprocessors with built-in ISP
Earlier, processors didn’t have a built-in ISP. But today, most of the modern manufacturers like Qualcomm, NXP, and NVIDIA offer an ISP with their microprocessors. Due to this, sensor manufacturers do not want to add to their product cost by integrating an ISP within. Since an ISP is not a value add anymore, sensor manufacturers do not want to run the risk of losing a design win or a bulk order on budget or cost grounds.

Product developers wanting to choose the ISP based on tuning requirements
The second significant reason for manufacturers to come up with raw Bayer filter sensors is that many product developers and design engineers want to pick the ISP on their own based on the tuning they would require. It is also dependent on the features and the interfacing supported by the ISP.

Different ISPs come with different feature sets, and picking and integrating an ISP many a times requires a thorough understanding of the sensor as well. One of e-con Systems’ key strengths is having a suite of products that come with different ISPs for different types of applications.

For example, Hyperyon – a 2MP ultra low light USB camera based on the Sony STARVIS IMX290 sensor – uses the Socionext ISP. On the other hand, e-CAM21_CUTX2 – Sony STARVIS IMX290 camera for NVIDIA Jetson TX2 – uses the NVIDIA Jetson ISP. While Hyperyon is targeted at low-light HDR USB 2.0 applications, e-CAM21_CUTX2 is suited only for low-light MIPI CSI-2 based applications.

External ISP vs Internal ISP
Now that we understand that an ISP comes integrated with a processor (which is when it is called an internal ISP), we need to look at why we might need an external ISP in some cases.

It is well understood that USB cameras definitely need an external ISP. So, the question of choice between an internal and external ISP arises only in the case of other cameras.

Even though processors come with an internal ISP, they have not yet become as sophisticated as external ISPs. External ISPs tend to offer more flexibility and array of features in comparison with internal ISPs. In complex use cases where multiple cameras (say as many as 6) need to be synchronized, an external ISP is recommended for better image output.

Also, some product developers who use NVIDIA processors do not want to use the internal ISP since it consumes additional GPU bandwidth for processing. They would instead prefer to use the processor for algorithm processing alone with the help of GPUs.

Hope you got a fair understanding of why today ISPs come integrated with processors and not image sensors. Also, the choice between an internal and external ISP pretty much depends on your application. The more complex your application, the higher the need for an external ISP.

In case you wish to learn more about ISP tuning, and the significance of using an external ISP, have a look at the article
Camera ISP and the significance of using an external ISP in imaging solutions.

If you have any queries on this topic, please write to us at camerasolutions@e-consystems.com. We would be more than happy to guide you.














A Bayer filter mosaic is a color filter array (CFA) for arranging RGB color filters on a square grid of photosensors. Its particular arrangement of color filters is used in most single-chip digital image sensors used in digital cameras, and camcorders to create a color image. The filter pattern is half green, one quarter red and one quarter blue, hence is also called BGGR, RGBG,[1][2] GRBG,[3] or RGGB.[4]

It is named after its inventor, Bryce Bayer of Eastman Kodak. Bayer is also known for his recursively defined matrix used in ordered dithering.

Alternatives to the Bayer filter include both various modifications of colors and arrangement and completely different technologies, such as color co-site sampling, the Foveon X3 sensor, the dichroic mirrors or a transparent diffractive-filter array.[5]

Explanation

Original scene
Output of a 120×80-pixel sensor with a Bayer filter
Output color-coded with Bayer filter colors
Reconstructed image after interpolating missing color information
Full RGB version at 120×80-pixels for comparison (e.g. as a film scan, Foveon or pixel shift image might appear)
Bryce Bayer's patent (U.S. Patent No. 3,971,065[6]) in 1976 called the green photosensors luminance-sensitive elements and the red and blue ones chrominance-sensitive elements. He used twice as many green elements as red or blue to mimic the physiology of the human eye. The luminance perception of the human retina uses M and L cone cells combined, during daylight vision, which are most sensitive to green light. These elements are referred to as sensor elements, sensels, pixel sensors, or simply pixels; sample values sensed by them, after interpolation, become image pixels. At the time Bayer registered his patent, he also proposed to use a cyan-magenta-yellow combination, that is another set of opposite colors. This arrangement was impractical at the time because the necessary dyes did not exist, but is used in some new digital cameras. The big advantage of the new CMY dyes is that they have an improved light absorption characteristic; that is, their quantum efficiency is higher.

The raw output of Bayer-filter cameras is referred to as a Bayer pattern image. Since each pixel is filtered to record only one of three colors, the data from each pixel cannot fully specify each of the red, green, and blue values on its own. To obtain a full-color image, various demosaicing algorithms can be used to interpolate a set of complete red, green, and blue values for each pixel. These algorithms make use of the surrounding pixels of the corresponding colors to estimate the values for a particular pixel.

Different algorithms requiring various amounts of computing power result in varying-quality final images. This can be done in-camera, producing a JPEG or TIFF image, or outside the camera using the raw data directly from the sensor. Since the processing power of the camera processor is limited, many photographers prefer to do these operations manually on a personal computer. The cheaper the camera, the fewer opportunities to influence these functions. In professional cameras, image correction functions are completely absent, or they can be turned off. Recording in Raw-format provides the ability to manually select demosaicing algorithm and control the transformation parameters, which is used not only in consumer photography but also in solving various technical and photometric problems.[7]

Demosaicing
Main article: Demosaicing
Demosaicing can be performed in different ways. Simple methods interpolate the color value of the pixels of the same color in the neighborhood. For example, once the chip has been exposed to an image, each pixel can be read. A pixel with a green filter provides an exact measurement of the green component. The red and blue components for this pixel are obtained from the neighbors. For a green pixel, two red neighbors can be interpolated to yield the red value, also two blue pixels can be interpolated to yield the blue value.

This simple approach works well in areas with constant color or smooth gradients, but it can cause artifacts such as color bleeding in areas where there are abrupt changes in color or brightness especially noticeable along sharp edges in the image. Because of this, other demosaicing methods attempt to identify high-contrast edges and only interpolate along these edges, but not across them.

Other algorithms are based on the assumption that the color of an area in the image is relatively constant even under changing light conditions, so that the color channels are highly correlated with each other. Therefore, the green channel is interpolated at first then the red and afterwards the blue channel, so that the color ratio red-green respective blue-green are constant. There are other methods that make different assumptions about the image content and starting from this attempt to calculate the missing color values.





Auto-focus (AF), auto-exposure (AE), and automatic white balance (AWB) are algorithms used in cameras to adjust for different lighting conditions: 
Auto-focus: Adjusts the focus of the camera 
Auto-exposure: Adjusts the brightness of the image based on the amount of light reaching the camera sensor 
Auto-white balance: Adjusts the color balance of the image based on the lighting conditions so that white appears white. In auto white balance mode, the camera analyzes the scene and chooses a color temperature to use. 
These algorithms are often used together in a camera's 3A (AE/AWB/AF) signal control. When used together, failed exposure or focus can cause dark or overexposed video, and loss of detail and sharpness. 
























Advanced Features
Faultless techniques for
flawless images
Auto White Balance (AWB)
When you take a picture of a white blank paper under an incandescent light bulb, the digital sensor can hardly illustrate the color of paper as a white. The advanced ISP detects the ambient light condition by analyzing color information from the image sensor and corrects overall color to be balanced automatically.
Samsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : AutoWhite Balance : AutoSamsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : Daylight/SunlightWhite Balance : Daylight/SunlightSamsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : ShadeWhite Balance : ShadeSamsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : CloudyWhite Balance : CloudySamsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : TungstenWhite Balance : TungstenSamsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : FluorescentWhite Balance : FluorescentSamsung Exynos Advanced ISP Advanced Features, Auto White Balance, AWB : FlashWhite Balance : FlashScenery of village with Custom White BalanceCustom White Balance
Auto Exposure (AE)
Auto Exposure is a feature that automatically determines how much light the image sensor is receiving. ISP analyzes the brightness information from the sensor, and calculates and controls the aperture, shutter speed and ISO for appropriate exposure.
Samsung Exynos Advanced ISP Advanced Features, Auto Exposure, AE
AF: Contrast AF and PDAF
In conventional contrast AF, ISP calculates the correct adjustment of focus by finding out highest contrast of colors through continuous trials and errors. Phase Detecting AF technology, once only available in premium DSLR cameras, is now available on mobile devices. ISP with PDAF solution calculates the distance from the object and finds the correct adjustment allowing camera to focus at once.
Samsung Exynos Advanced ISP Advanced Features, AF, Contrast AF and PDAF
Demosaicing
An image sensor, in general, consists of pixels in red, blue and green. These pixels are only able to recognize one of the three colors, so reconstruction process is needed to transform the color information into full color image. ISP calculates and interpolates each pixel’s color by analyzing RGB color information around of the pixel resulting in vivid and natural color image.
Image of a girl holding an ice cream cone with a Dalmatian in RGB colorRGB PixelsImage of a girl holding an ice cream cone with a Dalmatian in Natural colorDemosaicing
Noise Reduction
In a low light condition, image sensor can hardly receive enough light information. So higher ISO or slower shutter speed is required, but this causes heating issue that makes noisy image. The ISP in Exynos features advanced noise reduction algorithm to reduce diverse color or pattern noise while preserving texture details. Notably, 3DNR feature reduces the visible noise in the video footage as well. It distinguishes and reduces noises from the useful details by analyzing neighboring frames in the video stream.
Noisy image of a girl reading a book on the bedNoisy imageClear image of woman using tablet with Advanced Noise ReductionAdvanced Noise Reduction On
Lens and Sensor compensation
Smartphone camera has a handicap due to its usage of small sized components. So lens shading, a vignetting, chromatic aberration, and lens distortion could occur due to non-uniform colors and fixed focal length from the imperfect of a tiny camera module. ISP in Exynos processor with advanced color and distortion correcting algorithm adjusts erroneous color and deformed shape information.
Distorted image of people celebrating on a rooftopDistorted imageCorrected image of people celebrating on a rooftop with Advanced color and distortion correctionAdvanced color and
distortion correction
Gamma Correction
Human eyes are more sensitive to changes in dark and less in bright part whereas image sensor receives and exports light information linearly. And the light intensity is described nonlinearly in the display of mobile device as well. ISP encodes and decodes the light information generated by image sensor to the nonlinear information for human eyes watching display.
Image of people having fireworks on the beach in Linear RAW imageLinear RAW imageImage of people having fireworks on the beach with Gamma correctionGamma correction
Dynamic Range Compression
ISP analyzes dynamic range of an image and adjusts the brightness locally so that the image could have perfect contrast of dark and bright areas. DRC adaptively generates a tone mapping curve and applies it to each pixel of the image allowing the heavily dark and bright parts to be lightened and dimmed respectively for balanced dynamic range.
Dark image of Santorini landscapeRAW imageImage of Santorini landscape with perfect contrast of dark and bright areas through Dynamic Rage CompressionDynamic Rage Compression
Smart WDR (Wide Dynamic Range)
With Samsung image sensor that features zig-zag WDR pattern, ISP is able to generate the WDR image with just a single shot. WDR pattern consists of two types of pixel; one for long time exposure and another for short time exposure. ISP manages image sensor to capture the light by using two different exposure times in a single shot. Then two different light and color information are combined in to one image that displays great details in both bright and dark parts.