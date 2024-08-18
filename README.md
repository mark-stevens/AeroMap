![logo-crop-10](https://github.com/user-attachments/assets/e0e3a73e-73dc-45a0-888f-381da508d935)

<b>AeroMap is a simple desktop drone mapping application.</b>

Originally, AeroMap was a C/C++ port of [OpenDroneMap](https://github.com/OpenDroneMap/ODM). I have always been fascinated by
the seeming magic of photogrammetry as well as drone mapping in general. When I discovered ODM, I had finally found a project
that walked me through, step by step, how a big pile of pictures could be turned into accurate 3D models and orthophotos. Being a
C programmer, I could see the best way to really learn this would to port it from Python into my own "native" language :-).

After the initial port I added some limited lidar functionality and decided to publish it on GitHub should anyone find it useful.

Now that I'm hooked, I am continuing development. Here is my near-term road map:
    
    - Ground Control Points
    - Multi-Spectral Data Sets
    - Thermal Data Sets
    - View/Analyze Outputs Directly in AeroMap
    - Installer
    

<b>Setting Up Build Environment</b>

    - Install Qt 5.15.2
    - Install Visual Studio Community 2019
    - Install OpenCV 4.10.0
        - Set OPENCV_PATH environment variable to install location
        - Add to Path: [path to]opencv-4.10.0\build\bin
        - Add to Path: [path to]opencv-4.10.0\build\x64\vc16\bin
    - Set PROJ_LIB environment variable ([path to]/AeroMap/Lib/proj-7.1.1/build_vs14_x64)
    - Add to Path environment variable: [path to]/Lib/glew-2.2.0/bin/Release/x64
