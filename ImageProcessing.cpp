//
// Created by kuba on 20.08.2026.
//
#include "ImageProcessing.h"

void createGUI(GUIvalues& value) {
    value.lowNms = 80;
     value.highNms = 220;
    value.epsilonValue = 2;
    cv::namedWindow("frame", cv::WINDOW_NORMAL);

    cv::createTrackbar(
    "lowNMS",
    "frame",
     &value.lowNms,
    500
    );
    cv::createTrackbar(
      "hightNMS",
      "frame",
       &value.highNms,
       500
      );
    cv::createTrackbar(
    "epsilon",
    "frame",
     &value.epsilonValue,
    100
    );


}
void keyManager(showFiltr &filtr) {
    int key = cv::waitKey(1);
    int viewvMode = -1;
    if (key == 27) {
        filtr.end = true;
        return;
    }
    else if (key >= 49 && key <= 53) {
        viewvMode = key - '0';
    }
    switch (viewvMode) {
        case 1: {
            filtr.showOrginal = !filtr.showOrginal;
            break;
        }
        case 2: {
            filtr.showBLur = !filtr.showBLur;
            break;
        }
        case 3: {
            filtr.showGradient = !filtr.showGradient;
            break;
        }
        case 4: {
            filtr.searchCorner = !filtr.searchCorner;
            break;
        }
        case 5: {
            filtr.showROI = !filtr.showROI;
            break;
        }
    }
}
bool greyscale(const Image2DView<PixelBGR>& src , Image2DView<uint16_t>& dst) {
    if (ROI_X < 0 ||
        ROI_Y < 0 ||
        ROI_Y + ROI_HEIGHT > src.height() ||
        ROI_X + ROI_WIDTH > src.width())
    {
        return false;
    }

    for (int y = 0 ; y < ROI_HEIGHT ; y++ ) {
        for (int x = 0 ; x < ROI_WIDTH ; x ++ ) {

            const PixelBGR& p = src[ROI_Y + y][ROI_X + x];
            dst[y][x] = 0.299 * p.r + 0.587 * p.g + 0.114 * p.b;
        }


    }
    return true;



}
void grey(const Image2DView<PixelBGR>& src , Image2DView<uint16_t>& dst) {

    for (int y = 0 ; y < src.height(); y++) {
        for (int x = 0 ; x < src.width(); x++) {
            const PixelBGR& p = src[y][x];

            dst[y][x] = static_cast<uint8_t>(0.299 * p.r + 0.587 * p.g + 0.114 * p.b);
        }

    }

}
void blur(Image2DView<uint16_t>& src, Image2DView<uint8_t>& dst) {
    constexpr int scales[5][5] = {
        {1,  4,  7,  4, 1},
        {4, 16, 26, 16, 4},
        {7, 26, 41, 26, 7},
        {4, 16, 26, 16, 4},
        {1,  4,  7,  4, 1}
    };
    for ( int y = 0; y < src.height() ; y++) {
        for (int x = 0;  x < src.width(); x++) {
            int pixel = 0 ;
            int sum = 0 ;
            for (int yt = y -2, i = 0; yt <= y +2; yt ++, i ++) {
                for (int xt = x - 2, j = 0; xt <= x +2 ; xt ++, j ++ ){
                    if (yt  < 0 || xt < 0 || yt >= src.height() || xt >= src.width()) {
                        continue;
                    }
                    pixel += src[yt][xt] * scales[i][j];
                    sum += scales[i][j];
                }

            }
            //std::cerr<<pixel/sum<<std::endl;
            dst[y][x] = pixel/sum;
        }
    }
}
void gradient(Image2DView<uint8_t>& src, Image2DView<uint16_t>& strength, Image2DView<uint8_t>& pos) {
    int Gx[3][3] =
{
        {-1,0,1},
        {-2,0,2},
        {-1,0,1}
    };
    int Gy[3][3] =
        {
        {-1,-2,-1},
        {0,0,0},
        {1,2,1}
    };
    for (int y = 0; y <src.height() ; y++) {
        for (int x = 0; x <src.width(); x++) {
            float Gx_sum = 0;
            float Gy_sum = 0;
            for (int yt = y -1, i = 0; yt <= y +1 ; yt++, i ++) {
                for (int xt = x-1, j = 0; xt <= x +1; xt ++, j++ ) {
                    if (yt < 0 || xt < 0 || yt >= src.height() || xt >= src.width() ) {
                        continue;
                    }
                    Gx_sum += src[yt][xt] * Gx[i][j];
                    Gy_sum += src[yt][xt] * Gy[i][j];


                }
            }
            int avg = static_cast<int>(std::abs(Gx_sum) +  std::abs(Gy_sum));
            strength[y][x] = avg;
            float corner = std::atan2(Gy_sum, Gx_sum) * (180.0f/3.14159f);
            if (corner < 0) {
                corner += 180.0f;
            }

            if (corner < 23.0f)
            {
                pos[y][x] = 1;
            }
            else if (corner < 68.0f)
            {
                pos[y][x] = 2;
            }
            else if (corner < 113.0f)
            {
                pos[y][x] = 3;
            }
            else if (corner < 157.0f)
            {
                pos[y][x] = 4;
            }
            else
            {
                pos[y][x] = 1;
            }
           //std::cerr<<static_cast<int>(pos[y][x])<<std::endl;


        }
    }

}
void nms(Image2DView<uint16_t>& strength, Image2DView<uint8_t>& dst, Image2DView<uint8_t>& pos, int high, int low) {


    for (int y = 1; y < strength.height() - 1; y++) {
        for (int x = 1; x < strength.width() - 1; x++) {

            uint16_t current = strength[y][x];
            uint16_t q = 0;
            uint16_t r = 0;

            switch (pos[y][x]) {
                case 1:
                    q = strength[y][x + 1];
                    r = strength[y][x - 1];
                    break;
                case 2:
                    q = strength[y - 1][x + 1];
                    r = strength[y + 1][x - 1];
                    break;
                case 3: // Pionowo (90 deg)
                    q = strength[y - 1][x];
                    r = strength[y + 1][x];
                    break;
                case 4: // Przekątna 135 deg
                    q = strength[y - 1][x - 1];
                    r = strength[y + 1][x + 1];
                    break;
            }


            if (current >= q && current >= r && current > high) {
                dst[y][x] = 255;

            }else if ((current < q || current < r) && current < high && current > low){

                dst[y][x] = 100;
            }
            else {
                dst[y][x] = 0;
            }

        }
    }


}
void hysteresis(Image2DView<uint8_t>& src, Image2DView<uint8_t>& dst,std::queue<point>& point_queue) {
    for (int y = 0 ; y < src.height(); y ++) {
        for (int x = 0 ; x < src.width(); x ++) {
            if (src[y][x] == 255) {
                dst[y][x] = 255;
                point_queue.push({y,x});

            }
            else {
                dst[y][x] = 0;
            }
        }
    }
    while (!point_queue.empty()) {
        auto [y,x] = point_queue.front();
        for (int yt = y -1 ; yt <= y +1; yt ++) {
            for (int xt = x -1 ; xt <= x +1; xt ++) {
                if (xt < 0 || yt < 0  || xt >= src.width() || yt >= src.height() ) {
                    continue;
                }
                if (src[yt][xt] == 100) {
                    src[yt][xt] = 255;
                    dst[yt][xt] = 255;
                    point_queue.push({yt, xt});
                }
            }
        }
        point_queue.pop();
    }


}
void print_roi(Image2DView<PixelBGR>& frame, Image2DView<uint8_t>& roi) {
    for (int y = 0 ; y < roi.height(); y ++ ) {
        for (int x = 0; x <  roi.width(); x++ ) {
            //std::cerr<<+roi[y][x]<<std::endl;
            frame[ROI_Y + y][ROI_X + x].r = roi[y][x];
            frame[ROI_Y + y][ROI_X + x].g = roi[y][x];
            frame[ROI_Y + y][ROI_X + x].b = roi[y][x];



        }

    }


}
void search_bigger_contours(  int& bigestIndex ,std::vector<std::vector<cv::Point>>& contours) {

    double bigestArena = 0;

    for (int i = 0 ; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);

        if (area > bigestArena) {
            bigestArena = area;
            bigestIndex = i;
        }
    }
}
void find_shape(std::vector<cv::Point>& figure, cv::Mat& frame) {

    if ( figure.size() == 3) {
        print_shape(figure, "triangle", frame);
        return;
    }
    else if (figure.size() == 4) {
        double AB = cv::norm(figure[1] - figure[0]);
        double BC = cv::norm(figure[2] - figure[1]);
        double CD = cv::norm(figure[3] - figure[2]);
        double DA = cv::norm(figure[0] - figure[3]);

        double opp1 = AB / CD;
        double opp2 = BC / DA;

        bool validQuad = (opp1 >= 0.75 && opp1 <= 1.35) && (opp2 >= 0.75 && opp2 <= 1.35);

        if (validQuad) {

            double aspectRatio = AB / BC;

            if (aspectRatio >= 0.85 && aspectRatio <= 1.15) {
                print_shape(figure, "square", frame);
            } else {
                print_shape(figure, "rectangle", frame);
            }
        }

    }
    else {
        print_shape(figure, "circle", frame);
    }
}
void print_shape(std::vector<cv::Point>& figure ,std::string shape, cv::Mat& frame) {
    auto topPoint = figure[0];
    for (auto point : figure) {
        if (topPoint.y > point.y) {
            topPoint = point;
        }
    }
    cv::Point textPoint(topPoint.x, topPoint.y -10);
    cv::putText(
    frame,                  // Obraz źródłowy (RGB)
    shape,              // Tekst do wyświetlenia
    textPoint,                // Pozycja (X, Y) lewego dolnego rogu tekstu
    cv::FONT_HERSHEY_SIMPLEX, // Czcionka
    0.6,                    // Skala / rozmiar czcionki
    cv::Scalar(0, 255, 0),  // Kolor (BGR) - zielony
    2                       // Grubość linii tekstu
);


}







