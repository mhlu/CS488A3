#include "JointNode.hpp"
#include <string>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "cs488-framework/MathUtils.hpp"
using namespace glm;


//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
    : SceneNode(name)
{
    m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
    m_joint_x.min = min;
    m_joint_x.init = init;
    m_joint_x.max = max;
    m_x_angle = m_joint_x.init;
    SceneNode::rotate( 'x', m_x_angle );
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
    m_joint_y.min = min;
    m_joint_y.init = init;
    m_joint_y.max = max;
    m_y_angle = m_joint_y.init;
    SceneNode::rotate( 'y', m_y_angle );
}

void JointNode::rotate( char axis, float angle ) {
    vec3 rot_axis;

    switch (axis) {
        case 'x':

            if ( m_joint_x.max == m_joint_x.min )
                return;

            rot_axis = vec3(1,0,0);
            if ( (angle + m_x_angle) > m_joint_x.max ) {
                angle = m_joint_x.max - m_x_angle;
            } else if ( (angle + m_x_angle) < m_joint_x.min ) {
                angle = m_joint_x.min - m_x_angle;
            }
            m_x_angle += angle;

            break;

        case 'y':

            if ( m_joint_y.max == m_joint_y.min )
                return;

            rot_axis = vec3(0,1,0);
            if ( (angle + m_y_angle) > m_joint_y.max ) {
                angle = m_joint_y.max - m_y_angle;
            } else if ( (angle + m_y_angle) < m_joint_y.min ) {
                angle = m_joint_y.min - m_y_angle;
            }

            break;

        default:
            break;
    }
    mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
    trans = rot_matrix * trans;
    invtrans = glm::inverse(trans);
}
