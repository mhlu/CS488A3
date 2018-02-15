#pragma once

#include "SceneNode.hpp"

class JointNode : public SceneNode {
public:
    JointNode(const std::string & name);
    virtual ~JointNode();

    void set_joint_x(double min, double init, double max);
    void set_joint_y(double min, double init, double max);
    virtual void rotate(char axis, float angle);

    struct JointRange {
        double min, init, max;
    };


    JointRange m_joint_x, m_joint_y;
    float m_x_angle;
    float m_y_angle;
};
