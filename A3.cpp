#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cassert>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

static int interaction_radio = 0;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
    : m_luaSceneFile(luaSceneFile),
      m_positionAttribLocation(0),
      m_normalAttribLocation(0),
      m_vao_meshData(0),
      m_vbo_vertexPositions(0),
      m_vbo_vertexNormals(0),
      m_vao_arcCircle(0),
      m_vbo_arcCircle(0),

      m_sphereNode( nullptr ),
      m_left_mouse_key_down( false ),
      m_middle_mouse_key_down( false ),
      m_right_mouse_key_down( false )
{
    resetAll();

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
    // Set the background colour.
    glClearColor(0.35, 0.35, 0.35, 1.0);

    createShaderProgram();

    glGenVertexArrays(1, &m_vao_arcCircle);
    glGenVertexArrays(1, &m_vao_meshData);
    enableVertexShaderInputSlots();

    processLuaSceneFile(m_luaSceneFile);

    // Load and decode all .obj files at once here.  You may add additional .obj files to
    // this list in order to support rendering additional mesh types.  All vertex
    // positions, and normals will be extracted and stored within the MeshConsolidator
    // class.
    unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
            getAssetFilePath("cube.obj"),
            getAssetFilePath("sphere.obj"),
            getAssetFilePath("suzanne.obj")
    });


    // Acquire the BatchInfoMap from the MeshConsolidator.
    meshConsolidator->getBatchInfoMap(m_batchInfoMap);

    // Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
    uploadVertexDataToVbos(*meshConsolidator);

    mapVboDataToVertexShaderInputLocations();

    initPerspectiveMatrix();

    initViewMatrix();

    initLightSources();


    // Exiting the current scope calls delete automatically on meshConsolidator freeing
    // all vertex data resources.  This is fine since we already copied this data to
    // VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
    // this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
    // This version of the code treats the Lua file as an Asset,
    // so that you'd launch the program with just the filename
    // of a puppet in the Assets/ directory.
    // std::string assetFilePath = getAssetFilePath(filename.c_str());
    // m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

    // This version of the code treats the main program argument
    // as a straightforward pathname.
    m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
    if (!m_rootNode) {
        std::cerr << "Could not open " << filename << std::endl;
    }
    assert( m_rootNode->children.size() == 1);
    m_sphereNode = m_rootNode->children.front();

}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
    m_shader.generateProgramObject();
    m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
    m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
    m_shader.link();

    m_shader_arcCircle.generateProgramObject();
    m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
    m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
    m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
    //-- Enable input slots for m_vao_meshData:
    {
        glBindVertexArray(m_vao_meshData);

        // Enable the vertex shader attribute location for "position" when rendering.
        m_positionAttribLocation = m_shader.getAttribLocation("position");
        glEnableVertexAttribArray(m_positionAttribLocation);

        // Enable the vertex shader attribute location for "normal" when rendering.
        m_normalAttribLocation = m_shader.getAttribLocation("normal");
        glEnableVertexAttribArray(m_normalAttribLocation);

        CHECK_GL_ERRORS;
    }


    //-- Enable input slots for m_vao_arcCircle:
    {
        glBindVertexArray(m_vao_arcCircle);

        // Enable the vertex shader attribute location for "position" when rendering.
        m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
        glEnableVertexAttribArray(m_arc_positionAttribLocation);

        CHECK_GL_ERRORS;
    }

    // Restore defaults
    glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
        const MeshConsolidator & meshConsolidator
) {
    // Generate VBO to store all vertex position data
    {
        glGenBuffers(1, &m_vbo_vertexPositions);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

        glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
                meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    // Generate VBO to store all vertex normal data
    {
        glGenBuffers(1, &m_vbo_vertexNormals);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

        glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
                meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }

    // Generate VBO to store the trackball circle.
    {
        glGenBuffers( 1, &m_vbo_arcCircle );
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

        float *pts = new float[ 2 * CIRCLE_PTS ];
        for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
            float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
            pts[2*idx] = cos( ang );
            pts[2*idx+1] = sin( ang );
        }

        glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CHECK_GL_ERRORS;
    }
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
    // Bind VAO in order to record the data mapping.
    glBindVertexArray(m_vao_meshData);

    // Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
    // "position" vertex attribute location for any bound vertex shader program.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
    glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    // Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
    // "normal" vertex attribute location for any bound vertex shader program.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
    glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    //-- Unbind target, and restore default values:
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;

    // Bind VAO in order to record the data mapping.
    glBindVertexArray(m_vao_arcCircle);

    // Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
    // "position" vertex attribute location for any bound vertex shader program.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
    glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    //-- Unbind target, and restore default values:
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
    float aspect = ((float)m_windowWidth) / m_windowHeight;
    m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
    m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
            vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
    // World-space position
    m_light.position = vec3(-2.0f, 5.0f, 0.5f);
    m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
    m_shader.enable();
    {
        //-- Set Perpsective matrix uniform for the scene:
        GLint location = m_shader.getUniformLocation("Perspective");
        glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
        CHECK_GL_ERRORS;

        location = m_shader.getUniformLocation("picking");
        glUniform1i( location, m_do_picking ? 1 : 0 );

        if( !m_do_picking ) {
            //-- Set LightSource uniform for the scene:
            {
                location = m_shader.getUniformLocation("light.position");
                glUniform3fv(location, 1, value_ptr(m_light.position));
                location = m_shader.getUniformLocation("light.rgbIntensity");
                glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
                CHECK_GL_ERRORS;
            }

            //-- Set background light ambient intensity
            {
                location = m_shader.getUniformLocation("ambientIntensity");
                vec3 ambientIntensity(0.05f);
                glUniform3fv(location, 1, value_ptr(ambientIntensity));
                CHECK_GL_ERRORS;
            }
        }
    }
    m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
    // Place per frame, application logic here ...

    uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
    if( !show_gui ) {
        return;
    }

    static bool firstRun(true);
    if (firstRun) {
        ImGui::SetNextWindowPos(ImVec2(50, 50));
        firstRun = false;
    }

    static bool showDebugWindow(true);
    ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
    float opacity(0.5f);

    ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
            windowFlags);


        // Add more gui elements here here ...
        ImGui::Text( "\n -- Application -- " );
        if( ImGui::Button( "Reset Position (I)" ) ) { resetPosition(); }
        if( ImGui::Button( "Reset Orientation (O)" ) ) { resetOrientation(); }
        if( ImGui::Button( "Reset Joints (N)" ) ) { resetJoints(); }
        if( ImGui::Button( "Reset All(A)" ) ) { resetAll(); }
        if( ImGui::Button( "Quit Application" ) ) { glfwSetWindowShouldClose(m_window, GL_TRUE); }

        ImGui::Text( "\n -- Edit -- " );
        if( ImGui::Button( "Undo (U)" ) ) { undo(); }
        if( ImGui::Button( "Redo (R)" ) ) { redo(); }

        ImGui::Text( "\n -- Options -- " );
        if( ImGui::Button( "Circle (C)" ) ) { undo(); }
        if( ImGui::Button( "Z-buffer(Z)" ) ) { undo(); }
        if( ImGui::Button( "Backface culling (B)" ) ) { undo(); }
        if( ImGui::Button( "Frontface culling (F)" ) ) { undo(); }

        ImGui::Text( "\n -- Interaction Mode -- " );
        if ( ImGui::RadioButton( "Position/Orientation (P)", &interaction_radio, 0 ) ) {
            m_interaction_mode = 'P';
        }
        if ( ImGui::RadioButton( "Joints (J)", &interaction_radio, 1 ) ) {
            m_interaction_mode = 'J';
        }


        ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

    ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
        const ShaderProgram & shader,
        const GeometryNode & node,
        const glm::mat4 & viewMatrix,
        const bool do_picking
) {

    shader.enable();
    {
        //-- Set ModelView matrix:
        GLint location = shader.getUniformLocation("ModelView");
        mat4 modelView = viewMatrix * node.trans;
        glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
        CHECK_GL_ERRORS;

        if ( do_picking ) {

            unsigned int idx = node.m_nodeId;
            float r = float(idx&0xff) / 255.0f;
            float g = float((idx>>8)&0xff) / 255.0f;
            float b = float((idx>>16)&0xff) / 255.0f;

            location = shader.getUniformLocation("material.kd");
            glUniform3f( location, r, g, b );
            CHECK_GL_ERRORS;

        } else {
            //-- Set NormMatrix:
            location = shader.getUniformLocation("NormalMatrix");
            mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
            glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
            CHECK_GL_ERRORS;


            //-- Set Material values:
            location = shader.getUniformLocation("material.kd");
            vec3 kd = node.material.kd;

            if ( node.isSelected ) {
                kd = vec3(0.9, 0.2, 0.2);
            }

            glUniform3fv(location, 1, value_ptr(kd));
            CHECK_GL_ERRORS;
            location = shader.getUniformLocation("material.ks");
            vec3 ks = node.material.ks;
            glUniform3fv(location, 1, value_ptr(ks));
            CHECK_GL_ERRORS;
            location = shader.getUniformLocation("material.shininess");
            glUniform1f(location, node.material.shininess);
            CHECK_GL_ERRORS;
        }

    }
    shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

    glEnable( GL_DEPTH_TEST );
    renderSceneGraph(*m_rootNode);


    glDisable( GL_DEPTH_TEST );
    renderArcCircle();
}

