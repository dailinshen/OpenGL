
#version 120

// we are going to be getting an attribute from the main program, named
// "vPosition", one for each vertex.
// on mac you may have to say: "attribute vec4 vPosition;" instead of this:
//attribute vec4 vPosition;
attribute vec4 vPosition;

// on mac you may have to say: "attribute vec4 vNorm;" instead of this:
//attribute vec4 vNorm;
attribute vec4 vNorm;

// we are going to be outputting a single 4-vector, called color, which
// may be different for each vertex.
// the fragment shader will be expecting these values, and interpolate
// them according to the distance of the fragment from the vertices
varying vec4 color;

uniform vec4 eye;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 light_position;


uniform bool vshader_turnoff;

//
varying vec4 norm;
varying vec4 light_direction;
varying vec4 viewer_direction;

//void main()
//{
//    norm = vNorm;
//    light_direction = light_position - vPosition;
//    viewer_direction = eye - vPosition;
//    
//    gl_Position = projection * view * vPosition;
//} 



uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

void main() {
    if (vshader_turnoff) {
        norm = vNorm;
        light_direction = light_position - vPosition;
        viewer_direction = eye - vPosition;
    
        gl_Position = projection * view * vPosition;

    } else {
        vec4 vpos = view * vPosition;
        vec4 new_light_position = view * light_position;
        vec4 new_norms = view * vNorm;
        
        vec4 light_dir = normalize(new_light_position - vpos);
        vec4 eye_dir = normalize(eye - vpos);
        vec4 half_vec = normalize(light_dir + eye_dir);
        
        //ambient
        vec4 ambient;
        ambient = material_ambient * light_ambient;
        
        // diffuse
        vec4 diffuse;
        float cosnl = max(0.0, dot(new_norms, light_dir));
        diffuse = (material_diffuse * light_diffuse) * cosnl;
        
        //specular
        vec4 specular;
        float cosnh = max(0.0, dot(new_norms, half_vec));
        specular = (material_specular * light_specular) * pow(cosnh, material_shininess);
        
        color = diffuse + specular + ambient;
        color[3] = 1.0;
        
        gl_Position = projection *  vpos;
    }
}
