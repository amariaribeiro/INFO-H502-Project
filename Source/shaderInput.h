#ifndef SHADER_INPUT_H
#define SHADER_INPUT_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class ShaderInput
{
public:
    const std::string v_earth =
        "#version 330 core\n"
        "in vec3 position; \n"
        "in vec2 tex_coord; \n"
        "in vec3 normal; \n"

        "out vec3 v_normal; \n"
        "out vec3 v_frag_coord; \n"
        "out vec2 v_tex; \n"


        "uniform mat4 M; \n"
        "uniform mat4 itM; \n"
        "uniform mat4 V; \n"
        "uniform mat4 P; \n"

        " void main(){ \n"
        "vec4 frag_coord = M*vec4(position, 1.0);"
        "gl_Position = P*V*frag_coord;\n"
        "v_normal = vec3(itM * vec4(normal, 1.0)); \n"
        "v_frag_coord = frag_coord.xyz; \n"
        "\n"
        "v_tex = tex_coord; \n"
        "}\n";

    const std::string f_earth = "#version 330 core\n"
        "out vec4 FragColor;"
        "precision mediump float; \n"

        "in vec3 v_normal; \n"
        "in vec3 v_frag_coord; \n"
        "in vec2 v_tex; \n"

        "uniform sampler2D texture; \n"
        "uniform vec3 materialColour; \n"

        "uniform vec3 u_view_pos; \n"

        //for the light equation

        "struct Light{\n"
        "vec3 light_pos; \n"
        "float ambient_strength; \n"
        "float diffuse_strength; \n"
        "float specular_strength; \n"
        //attenuation factor
        "float constant;\n"
        "float linear;\n"
        "float quadratic;\n"
        "};\n"
        "uniform Light light;"

        "uniform float shininess; \n"

        "float specularCalculation(vec3 N, vec3 L, vec3 V ){ \n"
        "vec3 R = reflect (-L,N);  \n " //reflect (-L,N) is  equivalent to //max (2 * dot(N,L) * N - L , 0.0) ;
        "float cosTheta = dot(R , V); \n"
        "float spec = pow(max(cosTheta,0.0), 32.0); \n"
        "return light.specular_strength * spec;\n"
        "}\n"


        "void main() { \n"
        //computing light components
        "vec3 N = normalize(v_normal);\n"
        "vec3 L = normalize(light.light_pos - v_frag_coord) ; \n"
        "vec3 V = normalize(u_view_pos - v_frag_coord); \n"
        "float specular = specularCalculation(N, L, V); \n"
        "float diffuse = light.diffuse_strength * max(dot(N,L),0.0);\n"
        "float distance = length(light.light_pos - v_frag_coord);"
        "float attenuation = 1 / (light.constant + light.linear * distance + light.quadratic * distance * distance);"
        "float light = light.ambient_strength + attenuation * (diffuse + specular); \n"

        //applying light to object texture
        "FragColor = texture(texture, v_tex) * vec4(light); \n"
        "} \n";

    //for the cubemap
    const std::string sourceVCubeMap = "#version 330 core\n"
        "in vec3 position; \n"
        "in vec2 tex_coords; \n"
        "in vec3 normal; \n"

        //only P and V are necessary
        "uniform mat4 V; \n"
        "uniform mat4 P; \n"

        "out vec3 texCoord_v; \n"

        " void main(){ \n"
        "texCoord_v = position;\n"
        //remove translation info from view matrix to only keep rotation
        "mat4 V_no_rot = mat4(mat3(V)) ;\n"
        "vec4 pos = P * V_no_rot * vec4(position, 1.0); \n"
        // the positions xyz are divided by w after the vertex shader
        // the z component is equal to the depth value
        // we want a z always equal to 1.0 here, so we set z = w!
        // Remember: z=1.0 is the MAXIMUM depth value ;)
        "gl_Position = pos.xyww;\n"
        "\n"
        "}\n";

    const std::string sourceFCubeMap =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "precision mediump float; \n"
        "uniform samplerCube cubemapSampler; \n"
        "in vec3 texCoord_v; \n"
        "void main() { \n"
        "FragColor = texture(cubemapSampler,texCoord_v); \n"
        "} \n";

};

#endif