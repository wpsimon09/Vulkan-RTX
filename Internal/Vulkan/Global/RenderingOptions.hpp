//
// Created by wpsimon09 on 16/03/25.
//

#ifndef RENDERINGOPTIONS_HPP
#define RENDERINGOPTIONS_HPP

namespace GlobalVariables::RenderingOptions {
inline bool  EnableFrustrumCulling = true;
inline int   MSAASamples           = 4;
inline float OutlineWidth          = 0.1;
inline bool  PreformDepthPrePass   = true;
inline int   MaxRecursionDepth     = 1;
}  // namespace GlobalVariables::RenderingOptions

#endif  //RENDERINGOPTIONS_HPP