void A3::renderSceneGraph(const SceneNode &root) {

    glBindVertexArray(m_vao_meshData);

    renderSceneGraph( &root, m_view );

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

void A3::renderSceneGraph(const SceneNode *root, glm::mat4 M) {

    M = M * root->trans;
    for (const SceneNode * node : root->children) {
        if (node->m_nodeType == NodeType::SceneNode) {
            renderSceneGraph( node, M );
        } else if (node->m_nodeType == NodeType::JointNode) {
            renderJointGraph( node, M );
        } else if (node->m_nodeType == NodeType::GeometryNode) {
            renderGeometryGraph( node, M );
        }
    }
}

void A3::renderJointGraph(const SceneNode *root, glm::mat4 M ) {

    M = M * root->trans;
    for (const SceneNode * node : root->children) {
        if (node->m_nodeType == NodeType::SceneNode) {
            renderSceneGraph( node, M );
        } else if (node->m_nodeType == NodeType::JointNode) {
            renderJointGraph( node, M );
        } else if (node->m_nodeType == NodeType::GeometryNode) {
            renderGeometryGraph( node, M );
        }
    }
}

void A3::renderGeometryGraph(const SceneNode *root, glm::mat4 M ) {
    const GeometryNode * geometryNode = static_cast<const GeometryNode *>(root);

    updateShaderUniforms(m_shader, *geometryNode, M, m_do_picking);


    // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
    BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

    //-- Now render the mesh:
    m_shader.enable();
    glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
    m_shader.disable();

    M = M * root->trans;
    for (const SceneNode * node : root->children) {
        if (node->m_nodeType == NodeType::SceneNode) {
            renderSceneGraph( node, M );
        } else if (node->m_nodeType == NodeType::JointNode) {
            renderJointGraph( node, M );
        } else if (node->m_nodeType == NodeType::GeometryNode) {
            renderGeometryGraph( node, M );
        }
    }
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
    glBindVertexArray(m_vao_arcCircle);

    m_shader_arcCircle.enable();
        GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
        float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
        glm::mat4 M;
        if( aspect > 1.0 ) {
            M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
        } else {
            M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
        }
        glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
        glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
    m_shader_arcCircle.disable();

    glBindVertexArray(0);
    CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
        int entered
) {
    bool eventHandled(false);

    // Fill in with event handling code...

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
        double xPos,
        double yPos
) {
    bool eventHandled(false);

    if ( !ImGui::IsMouseHoveringAnyWindow() ) {

        float delta_x = xPos - m_mouse_x;
        float delta_y = yPos - m_mouse_y;
        m_mouse_x = xPos;
        m_mouse_y = yPos;

        if ( m_interaction_mode == 'P' ) {

            if( m_left_mouse_key_down ) {
                mat4 trans;
                trans = glm::translate(trans, vec3(delta_x/250, -delta_y/200, 0));
                m_view = trans*m_view;
            }

            if( m_middle_mouse_key_down ) {
                mat4 trans;
                trans = glm::translate(trans, vec3(0, 0, delta_y/100));
                m_view = trans*m_view;
            }

            if( m_right_mouse_key_down ) {
                m_sphereNode->rotate('y', delta_x);
                m_sphereNode->rotate('x', delta_y);
            }

        }

    }

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
        int button,
        int actions,
        int mods
) {
    bool eventHandled(false);

    if ( !ImGui::IsMouseHoveringAnyWindow() ) {

        double xPos, yPos;
        glfwGetCursorPos( m_window, &xPos, &yPos );

        if ( button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS ) {
            m_mouse_x = xPos;
            m_mouse_y = yPos;
            m_left_mouse_key_down = true;
            eventHandled = true;
        }

        if ( button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_RELEASE ) {
            m_mouse_x = xPos;
            m_mouse_y = yPos;
            m_left_mouse_key_down = false;
            eventHandled = true;
        }

        if ( button == GLFW_MOUSE_BUTTON_MIDDLE && actions == GLFW_PRESS ) {
            m_mouse_x = xPos;
            m_mouse_y = yPos;
            m_middle_mouse_key_down = true;
            eventHandled = true;
        }

        if ( button == GLFW_MOUSE_BUTTON_MIDDLE && actions == GLFW_RELEASE ) {
            m_mouse_x = xPos;
            m_mouse_y = yPos;
            m_middle_mouse_key_down = false;
            eventHandled = true;
        }

        if ( button == GLFW_MOUSE_BUTTON_RIGHT && actions == GLFW_PRESS ) {
            m_mouse_x = xPos;
            m_mouse_y = yPos;
            m_right_mouse_key_down = true;
            eventHandled = true;
        }

        if ( button == GLFW_MOUSE_BUTTON_RIGHT && actions == GLFW_RELEASE ) {
            m_mouse_x = xPos;
            m_mouse_y = yPos;
            m_right_mouse_key_down = false;
            eventHandled = true;
        }


        if ( m_interaction_mode == 'J' ) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS) {

                m_do_picking = true;

                uploadCommonSceneUniforms();
                glClearColor(1.0, 1.0, 1.0, 1.0 );
                glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
                glClearColor(0.35, 0.35, 0.35, 1.0);

                draw();
                // I don't know if these are really necessary anymore.
                // glFlush();
                // glFinish();
                CHECK_GL_ERRORS;

                xPos *= double(m_framebufferWidth) / double(m_windowWidth);
                yPos = m_windowHeight - yPos;
                yPos *= double(m_framebufferHeight) / double(m_windowHeight);

                GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
                glReadBuffer( GL_BACK );
                glReadPixels( int(xPos), int(yPos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
                CHECK_GL_ERRORS;

                // Reassemble the object ID.
                unsigned int obj_id = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

                SceneNode* body_part = m_rootNode->get_child_by_id( obj_id );

                // a node is selected and direct parent is a joint
                if ( nullptr != body_part
                         && nullptr != body_part->parent
                         && NodeType::JointNode == body_part->parent->m_nodeType ) {

                    assert( nullptr != body_part->parent->parent );

                    JointNode *joint = (JointNode*) body_part->parent;
                    if ( !joint->isSelected ) {
                        cout<< joint << "is selected" <<endl;
                        m_selected_joints.insert( joint );
                        joint->isSelected = true;
                        joint->parent->isSelected = true;

                    } else {
                        cout<< joint << "is deselected" <<endl;
                        m_selected_joints.erase( joint );
                        joint->isSelected = false;
                        joint->parent->isSelected = false;

                    }
                }

                m_do_picking = false;

                CHECK_GL_ERRORS;
            }
        }

    }

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
        double xOffSet,
        double yOffSet
) {
    bool eventHandled(false);

    // Fill in with event handling code...

    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
        int width,
        int height
) {
    bool eventHandled(false);
    initPerspectiveMatrix();
    return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
        int key,
        int action,
        int mods
) {
    bool eventHandled(false);

    if( action == GLFW_PRESS ) {

        if( key == GLFW_KEY_M ) {
            show_gui = !show_gui;
            eventHandled = true;
        }

        if ( key == GLFW_KEY_Q ) {
            glfwSetWindowShouldClose(m_window, GL_TRUE);
            eventHandled = true;
        }
    }
    // Fill in with event handling code...

    return eventHandled;
}

void A3::resetPosition() {
    initViewMatrix();
}

void A3::resetOrientation() {
    m_sphereNode->reset_transform();
}

void A3::resetJoints() {

}

void A3::resetAll() {
    m_interaction_mode = 'P';
    interaction_radio = 0;
    m_do_picking = false;

    resetPosition();
    if ( m_sphereNode != nullptr ) resetOrientation();
    resetJoints();
}
void A3::undo() {
}
void A3::redo() {
}
