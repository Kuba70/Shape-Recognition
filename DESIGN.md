
# Design Decisions & Technical Architecture

### 1. Choice of Language: C++
C++ was explicitly chosen for this project to achieve real-time computer vision performance through direct memory manipulation:
* **Pointers & Memory Control:** Allows passing raw pixel pointers (`reinterpret_cast<PixelBGR*>`) directly from OpenCV's `cv::Mat` to custom structures without reallocating image data.
* **Custom Structs & Lightweight Data Types:** Packed structures like `PixelBGR` and generic templates allow clear memory layouts tailored to the image format.

### 2. Pre-Allocated Pipeline Buffers (Zero Allocation Loop)
Dynamic memory allocation (`new` / `malloc` / `std::vector::resize`) inside a real-time frame capture loop causes heap fragmentation and performance drops. 
* All processing buffers (`bufA`, `bufB`, `bufC`, `output`) are pre-allocated once inside the `PiplineBuffers` constructor before the main execution loop starts.
* Inside the loop, algorithms populate these pre-allocated buffers directly via raw pointers and views, keeping frame processing fast and deterministic.

### 3. Image Abstraction: `Image2DView<T>`
Storing images in nested vectors (`std::vector<std::vector<T>>`) hurts cache locality due to multiple pointer indirections.
* Pixel data is stored in single contiguous 1D arrays (`std::vector<T>`).
* To keep code readable and maintain 2D grid coordinates, `Image2DView<T>` wraps flat arrays and overloads `operator[]`.
* This allows natural `view[y][x]` access while under the hood performing efficient strided offset arithmetic (`y * stride + x`).

### 4. Noise Suppression: $5 \times 5$ Gaussian Kernel
Initial testing with standard $3 \times 3$ kernels resulted in heavy edge distortion and false positives due to significant webcam sensor noise (graininess). 
* A larger $5 \times 5$ Gaussian kernel with pre-calculated integer coefficients was selected to aggressively smooth out camera grain before calculating Sobel gradients.

### 5. Region of Interest (ROI) Architecture & Trade-Offs
The project fully supports ROI-based visual processing:
* The `Image2DView::subview()` method can isolate specific frame regions without copying pixels.
* Switching from full-frame to ROI processing requires changing the `grey()` call to `greyscale()` and substituting full image dimensions with `ROI_WIDTH` and `ROI_HEIGHT`.
* **Decision:** Although ROI functionality is fully implemented in the codebase, full-frame processing was preferred for the final demo. At the current camera resolution ($640 \times 400$), a $100 \times 100$ ROI window was visually too small and constrained shape movement unnecessarily.

### 6. Queue-Based Hysteresis (BFS vs. Recursion)
Hysteresis edge tracking connects weak edges ($100$) to strong edges ($255$).
* Instead of recursive Depth-First Search (which risks stack overflow on large noise clusters), a Breadth-First Search using `std::queue<point>` was implemented.
* Strong edge coordinates are enqueued first, and their $8$-connected weak neighbors are iteratively promoted and enqueued.