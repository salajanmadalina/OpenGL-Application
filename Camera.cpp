#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        cameraUpDirection = cameraUp;
        cameraRightDirection = glm::normalize((glm::cross(glm::normalize(this->cameraTarget - cameraPosition), cameraUpDirection)));
        this->cameraTarget = cameraTarget + glm::normalize(this->cameraTarget - cameraPosition);
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::vec3 aux(
            (cos(glm::radians(yaw)) * cos(glm::radians(pitch))),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        );

        cameraTarget = cameraPosition + glm::normalize(aux);
        cameraRightDirection = glm::normalize(glm::cross(glm::normalize(aux), cameraUpDirection));
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        glm::vec3 cameraTargetCopy = this->cameraTarget;
        glm::vec3 cameraPosCopy = this->cameraPosition;

        if (direction == MOVE_FORWARD) {
            cameraPosition += speed * glm::normalize(this->cameraTarget - this->cameraPosition);
        }
        if (direction == MOVE_BACKWARD) {
            cameraPosition -= speed * glm::normalize(this->cameraTarget - this->cameraPosition);
        }
        if (direction == MOVE_RIGHT) {
            cameraPosition += speed * cameraRightDirection;
        }
        if (direction == MOVE_LEFT) {
            cameraPosition -= speed * cameraRightDirection;
        }
        cameraTarget = cameraPosition + glm::normalize(cameraTargetCopy - cameraPosCopy);
    }

}