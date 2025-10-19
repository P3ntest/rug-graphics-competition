#!/bin/bash

(
  # The cd and execution only happen if build.sh is successful (exit status 0)
  ./build.sh && cd build && ./OpenGL_2.app/Contents/MacOS/OpenGL_2
)