# Embedded Renderer
This is a basic 3D renderer I wrote for my EC En 330 (Embedded Systems) class in Fall of 2022.

I wanted to write a 3D renderer, but make it run as fast as I could. In the end, I made the "map" only 2D, where each line is represented as a gray wall of static height in the final rendered image.

I was worried about the computational power of the Zybo board I was using, but it turns out I shouldn't have been. The slowest part of the program was easily the SPI bus that controlled the screen. The actual renderer ran blazingly fast.

Note: I do not have access to the latest version of the code at the moment. I will update this when I get the chance.
