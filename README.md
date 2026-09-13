# Real-Time C++ Shape Recognition & Custom Edge Processing Pipeline


#### Description:
This project is a high-performance, real-time Computer Vision application written in C++. It features a custom-built **Canny Edge Detection pipeline** implemented entirely from scratch, paired with geometric shape classification using OpenCV for contour approximation and rendering.

Instead of relying on high-level library abstractions like `cv::Canny()`, this application manually processes raw pixel buffers to provide low-level control over memory, high execution speed, and direct visibility into edge detection algorithms.

---

### 🌟 Key Features

* **Custom Canny Edge Pipeline:**
    * **Grayscale Conversion:** Weighted luminosity model ($0.299R + 0.587G + 0.114B$).
    * **Gaussian Blur ($5 \times 5$):** Custom convolution kernel specifically tuned to suppress strong camera noise.
    * **Sobel Gradient & Orientation:** Magnitude calculation and $4$-direction angle quantization ($0^\circ, 45^\circ, 90^\circ, 135^\circ$).
    * **Non-Maximum Suppression (NMS):** Thinning multi-pixel edges down to 1-pixel wide response lines.
    * **Hysteresis Thresholding:** Queue-based Breadth-First Search (BFS) tracking strong ($255$) and weak ($100$) edge connectivity.

* **Memory Architecture:**
    * **Zero-Allocation Main Loop:** All frame processing buffers are pre-allocated during initialization.
    * **`Image2DView<T>` Abstraction:** Wrapper providing 2D indexing syntax (`img[y][x]`) over contiguous 1D memory buffers without dynamic memory overhead.
    * **Built-in ROI Support:** Native subview capability for Region of Interest processing.

* **Shape Classification:**
    * Identifies geometric primitives (Triangles, Squares, Rectangles, Circles) using contour polygon approximation (`approxPolyDP`) and side-ratio heuristics.

---

### 📁 Project Structure

* **`main.cpp`**: Application entry point, video capture loop, trackbar GUI bindings, and contour drawing.
* **`ImageProcessing.h`**: Data structures (`PixelBGR`, `Image2DView`, `PiplineBuffers`), GUI state, and function declarations.
* **`ImageProcessing.cpp`**: Custom implementations for low-level image processing functions and shape recognition logic.

---

### 🛠️ Compilation and Setup

#### Dependencies:
* C++17 compiler (`g++` or `clang++`)
* OpenCV 4.x

#### Build Command:
```bash
g++ -std=c++17 main.cpp ImageProcessing.cpp -o ShapeDetector `pkg-config --cflags --libs opencv4`
