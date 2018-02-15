root_node = gr.node( 'root' )
root_node:rotate( 'y', -20.0 )
root_node:scale( 0.25, 0.25, 0.25 )
root_node:translate( 0.0, 0.0, -2.0 )

ball_node = gr.node( 'ball' )
root_node:add_child( ball_node )

red = gr.material( {1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10 )
blue = gr.material( {0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10 )
green = gr.material( {0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10 )
white = gr.material( {1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10 )

torso_geo = gr.mesh( 'cube', 'torso_geo' )
ball_node:add_child( torso_geo )
torso_geo:set_material( green )
torso_geo:scale( 1, 1, 0.5 )

shoulder_geo = gr.mesh( 'cube', 'shoulder_geo' )
torso_geo:add_child( shoulder_geo )
shoulder_geo:set_material( green )
shoulder_geo:scale( 1.0, 1.0, 1/0.5 )
shoulder_geo:scale( 2, 0.2, 0.5 )
shoulder_geo:translate( 0, 0.6, 0 )

lower_neck_geo = gr.mesh( 'sphere', 'lower_neck_geo' )
shoulder_geo:add_child( lower_neck_geo )
lower_neck_geo:set_material( blue )
lower_neck_geo:scale( 1.0/2, 1.0/0.2, 1.0/0.5 )
lower_neck_geo:scale( 0.2, 0.2, 0.2 )
lower_neck_geo:translate( 0, 0.8, 0 )

neck_joint = gr.joint( 'lower_neck_joint', {-20, 0, 90}, {0, 0, 0} )
lower_neck_geo:add_child( neck_joint )

neck_geo = gr.mesh( 'sphere', 'neck_geo' )
neck_joint:add_child( neck_geo )
neck_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
neck_geo:scale( 0.1, 0.3, 0.1 )
neck_geo:translate( 0.0, 1.5, 0.0 )
neck_geo:set_material( green )

upper_neck_geo = gr.mesh( 'sphere', 'upper_neck_geo' )
neck_geo:add_child( upper_neck_geo )
upper_neck_geo:set_material( blue )
upper_neck_geo:scale( 1.0/0.1, 1.0/0.3, 1.0/0.1 )
upper_neck_geo:scale( 0.2, 0.2, 0.2 )
upper_neck_geo:translate( 0, 0.8, 0 )

upper_neck_joint = gr.joint( 'upper_neck_joint', {-20, 0, 90}, {-90, 0, 90} )
upper_neck_geo:add_child( upper_neck_joint )

head_geo = gr.mesh( 'sphere', 'head' )
upper_neck_joint:add_child( head_geo )
head_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
head_geo:scale( 0.4, 0.4, 0.4 )
head_geo:translate( 0.0, 2, 0.0 )
head_geo:set_material( green )

left_eye = gr.mesh('sphere', 'left_eye')
head_geo:add_child(left_eye)
left_eye:scale( 1/.4, 1/.4, 1/.4)
left_eye:scale(0.05, 0.05, 0.05)
left_eye:translate(-0.2, 0.2, 1)
left_eye:set_material(blue)

right_eye = gr.mesh('sphere', 'right_eye')
head_geo:add_child(right_eye)
right_eye:scale( 1/.4, 1/.4, 1/.4)
right_eye:scale(0.05, 0.05, 0.05)
right_eye:translate(0.2, 0.2, 1)
left_eye:set_material(blue)
right_eye:set_material(blue)


-- shoulder + arms ----------------------------------------------------------------
function get_arm( name )

    shoulder_joint = gr.joint( name .. '_shoulder_joint', {-90, 0, 0}, {0, 0, 0} )

    upper_arm_geo = gr.mesh( 'sphere', name .. '_upper_arm_geo' )
    shoulder_joint:add_child( upper_arm_geo )
    upper_arm_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
    upper_arm_geo:scale( 0.1, 0.5, 0.1 )
    upper_arm_geo:set_material( green )
    upper_arm_geo:translate( 0, -1.5, 0 )

    elbow_geo = gr.mesh( 'sphere', name .. '_elbow_geo' )
    upper_arm_geo:add_child( elbow_geo )
    elbow_geo:scale( 1/0.1, 1/0.5, 1/0.1 )
    elbow_geo:scale( 0.2, 0.2, 0.2 )
    elbow_geo:set_material( blue )
    elbow_geo:translate( 0, -0.5, 0 )

    elbow_joint = gr.joint( name .. '_elbow_joint', {-120, 0, 0}, {0, 0, 0} )
    elbow_geo:add_child( elbow_joint )

    lower_arm_geo = gr.mesh( 'sphere', name .. '_lower_arm_geo' )
    elbow_joint:add_child( lower_arm_geo )
    lower_arm_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
    lower_arm_geo:scale( 0.1, 0.5, 0.1 )
    lower_arm_geo:set_material( green )
    lower_arm_geo:translate( 0, -1, 0 )

    wrist_geo = gr.mesh( 'sphere', name .. '_wrist_geo' )
    lower_arm_geo:add_child( wrist_geo )
    wrist_geo:scale( 1/0.1, 1/0.5, 1/0.1 )
    wrist_geo:scale( 0.2, 0.2, 0.2 )
    wrist_geo:set_material( blue )
    wrist_geo:translate( 0, -0.8, 0 )

    wrist_joint = gr.joint( name .. '_wrist_joint', {-45, 0, 45}, {0, 0, 0} )
    wrist_geo:add_child( wrist_joint )

    hand_geo = gr.mesh( 'sphere', name .. '_hand_geo' )
    wrist_joint:add_child( hand_geo )
    hand_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
    hand_geo:scale( 0.3, 0.3, 0.1 )
    hand_geo:set_material( green )
    hand_geo:translate( 0, -1, 0 )

    return shoulder_joint
end



left_shoulder_geo = gr.mesh( 'sphere', 'left_shoulder_geo' )
torso_geo:add_child( left_shoulder_geo )
left_shoulder_geo:scale( 1, 1, 1/0.5 )
left_shoulder_geo:scale( 0.2, 0.2, 0.2 )
left_shoulder_geo:set_material( blue )
left_shoulder_geo:translate( -1, 0.6, 0 )

left_arm = get_arm( 'left ' )
left_shoulder_geo:add_child( left_arm )

right_shoulder_geo = gr.mesh( 'sphere', 'right_shoulder_geo' )
torso_geo:add_child( right_shoulder_geo )
right_shoulder_geo:scale( 1, 1, 1/0.5 )
right_shoulder_geo:scale( 0.2, 0.2, 0.2 )
right_shoulder_geo:set_material( blue )
right_shoulder_geo:translate( 1, 0.6, 0 )

right_arm = get_arm( 'right ' )
right_shoulder_geo:add_child( right_arm )



-- hip_geo + legs -------------------------------------------

hip_geo = gr.mesh( 'cube', 'hip_geo' )
torso_geo:add_child( hip_geo )
hip_geo:set_material( green )
hip_geo:scale( 1.0, 1.0, 1/0.5 )
hip_geo:scale( 1.25, 0.2, 0.5 )
hip_geo:translate( 0, -0.5, 0 )



function get_leg( name )
    hip_joint = gr.joint( name .. '_hip_joint', {-90, 0, 20}, {0, 0, 0} )

    upper_leg_geo = gr.mesh( 'sphere', name ..'_upper_leg_geo' )
    hip_joint:add_child( upper_leg_geo )
    upper_leg_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
    upper_leg_geo:scale( 0.1, 0.5, 0.1 )
    upper_leg_geo:set_material( green )
    upper_leg_geo:translate( 0, -1.5, 0 )

    knee_geo = gr.mesh( 'sphere', name .. '_knee_geo' )
    upper_leg_geo:add_child( knee_geo )
    knee_geo:scale( 1/0.1, 1/0.5, 1/0.1 )
    knee_geo:scale( 0.2, 0.2, 0.2 )
    knee_geo:set_material( blue )
    knee_geo:translate( 0, -0.5, 0 )

    knee_joint = gr.joint( name .. '_knee_joint', {0, 0, 120}, {0, 0, 0} )
    knee_geo:add_child( knee_joint )

    lower_leg_geo = gr.mesh( 'sphere', name .. '_lower_leg_geo' )
    knee_joint:add_child( lower_leg_geo )
    lower_leg_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
    lower_leg_geo:scale( 0.1, 0.5, 0.1 )
    lower_leg_geo:set_material( green )
    lower_leg_geo:translate( 0, -1, 0 )

    ankle_geo = gr.mesh( 'sphere', name .. '_ankle_geo' )
    lower_leg_geo:add_child( ankle_geo )
    ankle_geo:scale( 1/0.1, 1/0.5, 1/0.1 )
    ankle_geo:scale( 0.2, 0.2, 0.2 )
    ankle_geo:set_material( blue )
    ankle_geo:translate( 0, -0.8, 0 )


    ankle_joint = gr.joint( name .. '_ankle_joint', {-90, -80, 0}, {0, 0, 0} )
    ankle_geo:add_child( ankle_joint )

    foot_geo = gr.mesh( 'sphere', name .. '_foot_geo' )
    ankle_joint:add_child( foot_geo )
    foot_geo:scale( 1/0.2, 1/0.2, 1/0.2 )
    foot_geo:scale( 0.3, 0.3, 0.1 )
    foot_geo:set_material( green )
    foot_geo:translate( 0, -1, 0 )

    return hip_joint
end

left_hip_geo = gr.mesh( 'sphere', 'left_hip_geo' )
torso_geo:add_child( left_hip_geo )
left_hip_geo:scale( 1, 1, 1/0.5 )
left_hip_geo:scale( 0.2, 0.2, 0.2 )
left_hip_geo:set_material( blue )
left_hip_geo:translate( -0.3, -0.65, 0 )

left_leg = get_leg( 'left' )
left_hip_geo:add_child( left_leg )

right_hip_geo = gr.mesh( 'sphere', 'right_hip_geo' )
torso_geo:add_child( right_hip_geo )
right_hip_geo:scale( 1, 1, 1/0.5 )
right_hip_geo:scale( 0.2, 0.2, 0.2 )
right_hip_geo:set_material( blue )
right_hip_geo:translate( 0.3, -0.65, 0 )

right_leg = get_leg( 'right' )
right_hip_geo:add_child( right_leg )

return root_node
