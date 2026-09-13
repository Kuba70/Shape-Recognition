//
// Created by kuba on 20.08.2026.
//

#ifndef UNTITLED_IMAGEPROCESSING_H
#define UNTITLED_IMAGEPROCESSING_H
#include <cstddef>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <iostream>
#include <vector>

constexpr int ROI_X = 220;
constexpr int ROI_Y = 320;
constexpr int ROI_WIDTH = 100;
constexpr int ROI_HEIGHT = 100;
#pragma pack(push, 1)
struct PixelBGR {
    unsigned char b;
    unsigned char g;
    unsigned char r;
};
#pragma pack(pop)
struct point {
    int y;
    int x;
};
struct GUIvalues {
    int highNms;
    int lowNms;
    int epsilonValue;

};
struct showFiltr {
    bool showBLur = false;
    bool showGradient = false;
    bool showROI = false;
    bool showOrginal = true;
    bool searchCorner = false ;
    bool end = false;
};
template <typename T>
class Image2DView {
private:
    T* data_;
    int width_;
    int height_;
    std::size_t strideElements_; // obsługa paddingu z ramki (np. frame.step)

public:
    Image2DView(T* data, int width, int height, std::size_t strideInBytes = 0)
        : data_(data), width_(width), height_(height) {
        std::size_t bytesPerRow = (strideInBytes > 0) ? strideInBytes : (width * sizeof(T));
        strideElements_ = bytesPerRow / sizeof(T);
    }
    Image2DView subview(int x, int y, int width, int height) const {
        return Image2DView(
            data_ + y * strideElements_ + x,
            width,
            height,
            strideElements_ * sizeof(T)
        );
    }

    // Pozwala na intuicyjną składnię: img[y][x]
    inline T* operator[](int y) {
        return data_ + y * strideElements_;
    }
    Image2DView(const Image2DView&) = default;
    Image2DView& operator=(const Image2DView&) = default;

    inline const T* operator[](int y) const {
        return data_ + y * strideElements_;
    }

    int width() const { return width_; }
    int height() const { return height_; }
};
struct PiplineBuffers {
    std::vector<uint16_t> bufA;
    Image2DView<uint16_t> bufViewA;
    std::vector<uint8_t> bufB;
    Image2DView<uint8_t> bufViewB;
    std::vector<uint8_t> bufC;
    Image2DView<uint8_t> bufViewC;
    std::vector<uint8_t> output;
    Image2DView<uint8_t> outputView;
    std::vector<cv::Point> approx;
    std::vector<std::vector<cv::Point>> contours;
    std::queue<point> point_queue;
    cv::Mat edges;
    PiplineBuffers(size_t width, size_t height)
    : bufA(width * height),
      bufViewA(bufA.data(), width, height),
      bufB(width * height),
      bufViewB(bufB.data(), width, height),
      bufC(width * height),
      edges(height, width, CV_8UC1, bufC.data()),
      bufViewC(bufC.data(), width, height),
      output(height * width),
      outputView(output.data(), width, height) {}

};
struct PipelineViews {
    Image2DView<uint16_t> A;
    Image2DView<uint8_t> B;
    Image2DView<uint8_t> C;
    PipelineViews(PiplineBuffers& bufs)
       : A(bufs.bufViewA.subview(ROI_X, ROI_Y, ROI_WIDTH, ROI_HEIGHT)),
         B(bufs.bufViewB.subview(ROI_X, ROI_Y, ROI_WIDTH, ROI_HEIGHT)),
         C(bufs.bufViewC.subview(ROI_X, ROI_Y, ROI_WIDTH, ROI_HEIGHT)) {}

};
void createGUI(GUIvalues& value);
void keyManager(showFiltr& filtr);
bool greyscale(const Image2DView<PixelBGR>& src, Image2DView<uint16_t>& dst);
void grey(const Image2DView<PixelBGR>& src, Image2DView<uint16_t>& dst);
void blur(Image2DView<uint16_t>& src, Image2DView<uint8_t>& dst);
void gradient(Image2DView<uint8_t>& src, Image2DView<uint16_t>& strength, Image2DView<uint8_t>& pos);
void nms(Image2DView<uint16_t>& strength, Image2DView<uint8_t>& dst, Image2DView<uint8_t>& pos, int high, int low);
void print_roi(Image2DView<PixelBGR>& frame, Image2DView<uint8_t>& roi);
void hysteresis(Image2DView<uint8_t>& src, Image2DView<uint8_t>& dst, std::queue<point>& point_queue );
void search_bigger_contours(  int& bigestIndex ,std::vector<std::vector<cv::Point>>& contours);
void find_shape(std::vector<cv::Point>& figure, cv::Mat& frame);
void mouseCallback(int event, int x, int y, int flags, void* userdata);
void print_shape(std::vector<cv::Point>& figure ,std::string shape, cv::Mat& frame);
#endif //UNTITLED_IMAGEPROCESSING_H
