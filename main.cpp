#include "ImageProcessing.h"

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Please check video input!!" << std::endl;
        return 1;
    }

    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) {
        std::cerr << "Failed to grab initial frame!" << std::endl;
        return 1;
    }

    size_t width = frame.cols;
    size_t height = frame.rows;

    PiplineBuffers bufs(width, height);
    PipelineViews roi(bufs);
    GUIvalues GUIvalue;
    showFiltr filtr;

    createGUI(GUIvalue);

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "ZOMBIE frame" << std::endl;
            return 1;
        }

        // 1. Przygotowanie danych wejściowych z OpenCV frame
        PixelBGR* rawPixels = reinterpret_cast<PixelBGR*>(frame.data);
        Image2DView<PixelBGR> inputView(rawPixels, width, height);

        // Wybór odpowiednich widoków (cały obraz vs. ROI)
        int bigestPointIndex = -1;


        grey(inputView, bufs.bufViewA);

        blur(bufs.bufViewA, bufs.bufViewB);


        gradient(bufs.bufViewB, bufs.bufViewA, bufs.bufViewC);
        nms(bufs.bufViewA, bufs.bufViewB, bufs.bufViewC, GUIvalue.highNms, GUIvalue.lowNms);
        hysteresis(bufs.bufViewB, bufs.bufViewC, bufs.point_queue);


        cv::Mat edgeMat(height, width, CV_8UC1, bufs.bufC.data());
        cv::findContours(edgeMat, bufs.contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        search_bigger_contours(bigestPointIndex, bufs.contours);
        if (bigestPointIndex >= 0) {
            double epsilon = (GUIvalue.epsilonValue / 100.0f) * cv::arcLength(bufs.contours[bigestPointIndex], true);
            cv::approxPolyDP(bufs.contours[bigestPointIndex], bufs.approx, epsilon, true);
            for (size_t i = 0; i < bufs.approx.size(); i++) {
                cv::line(
                    frame,
                    bufs.approx[i],
                    bufs.approx[(i + 1) % bufs.approx.size()],
                    cv::Scalar(0, 255, 0),
                    2
                );
            }
        }
        find_shape(bufs.approx, frame);
    cv::imshow("frame", frame);
    //cv::Mat debugImg(height, width, CV_8UC1, bufs.output.data());
    //cv::imshow("frame", debugImg);


    // Key press handling & break condition (Wymagane przez OpenCV GUI!)
    if (cv::waitKey(1) == 27) { // Klawisz ESC lub flaga końca
        break;
    }
}
    return 0;
}