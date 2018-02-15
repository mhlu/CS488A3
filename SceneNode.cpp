#include "SceneNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
using namespace std;

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;


// Static class variable
unsigned int SceneNode::nodeInstanceCount = 0;


//---------------------------------------------------------------------------------------
SceneNode::SceneNode(const std::string& name)
  : m_name(name),
    m_nodeType(NodeType::SceneNode),
    trans(mat4()),
    isSelected(false),
    m_nodeId(nodeInstanceCount++),
    parent( nullptr )
{
    invtrans = glm::inverse(trans );
}

//---------------------------------------------------------------------------------------
// Deep copy
SceneNode::SceneNode(const SceneNode & other)
    : m_nodeType(other.m_nodeType),
      m_name(other.m_name),
      trans(other.trans),
      invtrans(other.invtrans)
{
    for(SceneNode * child : other.children) {
        this->children.push_front(new SceneNode(*child));
    }
}

//---------------------------------------------------------------------------------------
SceneNode::~SceneNode() {
    for(SceneNode * child : children) {
        delete child;
    }
}

//---------------------------------------------------------------------------------------
void SceneNode::set_transform(const glm::mat4& m) {
    trans = m;
    invtrans = glm::inverse(trans);
}

//---------------------------------------------------------------------------------------
void SceneNode::reset_transform() {
    trans = glm::mat4();
    invtrans = glm::mat4();
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_transform() const {
    return trans;
}

//---------------------------------------------------------------------------------------
const glm::mat4& SceneNode::get_inverse() const {
    return invtrans;
}

//---------------------------------------------------------------------------------------
void SceneNode::add_child(SceneNode* child) {
    children.push_back(child);
    child->parent = this;
}

//---------------------------------------------------------------------------------------
void SceneNode::remove_child(SceneNode* child) {
    children.remove(child);
    child->parent = nullptr;
}

//---------------------------------------------------------------------------------------
SceneNode* SceneNode::get_child_by_id(unsigned int id) {

    if ( m_nodeId == id )
        return this;

    SceneNode* res = nullptr;
    for ( auto child : children ) {
        res = child->get_child_by_id( id );
        if ( res != nullptr )
            break;
    }
    return res;
}

//---------------------------------------------------------------------------------------
void SceneNode::rotate(char axis, float angle) {
    vec3 rot_axis;

    switch (axis) {
        case 'x':
            rot_axis = vec3(1,0,0);
            break;
        case 'y':
            rot_axis = vec3(0,1,0);
            break;
        case 'z':
            rot_axis = vec3(0,0,1);
            break;
        default:
            break;
    }
    mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);
    trans = rot_matrix * trans;
    invtrans = glm::inverse(trans);
}

//---------------------------------------------------------------------------------------
void SceneNode::scale(const glm::vec3 & amount) {
    trans = glm::scale(amount) * trans;
    invtrans = glm::inverse(trans);
}

//---------------------------------------------------------------------------------------
void SceneNode::translate(const glm::vec3& amount) {
    trans = glm::translate(amount) * trans;
    invtrans = glm::inverse(trans);
}


//---------------------------------------------------------------------------------------
int SceneNode::totalSceneNodes() const {
    return nodeInstanceCount;
}

//---------------------------------------------------------------------------------------
std::ostream & operator << (std::ostream & os, const SceneNode & node) {

    //os << "SceneNode:[NodeType: ___, name: ____, id: ____, isSelected: ____, transform: ____"
    switch (node.m_nodeType) {
        case NodeType::SceneNode:
            os << "SceneNode";
            break;
        case NodeType::GeometryNode:
            os << "GeometryNode";
            break;
        case NodeType::JointNode:
            os << "JointNode";
            break;
    }
    os << ":[";

    os << "name:" << node.m_name << ", ";
    os << "id:" << node.m_nodeId;
    os << "]";

    return os;
}
