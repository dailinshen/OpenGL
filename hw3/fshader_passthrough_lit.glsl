#version 120

varying vec4 norm;
varying vec4 light_direction;
varying vec4 viewer_direction;

// uniforms that describe the light and material parameters
uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;

uniform float material_shininess;

uniform bool vshader_turnoff;
varying vec4 color;

void main()
{
    if (vshader_turnoff) {
        vec4 n = normalize(norm);
        vec4 ld = normalize(light_direction);
        vec4 vd = normalize(viewer_direction);
        
        // compute these three terms:
        vec4 ambient_color, diffuse_color, specular_color;
        
        // first, ambient light
        ambient_color = light_ambient * material_ambient;
        
        // next, diffuse
        float dd = dot(ld, n);
        if (dd > 0.0) diffuse_color = dd * (light_diffuse * material_diffuse);
        else diffuse_color = vec4(0.0, 0.0, 0.0, 1.0);
        
        // last, specular
        float sd = dot(normalize(ld + vd), n);
        if (sd > 0.0) specular_color = pow(sd, material_shininess) * (light_specular * material_specular);
        else specular_color = vec4(0.0, 0.0, 0.0, 1.0);
        
        // "gl_FragColor" is already defined for us - it's the one thing you have
        // to set in the fragment shader:
        gl_FragColor = ambient_color + diffuse_color + specular_color;
        gl_FragColor[3] = 1.0;
    } else {
        gl_FragColor = color;
    }
    
}

