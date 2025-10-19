# Screen Space Reflections Shader for OpenGL

This project was made as a submission to the RUG Computer Graphics Course Competition 2025. It implements a screen space reflections (SSR) shader in OpenGL using a deferred rendering pipeline.

> Author: Julius van Voorden (j.van.voorden@student.rug.nl)

## Final Product

The following GIF showcases the real-time rendered screen space reflections effect in action:

![Recording of the finished project](screenshots/recording.gif)

## Assets

All models have been hand made for the competition using Blender. The RUG Logo for the emissive sign was sourced from an official RUG publication and cropped to fit the model. Two external textures under CC0 license were used for the concrete floor and apartment building facade:

- [Facade](https://ambientcg.com/view?id=Facade002)
- [Concrete Wall](https://polyhaven.com/a/concrete_wall_006)

## Inspiration

Early on in the semester I was visiting a friend in Den Haag, NL. During a night time walk along the water, I noticed beautiful reflections of different lights in the very slightly rippling water surface. I took some reference pictures and used them as inspiration throughout the project:

![Inspiration 1](screenshots/inspiration1.jpg)
![Inspiration 2](screenshots/inspiration2.jpg)

## Quick overview of the SSR technique

From my research I have found that the screen space reflection ray tracing can be done on a pixel level, using a pixel-tracer similar to what advanced voxel tracer engines do in 3D. Alternatively, one can step through view( / world) space using a small enough fixed step size. Even though this solution is less accurate and can be more performance intensive if the step size is too small, it seemed easier to implement, so I went with this approach for this project. The SSR algorithm can be found in the `lighting_frag.glsl` shader.
