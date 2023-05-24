#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <Windows.h>

// Copied from stack overflow lol
cv::Mat hwnd2mat(HWND hwnd)
{
    HDC hwindowDC, hwindowCompatibleDC;

    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    cv::Mat src;
    BITMAPINFOHEADER  bi;

    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    float valueOfChangeTheSizeOfTextAppsAndOtherItemsInWindowsDisplaySettings = 1.5;
    srcheight = (int)((float)windowsize.bottom * valueOfChangeTheSizeOfTextAppsAndOtherItemsInWindowsDisplaySettings);
    srcwidth = (int)((float)windowsize.right * valueOfChangeTheSizeOfTextAppsAndOtherItemsInWindowsDisplaySettings);
    height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
    width = windowsize.right / 1;

    src.create(height, width, CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

    return src;
}

int main()
{
    int pxChar;
    std::cout << "Enter how many pixels per character(for best result choose a number between 4-16, ex. 8): " << std::endl;
    std::cin >> pxChar;

    //anything less than 4 pixels per ascii really messes with the fps
    if (pxChar < 4)
        pxChar = 4;
    HWND hwndDesktop = GetDesktopWindow();
    float fontScale = 1.0 + log2(pxChar) * .20;

    // rows = 1067 cols = 1707
    cv::namedWindow("Display window");

    // different density strings that are sometimes cool.
    //std::string densityString = "MQW#BNqpHERmKdgAGbX8@SDO$PUkwZyF69heT0a&xV%Cs4fY52Lonz3ucJjvItr}{li?1][7<>=)(+*|!/\\;:-,\"_~^.'`";
    //std::string densityString = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1\{\}[]?-_+~<>i!lI;:,\"^`'.        ";
    //reverse(densityString.begin(), densityString.end());
    std::string densityString = " .:-=+*#%@";

    while (true) {
        cv::Mat image = hwnd2mat(hwndDesktop);
        std::cout << image.size << std::endl;
        cv::Mat grayImage;
        cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);

        uint8_t* pixelPtr = (uint8_t*)grayImage.data;
        int cn = grayImage.channels();
        cv::Mat test(1067, 1707, CV_8UC1);

        int dataCounter = 0;
        for (int i = 0; i < grayImage.rows; i += pxChar)
        {
            for (int j = 0; j < grayImage.cols; j += pxChar)
            {
                int grayness = 0;
                grayness += pixelPtr[i * grayImage.cols * cn + j * cn + 0];
                int charValue = int(round(densityString.length() / 255.0 * (grayness / 3)));
                charValue = max(charValue, 0);

                std::string s;
                s.push_back(densityString.at(charValue));
                cv::putText(test,
                    s,
                    cv::Point(j, i), // Coordinates
                    cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                    fontScale, // Scale. 2.0 = 2x bigger
                    cv::Scalar(255, 255, 255), // BGR Color
                    1, // Line Thickness
                    cv::LINE_AA); // Anti-alias
            }
        }

        cv::imshow("Display window", test);
        cv::waitKey(25);
    }
    return 0;
}
