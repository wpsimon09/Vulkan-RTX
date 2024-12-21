//
// Created by wpsimon09 on 21/12/24.
//

#ifndef VIEWPORT_HPP
#define VIEWPORT_HPP

namespace vk
{
    class ImageView;
}

namespace VEditor
{
    class ViewPort {
    public:
        ViewPort(vk::ImageView& rendererOutput);
    private:
        vk::ImageView& m_rendererOutput;
    };
}



#endif //VIEWPORT_HPP
