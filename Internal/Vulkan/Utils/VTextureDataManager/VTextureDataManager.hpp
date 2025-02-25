//
// Created by wpsimon09 on 25/02/25.
//

#ifndef VTEXTUREDATAMANAGER_HPP
#define VTEXTUREDATAMANAGER_HPP

namespace VulkanCore {
    /**
     * This class is going to be responsible for allocating the textures and passing all commands in the current "session"
     * meaning that if i load a model a session is created which can be useed to record session specific command buffers and in bulk submit all of them at once, everything from texture layout transition
     * to texture copping and more
     */
    class VTextureDataManager {

};

} // VulkanCore

#endif //VTEXTUREDATAMANAGER_HPP
