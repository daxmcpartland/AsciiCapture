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
    bool colored;
    std::string response;
    std::cout << "Colored? (yes or no)" << std::endl;
    std::cin >> response;
    if (response == "yes")
        colored = true;
    else
        colored = false;
    std::cout << "Close this window to exit" << std::endl;

    //anything less than 4 pixels per ascii really messes with the fps
    if (pxChar < 4)
        pxChar = 4;
    HWND hwndDesktop = GetDesktopWindow();
    float fontScale = 1.0 + log2(pxChar) * .20;

    cv::namedWindow("Display window");

    // different density strings that are sometimes cool.
    //std::string densityString = "MQW#BNqpHERmKdgAGbX8@SDO$PUkwZyF69heT0a&xV%Cs4fY52Lonz3ucJjvItr}{li?1][7<>=)(+*|!/\\;:-,\"_~^.'`";
    //std::string densityString = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1\{\}[]?-_+~<>i!lI;:,\"^`'.        ";
    //reverse(densityString.begin(), densityString.end());
    std::string densityString = " .:-=+*#%@";

    while (true) {
        cv::Mat screen = hwnd2mat(hwndDesktop);
        cv::Mat screenData;
        if (!colored)
            cvtColor(screen, screenData, cv::COLOR_BGR2GRAY);
        else
            screenData = screen; // yea this is dumb idc i'm lazy

        uint8_t* pixelPtr = (uint8_t*)screenData.data;
        int cn = screenData.channels();
        cv::Mat endResult;
        if(!colored)
            endResult = cv::Mat(screen.rows, screen.cols, CV_8UC1);
        else
            endResult = cv::Mat(screen.rows, screen.cols, CV_8UC3);

        int dataCounter = 0;
        for (int i = 0; i < screenData.rows; i += pxChar)
        {
            for (int j = 0; j < screenData.cols; j += pxChar)
            {

                int b = pixelPtr[i * screenData.cols * cn + j * cn + 0];
                int g = pixelPtr[i * screenData.cols * cn + j * cn + 1];
                int r = pixelPtr[i * screenData.cols * cn + j * cn + 2];
                int grayness = 0;
                grayness += (r + b + g) / 3;
                int charValue = int(round(densityString.length() / 255.0 * (grayness / 3)));
                charValue = max(charValue, 0);

                std::string s;
                s.push_back(densityString.at(charValue));
                // make ascii either color of pixel or black and white
                if(!colored)
                    cv::putText(endResult,
                        s,
                        cv::Point(j, i), // Coordinates
                        cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                        fontScale, // Scale. 2.0 = 2x bigger
                        cv::Scalar(255, 255, 255), // BGR Color
                        1, // Line Thickness
                        cv::LINE_AA); // Anti-alias
                else
                    cv::putText(endResult,
                        s,
                        cv::Point(j, i), // Coordinates
                        cv::FONT_HERSHEY_COMPLEX_SMALL, // Font
                        fontScale, // Scale. 2.0 = 2x bigger
                        cv::Scalar(b, g, r), // BGR Color
                        1, // Line Thickness
                        cv::LINE_AA); // Anti-alias
            }
        }

        cv::imshow("Display window", endResult);
        cv::waitKey(27);
    }
    return 0;
}