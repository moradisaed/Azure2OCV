#include <stdio.h>
#include <stdlib.h>
#include <k4a/k4a.h>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/core/core.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
using namespace std;
using namespace cv;
int main(int argc, char **argv)
{
	int returnCode = 1;
	k4a_device_t device = NULL;
	const int32_t TIMEOUT_IN_MS = 1000;
	int captureFrameCount;
	k4a_capture_t capture = NULL;

	if (argc < 2)
	{
		printf("%s FRAMECOUNT\n", argv[0]);
		printf("Capture FRAMECOUNT color and depth frames from the device using the separate get frame APIs\n");
		returnCode = 2;
		goto Exit;
	}

	captureFrameCount = atoi(argv[1]);
	printf("Capturing %d frames\n", captureFrameCount);

	uint32_t device_count = k4a_device_get_installed_count();

	if (device_count == 0)
	{
		printf("No K4A devices found\n");
		return 0;
	}

	if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
	{
		printf("Failed to open device\n");
		goto Exit;
	}

	k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	config.color_resolution = K4A_COLOR_RESOLUTION_720P;
	config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	config.camera_fps = K4A_FRAMES_PER_SECOND_15;

	if (K4A_RESULT_SUCCEEDED != k4a_device_start_cameras(device, &config))
	{
		printf("Failed to start device\n");
		goto Exit;
	}

	while (captureFrameCount-- > 0)
	{
		k4a_image_t image;

		// Get a depth frame
		switch (k4a_device_get_capture(device, &capture, TIMEOUT_IN_MS))
		{
		case K4A_WAIT_RESULT_SUCCEEDED:
			break;
		case K4A_WAIT_RESULT_TIMEOUT:
			printf("Timed out waiting for a capture\n");
			continue;
			break;
		case K4A_WAIT_RESULT_FAILED:
			printf("Failed to read a capture\n");
			goto Exit;
		}

		printf("Capture");

		// Probe for a color image
		image = k4a_capture_get_color_image(capture);
		if (image != NULL)
		{
			// get raw buffer
			uint8_t* buffer = k4a_image_get_buffer(image);

			// convert the raw buffer to cv::Mat
			int rows = k4a_image_get_height_pixels(image);
			int cols = k4a_image_get_width_pixels(image);
			Mat rgbMat(rows, cols, CV_8UC4, (void*)buffer, cv::Mat::AUTO_STEP);
			imwrite("rgb.png", rgbMat);
			printf(" | Color res:%4dx%4d stride:%5d ",
				k4a_image_get_height_pixels(image),
				k4a_image_get_width_pixels(image),
				k4a_image_get_stride_bytes(image));

			k4a_image_release(image);
		}
		else
		{
			printf(" | Color None                       ");
		}

		// probe for a IR16 image
		image = k4a_capture_get_ir_image(capture);
		if (image != NULL)
		{
			// get raw buffer
			uint8_t* buffer = k4a_image_get_buffer(image);

			// convert the raw buffer to cv::Mat
			int rows = k4a_image_get_height_pixels(image);
			int cols = k4a_image_get_width_pixels(image);
			Mat irMat(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP);
			imwrite("raw_IR.png", irMat);
			printf(" | Ir16 res:%4dx%4d stride:%5d ",
				k4a_image_get_height_pixels(image),
				k4a_image_get_width_pixels(image),
				k4a_image_get_stride_bytes(image));
			k4a_image_release(image);
		}
		else
		{
			printf(" | Ir16 None                       ");
		}

		// Probe for a depth16 image
		image = k4a_capture_get_depth_image(capture);
		if (image != NULL)
		{
			// get raw buffer
			uint8_t* buffer = k4a_image_get_buffer(image);

			// convert the raw buffer to cv::Mat
			int rows = k4a_image_get_height_pixels(image);
			int cols = k4a_image_get_width_pixels(image);
			Mat depthMat(rows, cols, CV_16U, (void*)buffer, cv::Mat::AUTO_STEP);
			imwrite("raw_depth.png", depthMat);
			printf(" | Depth16 res:%4dx%4d stride:%5d\n",
				k4a_image_get_height_pixels(image),
				k4a_image_get_width_pixels(image),
				k4a_image_get_stride_bytes(image));

			k4a_image_release(image);
		}
		else
		{
			printf(" | Depth16 None\n");
		}

		// release capture
		k4a_capture_release(capture);
		fflush(stdout);
	}

	returnCode = 0;
Exit:
	if (device != NULL)
	{
		k4a_device_close(device);
	}

	return returnCode;
}
